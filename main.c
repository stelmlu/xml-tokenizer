#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// TODO: Support escape signs (done)
// TODO: Allocate the stack and xml_t block as the same.
// TODO: Ignore <!DOCTYPE ... > tags. (done)
// TODO: Support trim, collapse and xml:space (done)
// TODO: Create custom function for strcmp, itoa and toupper.
// TODO: Function document
// TODO: Add to a header file.
// TODO: README.md
// TODO: Test with different xml files.

typedef struct xml__impl xml_t;

typedef enum {
	XML_DECLARATION,
	XML_DOCUMENT_BEGIN, XML_DOCUMENT_END,
	XML_TAG_START, XML_TAG_END,
	XML_ATTRIBUTE,
	XML_TEXT,
	XML_ERROR
} xml_token_t;

xml_t* xml_fopen(const char* filename);

xml_token_t xml_next_token(xml_t* xml);

const char* xml_get_name(xml_t* xml);

const char* xml_get_value(xml_t* xml);

const char* xml_get_text(xml_t* xml);

const char* xml_get_error(xml_t* xml);

int xml_get_trim(xml_t* xml);

int xml_get_collapse(xml_t* xml);

void xml_set_trim(xml_t* xml, int enable);

void xml_set_collapse(xml_t* xml, int enable);

void xml_close(xml_t* xml);

///////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////////////////////////////////////////////

#define STACK_SIZE (4096)
#define XML_SPACE_STACK_SIZE (32)
#define LABEL(addr) do{case addr:;}while(0);
#define JMP(addr) do{xml->lc=addr;goto jp;}while(0)
#define CALL(ret,addr) do{xml->lc=addr;xml__push(xml,&(enum xml__label){ret},sizeof(enum xml__label));goto jp;case ret:;}while(0)
#define RET() do{xml->lc=*(enum xml__label*)xml__pop(xml, sizeof(enum xml__label));goto jp;}while(0);
#define TOK(addr,tok) do{xml->lc=addr;return tok;case addr:;}while(0)
#define NEXTCH() do{if(!xml__nextch(xml)) JMP(xml__error_loop);}while(0)
#define FLAG_TRIM (0)
#define FLAG_COLLAPSE (1)
#define FLAG_PRESERVE (2)

enum xml__label {
	xml__start,
	xml__padding,
	xml__name,
	xml__value,
	xml__attr,
	xml__tag, xml__tag_loop,
	xml__escape_sign,
	xml__error, xml__error_loop,
	xml__c1, xml__c2, xml__c3, xml__c4, xml__c5, xml__c6, xml__c7, xml__c8, xml__c9, xml__c10,
	xml__c11, xml__c12, xml__c13, xml__c14, xml__c15, xml__c16, xml__c17, xml__c18, xml__c19,
	xml__c20, xml__c21, xml__c22, xml__c23, xml__c24,
	xml__l1, xml__l2, xml__l3, xml__l4,
	xml__t1, xml__t2, xml__t3, xml__t4, xml__t5, xml__t6, xml__t7, xml__t8
};

const char xml__error_unexpected_end_of_file[] = "Error: Unexpected end of file.";
const char xml__error_while_reading_file[] = "Error: While reading file, code: ";
const char xml__error_prefix[] = "Error(";
const char xml__unexpected_sign[] = "): Unexpected sign.";

struct xml__xml_space {
	int level, preserve;
};

struct xml__impl {
	FILE* fp;
	enum xml__label lc;
	int ch, ra, rb, rc, row, col, sc, level, flags, xml_space_count;
	struct xml__xml_space xml_space_stack[XML_SPACE_STACK_SIZE];
	uint8_t* stack;
};

inline void xml__push(xml_t* xml, const void* data, size_t size)
{
	if ((xml->sc + size) > STACK_SIZE) {
		fprintf(stderr, "PANIC: Svg stack overflow.");
		exit(-1);
	}
	for (size_t i = 0; i < size; i++) {
		xml->stack[xml->sc++] = ((uint8_t*)data)[i];
	}
}

inline const void* xml__pop(xml_t* xml, size_t size)
{
	xml->sc -= size;
	return &(xml->stack[xml->sc]);
}

inline const void* xml__peek(xml_t* xml, size_t size, size_t index)
{
	return &(xml->stack[xml->sc - size - index]);
}

inline const char* xml__peek_str(xml_t* xml)
{
	int size = *((uint32_t*)&xml->stack[xml->sc - sizeof(int)]);
	return xml->stack + xml->sc - sizeof(uint32_t) - size - 1;
}

inline const char* xml__pop_str(xml_t* xml) {
	int size = *((int*)&xml->stack[xml->sc - sizeof(int) - sizeof(uint8_t)]);
	const char* str = xml->stack + xml->sc - sizeof(int) - size - sizeof(uint8_t);
	xml->sc -= (sizeof(int) + size + sizeof(uint8_t));
	return str;
}

inline void xml__push_str(xml_t* xml, const char* str, uint8_t postfix) {
	size_t len = strlen(str);
	xml__push(xml, str, len + sizeof(uint8_t));
	xml__push(xml, &(uint32_t){ len + sizeof(uint8_t) }, sizeof(uint32_t));
	xml__push(xml, &(uint8_t){ postfix }, sizeof(uint8_t));
}

inline char xml__toupper(char c)
{
	if (c >= 'a' && c <= 'z') return 'A' - 'a' + c;
	return c;
}

void xml__restore_xml_space_stack(xml_t* xml)
{
	if (xml->xml_space_count > 0) {
		struct xml__xml_space* xml_space = &xml->xml_space_stack[xml->xml_space_count - 1];
		if (xml_space->level == xml->level) {
			if (xml_space->preserve > 0 && xml->xml_space_count > 1) {
				xml->flags |= (1 << FLAG_PRESERVE);
			}
			else {
				xml->flags &= ~(1 << FLAG_PRESERVE);
			}
			xml->xml_space_count--;
		}
	}
	xml->level--;
}

inline int xml__nextch(xml_t* xml)
{
	int c = fgetc(xml->fp);

	if (c == EOF) {
		if (feof(xml->fp)) {
			xml__push(xml, xml__error_unexpected_end_of_file, sizeof(xml__error_unexpected_end_of_file));
			xml__push(xml, &(uint32_t){ sizeof(xml__error_unexpected_end_of_file) }, sizeof(uint32_t));
			xml__push(xml, &(uint8_t){ 'e' }, sizeof(uint8_t));
		}
		else {
			uint32_t sc = xml->sc;
			xml__push(xml, xml__error_while_reading_file, sizeof(xml__error_while_reading_file));
			xml__push(xml, &(uint32_t){ sizeof(xml__error_while_reading_file) }, sizeof(uint32_t));
			xml__push(xml, &(uint8_t){ 'e' }, sizeof(uint8_t));
		}
		return 0;
	}
	else {
		if (c == '\n') {
			xml->row++;
			xml->col = 1;
		}
		else {
			xml->col++;
		}
	}
	xml->ch = c;
	return 1;
}

inline int xml__isalnum(char ch)
{
	if (ch >= 'a' && ch <= 'z') return 1;
	else if (ch >= 'A' && ch <= 'Z') return 1;
	else if (ch >= '0' && ch <= '9') return 1;
	else return 0;
}

xml_t* xml_fopen(const char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		return NULL;
	}

	xml_t* xml = malloc(sizeof(xml_t));
	if (xml == NULL) {
		fprintf(stderr, "PANIC: Failed to allocate memory for svg structure.");
		exit(-1);
	}

	xml->stack = calloc(STACK_SIZE, sizeof(uint8_t));
	if (xml->stack == NULL) {
		fprintf(stderr, "PANIC: Failed to allocate memory for svg stack.");
		exit(-1);
	}

	xml->lc = xml__start;
	xml->col = 1;
	xml->row = 1;
	xml->sc = 0;
	xml->fp = fp;
	xml->level = 0;
	xml->flags = (1 << FLAG_TRIM) | (1 << FLAG_COLLAPSE);
	xml->xml_space_count = 0;

	return xml;
}

xml_token_t xml_next_token(xml_t* xml)
{
jp: switch (xml->lc) {
	LABEL(xml__start);
	NEXTCH();
	if (xml->ch == 0xEF) for (int i = 0; i < 3; i++) NEXTCH(); // Ignore BOM
	xml->col = 1;
	CALL(xml__c1, xml__padding);
	if (xml->ch == '<') {
		NEXTCH();
		if (xml->ch == '?') {
			NEXTCH();
			CALL(xml__c2, xml__name);
			if(strcmp("xml", xml__pop_str(xml)) != 0) JMP(xml__error);
			CALL(xml__c3, xml__padding);
			while (xml->ch != '?') {
				CALL(xml__c9, xml__attr);
				TOK(xml__t2, XML_DECLARATION);
				xml__pop_str(xml);
				xml__pop_str(xml);
				CALL(xml__c10, xml__padding);
			}
			NEXTCH();
			if (xml->ch != '>') JMP(xml__error);
			NEXTCH();
			CALL(xml__c11, xml__padding);
			if (xml->ch != '<') JMP(xml__error);
			NEXTCH();
		}
		xml->ra = 0;
		while(xml->ra == 0) {
			CALL(xml__c12, xml__tag);
		}
	}
	else JMP(xml__error);
	for (;;) TOK(xml__t1, XML_DOCUMENT_END);

	LABEL(xml__padding);
	while (xml->ch == ' ' || xml->ch == '\r' || xml->ch == '\n' || xml->ch == '\t' || xml->ch == '\f') NEXTCH();
	RET();

	LABEL(xml__name);
	{
		enum xml__label lc = *((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
		int sc = xml->sc;
		if (xml->ch > 127 || xml__isalnum(xml->ch) || xml->ch == '_') {
			xml__push(xml, &(uint8_t){ xml->ch }, sizeof(uint8_t));
			NEXTCH();
		}
		else JMP(xml__error);
		while (xml->ch > 127 || xml__isalnum(xml->ch) || xml->ch == '_' || xml->ch == ':' || xml->ch == '-' || xml->ch == '.') {
			xml__push(xml, &(uint8_t){ xml->ch }, sizeof(uint8_t));
			NEXTCH();
		}
		xml__push(xml, &(uint8_t){ '\0' }, sizeof(uint8_t));
		xml__push(xml, &(int){ xml->sc - sc }, sizeof(int));
		xml__push(xml, &(uint8_t){ 'n' }, sizeof(uint8_t));
		xml__push(xml, &(enum xml_label){ lc }, sizeof(enum xml__label));
		RET();
	}

	LABEL(xml__value);
	{
		xml->ra = (int)*((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
		xml->rb = xml->ch;
		if (xml->ch != '\'' && xml->ch != '\"') JMP(xml__error);
		NEXTCH();
		xml->rc = xml->sc;
		while (xml->ch != xml->rb) {
			if (xml->ch == '&') {
				CALL(xml__c22, xml__escape_sign);
			}
			else {
				xml__push(xml, &(uint8_t){ xml->ch }, sizeof(uint8_t));
				NEXTCH();
			}
		}
		NEXTCH();
		xml__push(xml, &(uint8_t){ '\0' }, sizeof(uint8_t));
		xml__push(xml, &(int){ xml->sc - xml->rc }, sizeof(int));
		xml__push(xml, &(uint8_t){ 'v' }, sizeof(uint8_t));
		xml__push(xml, &(enum xml_label){ (enum xml_label)xml->ra }, sizeof(enum xml__label));
		RET();
	}

	LABEL(xml__attr);
	{
		enum xml__label lc;
		CALL(xml__c4, xml__name);
		const char* name = xml__pop_str(xml);
		lc = *((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
		xml__push_str(xml, name, 'n');
		xml__push(xml, &(enum xml__label){ lc }, sizeof(enum xml__label));
		CALL(xml__c5, xml__padding);
		if (xml->ch == '=') {
			NEXTCH();
			CALL(xml__c6, xml__padding);
			CALL(xml__c7, xml__value);
			const char* name = xml__pop_str(xml);
			lc = *((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
			xml__push_str(xml, name, 'v');
			xml__push(xml, &(enum xml__label){ lc }, sizeof(enum xml__label));
			CALL(xml__c8, xml__padding);
		}
		else {
			lc = *((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
			xml__push(xml, "1", sizeof("1"));
			xml__push(xml, &(int){ 2 }, sizeof(int));
			xml__push(xml, &(uint8_t){'v'}, sizeof(uint8_t));
			xml__push(xml, &(enum xml__label){ lc }, sizeof(enum xml__label));
		}
		RET();
	}

	LABEL(xml__tag);
	{
		if (xml->ch == '!') {
			NEXTCH();
			if (xml->ch == '-') {
				NEXTCH();
				if (xml->ch == '-') {
					int m1 = xml->ch;
					NEXTCH();
					int m2 = xml->ch;
					while (!(m1 == '-' && m2 == '-' && xml->ch == '>')) {
						m1 = m2;
						m2 = xml->ch;
						NEXTCH();
					}
					NEXTCH();
					CALL(xml__c20, xml__padding);
					if (xml->ch != '<') JMP(xml__error);
					NEXTCH();
					xml->ra = 0;
					RET();
				}
				else JMP(xml__error);
			}
			else if (xml->ch == '[') {
				NEXTCH();
				int sc = xml->sc;
				while (xml->ch != '[') {
					xml__push(xml, &(uint8_t){ xml->ch }, sizeof(uint8_t));
					NEXTCH();
				}
				NEXTCH();
				xml__push(xml, &(uint8_t){ '\0' }, sizeof(uint8_t));
				int cnt = xml->sc - sc;
				xml->sc = sc;
				if (strcmp(&xml->stack[xml->sc], "CDATA") == 0) {
					int m1 = xml->ch;
					NEXTCH();
					int m2 = xml->ch;
					NEXTCH();
					sc = xml->sc;
					while (!(m1 == ']' && m2 == ']' && xml->ch == '>')) {
						xml__push(xml, &(uint8_t){ m1 }, sizeof(uint8_t));
						m1 = m2;
						m2 = xml->ch;
						NEXTCH();
					}
					xml__push(xml, &(uint8_t){ '\0' }, sizeof(uint8_t));
					xml__push(xml, &(int){ xml->sc - sc }, sizeof(int));
					xml__push(xml, &(uint8_t){ 't' }, sizeof(uint8_t));
					if ((xml->sc - sc) > (sizeof(int) + 2 * sizeof(uint8_t))) TOK(xml__t8, XML_TEXT);
					xml__pop_str(xml);
					xml->ra = 0;
					RET();
				}
				else JMP(xml__error);
			}
			else if (xml->ch == 'D') {
				const char doctype[] = "OCTYPE";
				for (int i = 0; i < sizeof(doctype) - 1; i++) {
					NEXTCH();
					if (xml->ch != doctype[i]) JMP(xml__error);
				}
				NEXTCH();
				while (xml->ch != '>') {
					if (xml->ch == '[') {
						NEXTCH();
						while(xml->ch != ']') {
							NEXTCH();
						}
					}
					NEXTCH();
				}
				NEXTCH();
				CALL(xml__c21, xml__padding);
				if (xml->ch != '<') JMP(xml__error);
				NEXTCH();
				xml->ra = 0;
				RET();
			}
			else JMP(xml__error);
		}
		CALL(xml__c13, xml__name);
		xml->level++;
		TOK(xml__t3, XML_TAG_START);
		CALL(xml__c14, xml__padding);
		while (xml->ch != '>' && xml->ch != '/') {
			CALL(xml__c15, xml__attr);
			if (strcmp(xml_get_name(xml), "xml:space") == 0) {
				if (xml->xml_space_count > (XML_SPACE_STACK_SIZE - 1)) {
					fprintf(stderr, "PANIC Maximum %d number of xml:space attribute have been reached.", XML_SPACE_STACK_SIZE);
					exit(-1);
				}
				if (strcmp(xml_get_value(xml), "preserve") == 0) {
					xml->xml_space_stack[xml->xml_space_count++] = (struct xml__xml_space){
						.level = xml->level,
						.preserve = (xml->flags & (1 << FLAG_PRESERVE)) > 0
					};
					xml->flags |= (1 << FLAG_PRESERVE);
				}
				else {
					xml->flags &= ~(1 << FLAG_PRESERVE);
					xml->xml_space_stack[xml->xml_space_count++] = (struct xml__xml_space){
						.level = xml->level,
						.preserve = (xml->flags & (1 << FLAG_PRESERVE)) > 0
					};
				}
			}
			else {
				TOK(xml__t4, XML_ATTRIBUTE);
			}
			xml__pop_str(xml);
			xml__pop_str(xml);
			CALL(xml__c16, xml__padding);
		}
		if (xml->ch == '/') {
			TOK(xml__t5, XML_TAG_END);
			xml__restore_xml_space_stack(xml);
			xml__pop_str(xml);
			RET();
		}
		xml__pop_str(xml);
		xml->ra = xml->sc;
		LABEL(xml__tag_loop);
		NEXTCH();
		if (xml->flags & (1 << FLAG_TRIM) && ((xml->flags & (1 << FLAG_PRESERVE)) == 0)) {
			CALL(xml__c24, xml__padding); // Padding
		}
		xml->rb = '\0';
		while (xml->ch != '<') {
			if (xml->ch == '&') {
				CALL(xml__c23, xml__escape_sign);
				xml->rb = *(uint8_t*)xml__peek(xml, sizeof(uint8_t), 0);
			}
			else {
				if (xml->flags & (1 << FLAG_COLLAPSE) && ((xml->flags & (1 << FLAG_PRESERVE)) == 0)) {
					if (xml->ch == '\n' || xml->ch == '\r' || xml->ch == '\t') {
						xml->ch = ' ';
					}
					if (!(xml->ch == ' ' && xml->rb == ' ')) {
						xml__push(xml, &(uint8_t){ xml->ch }, sizeof(uint8_t));
					}
					xml->rb = xml->ch;
				}
				else {
					xml__push(xml, &(uint8_t){ xml->ch }, sizeof(uint8_t));
				}
				NEXTCH();
			}
		}
		if (xml->flags & (1 << FLAG_TRIM) && ((xml->flags & (1 << FLAG_PRESERVE)) == 0) && xml->sc != xml->ra) {
			char ch = xml->stack[xml->sc - 1];
			while (xml->sc > 0 && (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\t')) {
				ch = xml->stack[--(xml->sc) - 1];
			}
		}
		NEXTCH();
		if (xml->ch == '/') {
			xml__push(xml, &(uint8_t){ '\0' }, sizeof(uint8_t));
			xml__push(xml, &(int){ xml->sc - xml->ra }, sizeof(int));
			xml__push(xml, &(uint8_t){ 't' }, sizeof(uint8_t));
			if ((xml->sc - xml->ra) > (sizeof(int) + 2*sizeof(uint8_t))) TOK(xml__t6, XML_TEXT);
			xml__pop_str(xml);
			NEXTCH();
			CALL(xml__c18, xml__name);
			TOK(xml__t7, XML_TAG_END);
			xml__restore_xml_space_stack(xml);
			xml__pop_str(xml);
			CALL(xml__c17, xml__padding);
			if (xml->ch != '>') JMP(xml__error);
			xml->ra = 1;
			RET();
		}
		else {
			xml__push(xml, &(int){ xml->ra }, sizeof(int));
			CALL(xml__c19, xml__tag);
			xml->ra = *(int*)xml__pop(xml, sizeof(int));
			NEXTCH();
			JMP(xml__tag_loop);
		}
	}

	LABEL(xml__escape_sign);
	{
		enum xml__label lc = *((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
		int sc = xml->sc;
		uint8_t base = ' ';
		NEXTCH();
		if (xml->ch == '#') {
			NEXTCH();
			base = 'd';
			if (xml->ch == 'x') {
				base = 'x';
				NEXTCH();
			}
		}
		while (xml->ch != ';') {
			xml__push(xml, &(uint8_t){ xml->ch }, sizeof(uint8_t));
			NEXTCH();
		}
		NEXTCH();
		xml__push(xml, &(uint8_t){ '\0' }, sizeof(uint8_t));
		int cnt = xml->sc - sc;
		xml->sc -= cnt;
		if (base == 'd') {
			if (cnt == 2) {
				xml__push(xml, &(uint8_t){ xml->stack[xml->sc] - '0' }, sizeof(uint8_t));
			}
			else if (cnt == 3) {
				xml__push(xml, &(uint8_t){ (xml->stack[xml->sc] - '0') * 10 + xml->stack[xml->sc + 1] - '0' }, sizeof(uint8_t));
			}
			else if (cnt == 4) {
				xml__push(xml, &(uint8_t){ (xml->stack[xml->sc] - '0') * 100 + (xml->stack[xml->sc + 1] - '0') * 10 + xml->stack[xml->sc + 2] - '0' }, sizeof(uint8_t));
			}
			else JMP(9000);
		}
		else if (base == 'x') {
			if (cnt == 2) {
				char a = xml__toupper(xml->stack[xml->sc]);
				xml__push(xml, &(uint8_t){ a >= 'A' ? (a - 'A' + 10) : (a - '0') }, sizeof(uint8_t));
			}
			else if (cnt == 3) {
				char a = xml__toupper(xml->stack[xml->sc]);
				char b = xml__toupper(xml->stack[xml->sc + 1]);
				xml__push(xml, &(uint8_t){ (a >= 'A' ? (a - 'A' + 10) : (a - '0') << 4) + (b >= 'A' ? (b - 'A' + 10) : (b - '0')) }, sizeof(uint8_t));
			}
			else JMP(9000);
		}
		else if (strcmp(&xml->stack[xml->sc], "amp") == 0) {
			xml__push(xml, &(uint8_t){ '&' }, sizeof(uint8_t));
		}
		else if (strcmp(&xml->stack[xml->sc], "apos") == 0) {
			xml__push(xml, &(uint8_t){ '\'' }, sizeof(uint8_t));
		}
		else if (strcmp(&xml->stack[xml->sc], "lt") == 0) {
			xml__push(xml, &(uint8_t){ '<' }, sizeof(uint8_t));
		}
		else if(strcmp(&xml->stack[xml->sc], "gt") == 0) {
			xml__push(xml, &(uint8_t){ '>' }, sizeof(uint8_t));
		}
		else if (strcmp(&xml->stack[xml->sc], "quot") == 0) {
			xml__push(xml, &(uint8_t){ '\"' }, sizeof(uint8_t));
		}
		else JMP(xml__error);
		xml__push(xml, &(enum xml_label){ lc }, sizeof(enum xml__label));
		RET();
	}

	LABEL(xml__error);
	{
		char buf[32];
		int sc = xml->sc;
		xml__push(xml, xml__error_prefix, sizeof(xml__error_prefix) - 1);
		const char* rowstr = _itoa(xml->row, buf, 10);
		xml__push(xml, rowstr, strlen(rowstr));
		xml__push(xml, &(uint8_t){','}, sizeof(uint8_t));
		const char* colstr = _itoa(xml->col, buf, 10);
		xml__push(xml, colstr, strlen(colstr));
		xml__push(xml, xml__unexpected_sign, sizeof(xml__unexpected_sign));
		xml__push(xml, &(uint32_t){ xml->sc - sc }, sizeof(uint32_t));
		xml__push(xml, &(uint8_t){ 'e' }, sizeof(uint8_t));
		for (;;) TOK(xml__error_loop, XML_ERROR);
	}}
	return XML_ERROR;
}

const char* xml_get_error(xml_t* xml) {
	if (xml->stack[xml->sc - sizeof(uint8_t)] == 'e') {
		uint32_t cnt = *(int*)xml__peek(xml, sizeof(int), sizeof(uint8_t));
		return &xml->stack[xml->sc - cnt - sizeof(int) - sizeof(uint8_t)];
	}
	return NULL;
}

const char* xml_get_name(xml_t* xml) {
	if (xml->stack[xml->sc - sizeof(uint8_t)] == 'n') {
		uint32_t cnt = *(int*)xml__peek(xml, sizeof(int), sizeof(uint8_t));
		return &xml->stack[xml->sc - cnt - sizeof(int) - sizeof(uint8_t)];
	}
	else if (xml->stack[xml->sc - sizeof(uint8_t)] == 'v') {
		uint32_t cnt = *((int*)xml__peek(xml, sizeof(int), sizeof(uint8_t)));
		cnt += *((int*)xml__peek(xml, sizeof(int), cnt + sizeof(int) + 2*sizeof(uint8_t)));
		return &xml->stack[xml->sc - cnt - 2*sizeof(int) - 2*sizeof(uint8_t)];
	}
	return NULL;
}

const char* xml_get_value(xml_t* xml) {
	if (xml->stack[xml->sc - sizeof(uint8_t)] == 'v') {
		uint32_t cnt = *(int*)xml__peek(xml, sizeof(int), sizeof(uint8_t));
		return &xml->stack[xml->sc - cnt - sizeof(int) - sizeof(uint8_t)];
	}
	return NULL;
}

const char* xml_get_text(xml_t* xml)
{
	if (xml->stack[xml->sc - sizeof(uint8_t)] == 't') {
		uint32_t cnt = *(int*)xml__peek(xml, sizeof(int), sizeof(uint8_t));
		return &xml->stack[xml->sc - cnt - sizeof(int) - sizeof(uint8_t)];
	}
	return NULL;
}

int xml_get_trim(xml_t* xml)
{
	return (xml->flags & (1 << FLAG_TRIM)) > 0;
}

int xml_get_collapse(xml_t* xml)
{
	return (xml->flags & (1 << FLAG_COLLAPSE)) > 0;
}

void xml_set_trim(xml_t* xml, int enable)
{
	if (enable > 0) {
		xml->flags |= 1 << FLAG_TRIM;
	}
	else {
		xml->flags &= ~(1 << FLAG_TRIM);
	}
}

void xml_set_collapse(xml_t* xml, int enable)
{
	if (enable > 0) {
		xml->flags |= 1 << FLAG_COLLAPSE;
	}
	else {
		xml->flags &= ~(1 << FLAG_COLLAPSE);
	}
}

void xml_close(xml_t* xml)
{
	fclose(xml->fp);
	free(xml->stack);
	free(xml);
}

int main()
{
	xml_t* test = xml_fopen("test.xml");
	if (test == NULL) {
		fprintf(stderr, "Failed to open the file test.xml.\n");
		exit(-1);
	}

	xml_token_t tok = xml_next_token(test);
	while (tok != XML_DOCUMENT_END) {
		switch (tok) {
		case XML_DECLARATION:
			printf("Declaration: %s: \'%s\'\n", xml_get_name(test), xml_get_value(test));
			break;
		case XML_TAG_START:
			printf("Tag start: %s\n", xml_get_name(test));
			break;
		case XML_TAG_END:
			printf("Tag end: %s\n", xml_get_name(test));
			break;
		case XML_TEXT:
			printf("Text: \"%s\"\n", xml_get_text(test));
			break;
		case XML_ATTRIBUTE:
			printf("Attribute: %s: \'%s\'\n", xml_get_name(test), xml_get_value(test));
			break;
		case XML_ERROR:
			fprintf(stderr, "%s\n", xml_get_error(test));
			exit(-1);
		}
		tok = xml_next_token(test);
	}

	xml_close(test);

	return 0;
}