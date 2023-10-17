/* xml-tokenizer.h - v0.1 - public domain data structures - Stefan Elmlund 2023
*
*  This is a single-header-file [STB-style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt)
*  library that tokenize xml file for C (also works in C++)
* 
*  The easiest way to install the library to your C/C++ project is to copy 'n' paste the xml-tokenizer.h
*  to your project and do this in *one* C or C++ file:
*
*    #define XML_TOKENIZER_IMPLEMENTATION
*    #include "xml-tokenizer.h"
*
*  COMPILE-TIME OPTIONS
*
*    #define XML_REALLOC(context,ptr,size) better_realloc
*    #define XML_FREE(context,ptr)         btter_free
*  
*      These defines only need to be set in the file containing XML_TOKENIZER_IMPLEMENTATION    
*
*      By default the stdlib realloc() and free() is used. You can defines your own by
*      defining these symbols. You must either define both, or neither.
*
*      Note that at the moment, 'context' will always be NULL.
*
*    #define XML_FOPEN(fp,filename,mode) better_fopen
*    #define XML_FGETC(fp)               better_fgetc
*    #define XML_FCLOSE(fp)              better_fclose
*
*      These defines only need to be set in the file containing XML_TOKENIZER_IMPLEMENTATION
*
*      By default the stdlib fopen(), fgetc() and free() is used. You can defines you own
*      by defining these symbols. You most either define all three, or neither
*
*  LICENSE
* 
*    Placed in the public domain and also MIT licensed.
*    See end of file for detailed license information.
*  
*  EXAMPLE
*
*    You can find examples how to parser a sample xml file that contains a book catalog using C and C++ (C++11).
*
*      example/parser_catalog.c
*      example/xml_dom.hpp
*/

#ifndef __XML_TOKENIZER_H__
#define __XML_TOKENIZER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

	typedef struct xml__impl xml_t;

	typedef enum {
		XML_DECLARATION,
		XML_START_DOCUMENT, XML_END_DOCUMENT,
		XML_START_TAG, XML_END_TAG,
		XML_START_ATTRIBUTES, XML_END_ATTRIBUTES, XML_ATTRIBUTE,
		XML_TEXT,
		XML_ERROR
	} xml_token_t;

	/** @brief Open a file reading xml.
	*   @param filename Name of the xml file.
	*   @return NULL on failure and a pointer to a xml structure on success.
	*/
	xml_t* xml_fopen(const char* filename);

	/** @brief Read the next token from the xml input.
	*   @param xml Pointer to a pointer to the xml structure.
	*   @return The next token.
	*/
	xml_token_t xml_next_token(xml_t* xml);

	/** @brief Return the name of a tag, can only be read after a XML_DECLARATION, XML_TAG_START, XML_ATTRIBUTE or XML_TAG_END token.
	*   @param xml Pointer to the xml structure.
	*   @return String with a name of a tag.
	*/
	const char* xml_get_name(xml_t* xml);

	/** @brief Return the value of an attribute, can only be read after a XML_DECLARATION or XML_ATTRIBUTE token.
	*   @param xml Pointer to the xml structure.
	*   @return String with the attribute.
	*/
	const char* xml_get_value(xml_t* xml);

	/** @brief Return the text for a tag, can only be read after a XML_TEXT token.
	*   @param xml Pointer to the xml structure.
	*   @return String with text.
	*/
	const char* xml_get_text(xml_t* xml);

	/** @brief Return a string with an error, can only be read after a XML_ERROR token.
	*   @param xml Pointer to the xml structure.
	*   @return String with the error message.
	*/
	const char* xml_get_error(xml_t* xml);

	/** @brief Get trim status.
	*	@param xml Pointer to a xml structure.
	*   @return value > 0 if enabled else it's disabled.
	*/
	int xml_get_trim(xml_t* xml);

	/** @bried Get collapse status.
	*   @param xml Pointer to a xml structure
	*   @return value > 0 if enabled else it is disabled.
	*/
	int xml_get_collapse(xml_t* xml);

	/** @brief Set trim status. If set to true, leading and trailing white-spaces for the text are removed.
	*   @param xml Pointer to a xml a structure
	*   @param value If value > 0 then trim is enabled else it is disabled.
	*/
	void xml_set_trim(xml_t* xml, int enable);

	/** @brief Set collapse status, If set to true, line-feed, carrage-return and tab are replace by space. Muliple spaces are collapsed to one space.
	*   @param xml Pointer to a xml structure.
	*   @return value If value > 0 then collapse is enabled else it is disabled.
	*/
	void xml_set_collapse(xml_t* xml, int enable);

	/** @brief Close the xml file and free memory for the xml structure
	*   @param xml Pointer to the xml structure.
	*/
	void xml_close(xml_t* xml);

#ifdef XML_TOKENIZER_IMPLEMENTATION

#if defined(XML_REALLOC) && !defined(XML_FREE) || !defined(XML_REALLOC) && defined(XML_FREE)
#error "You must define both XML_REALLOC and XML_FREE, or neither."
#endif
#if !defined(XML_REALLOC) && !defined(XML_FREE)
#include <stdlib.h>
#define XML_REALLOC(c,p,s) realloc(p,s)
#define XML_FREE(c,p)      free(p)
#endif

#if defined(XML_FOPEN) && !defined(XML_FGETC) || defined(XML_FOPEN) && !defined(XML_FCLOSE)
#error "You must define both XML_FOPEN, XML_FGETC and XML_FCLOSE, or neither."
#endif

#if defined(XML_FGETC) && !defined(XML_FOPEN) || defined(XML_FGETC) && !defined(XML_FCLOSE)
#error "You must define both XML_FOPEN, XML_FGETC and XML_FCLOSE, or neither."
#endif

#if defined(XML_FCLOSE) && !defined(XML_FOPEN) || defined(XML_FCLOSE) && !defined(XML_FGETC)
#error "You must define both XML_FOPEN, XML_FGETC and XML_FCLOSE, or neither."
#endif

#if !defined(XML_FOPEN) && !defined(XML_FGETC) && !defined(XML_FCLOSE)
#ifdef _MSC_VER
#define XML_FOPEN(fp,filename,mode) fopen_s(&(fp),filename,mode)
#else
#define XML_FOPEN(fp,filename,mode) (((fp=fopen(filename,mode))==NULL)?(feof(fp)||ferror(fp)):(0))
#endif
#define XML_FGETC(fp) fgetc(fp)
#define XML_FCLOSE(fp) fclose(fp)
#endif

#define STACK_SIZE (4096)
#define XML_SPACE_STACK_SIZE (32)
#define LABEL(addr) do{case addr:;}while(0);
#define JMP(addr) do{xml->lc=addr;goto jp;}while(0)
#define CALL(ret_addr,call_addr) do{{enum xml__label ret=ret_addr; xml->lc=call_addr;xml__push(xml,&ret,sizeof(enum xml__label));}goto jp;case ret_addr:;}while(0)
#define RET() do{xml->lc=*(enum xml__label*)xml__pop(xml, sizeof(enum xml__label));goto jp;}while(0);
#define TOK(addr,tok) do{xml->lc=addr;return tok;case addr:;}while(0)
#define NEXTCH() do{if(!xml__nextch(xml)) JMP(xml__error_loop);}while(0)
#define FLAG_TRIM (0)
#define FLAG_COLLAPSE (1)
#define FLAG_PRESERVE (2)
#define RET_COMMENT_OR_DOCTYPE (0)
#define RET_TAG_END (1)
#define RET_CDATA (2)

	enum xml__label {
		xml__start,
		xml__padding,
		xml__name,
		xml__value,
		xml__attr,
		xml__tag, xml__tag_loop, xml__tag_loop_no_trim,
		xml__escape_sign,
		xml__error, xml__error_loop,
		xml__c1, xml__c2, xml__c3, xml__c4, xml__c5, xml__c6, xml__c7, xml__c8, xml__c9, xml__c10,
		xml__c11, xml__c12, xml__c13, xml__c14, xml__c15, xml__c16, xml__c17, xml__c18, xml__c19,
		xml__c20, xml__c21, xml__c22, xml__c23, xml__c24,
		xml__l1, xml__l2, xml__l3, xml__l4,
		xml__t1, xml__t2, xml__t3, xml__t4, xml__t5, xml__t6, xml__t7, xml__t8, xml__t9, xml__t10, xml__t11
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
		size_t stack_capacity;
		uint8_t* stack;
	};

	static void xml__push(xml_t* xml, const void* data, size_t size)
	{
		if ((xml->sc + size) > xml->stack_capacity) {
			size_t new_capacity = xml->stack_capacity * 2;
			uint8_t* new_stack = (uint8_t*)XML_REALLOC(NULL, xml->stack, new_capacity);
			if (new_stack == NULL) {
				fprintf(stderr, "PANIC failed to allocate memory for xml_t stack!");
				exit(-1);
			}
			xml->stack = new_stack;
			xml->stack_capacity = new_capacity;
		}
		for (size_t i = 0; i < size; i++) {
			xml->stack[xml->sc++] = ((uint8_t*)data)[i];
		}
	}

	static const void* xml__pop(xml_t* xml, size_t size)
	{
		xml->sc -= (int)size;
		return &(xml->stack[xml->sc]);
	}

	static const void* xml__peek(xml_t* xml, size_t size, size_t index)
	{
		return &(xml->stack[xml->sc - size - index]);
	}

	static const char* xml__peek_str(xml_t* xml)
	{
		int size = *((uint32_t*)&xml->stack[xml->sc - sizeof(int)]);
		return (const char*)(xml->stack + xml->sc - sizeof(uint32_t) - size - 1);
	}

	static const char* xml__pop_str(xml_t* xml) {
		int size = *((int*)&xml->stack[xml->sc - sizeof(int) - sizeof(uint8_t)]);
		const char* str = (const char*)(xml->stack + xml->sc - sizeof(int) - size - sizeof(uint8_t));
		xml->sc -= (sizeof(int) + size + sizeof(uint8_t));
		return str;
	}

	static size_t xml__strlen(const char* str) {
		const char* n = str;
		while (*n != '\0') n++;
		return n - str;
	}

	static void xml__push_str(xml_t* xml, const char* str, uint8_t postfix) {

		int  len = (int)(xml__strlen(str) + sizeof(uint8_t));
		xml__push(xml, str, len);
		xml__push(xml, &len, sizeof(int));
		xml__push(xml, &postfix, sizeof(uint8_t));
	}

	static int xml__strncmp(const char* a, const char* b, size_t n)
	{
		while (n && *a && (*a == *b)) {
			++a; ++b; n--;
		}
		if (n == 0) return 0;
		else return (*(uint8_t*)a - *(uint8_t*)b);
	}

	static char* xml__itoa(char* buf, size_t bufsize, int val, int base)
	{
		size_t i = bufsize - 2;
		buf[bufsize - 1] = '\0';

		for (; val && i; --i, val /= base) {
			buf[i] = "0123456789abcdef"[val % base];
		}

		return &buf[i + 1];
	}

	static char xml__toupper(char c)
	{
		if (c >= 'a' && c <= 'z') return 'A' - 'a' + c;
		return c;
	}

	static void xml__restore_xml_space_stack(xml_t* xml)
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

	static int xml__nextch(xml_t* xml)
	{
		int c = XML_FGETC(xml->fp);

		if (c == EOF) {
			uint8_t postfix = 'e';
			if (feof(xml->fp)) {
				xml__push(xml, xml__error_unexpected_end_of_file, sizeof(xml__error_unexpected_end_of_file));
				int len = (int)sizeof(xml__error_unexpected_end_of_file);
				xml__push(xml, &len, sizeof(int));
				xml__push(xml, &postfix, sizeof(uint8_t));
			}
			else {
				size_t sc = xml->sc;
				xml__push(xml, xml__error_while_reading_file, sizeof(xml__error_while_reading_file));
				int len = (int)sizeof(xml__error_while_reading_file);
				xml__push(xml, &len, sizeof(int));
				xml__push(xml, &postfix, sizeof(uint8_t));
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

	static int xml__isalnum(char ch)
	{
		if (ch >= 'a' && ch <= 'z') return 1;
		else if (ch >= 'A' && ch <= 'Z') return 1;
		else if (ch >= '0' && ch <= '9') return 1;
		else return 0;
	}

	xml_t* xml_fopen(const char* filename)
	{
		FILE* fp = NULL;

		if (XML_FOPEN(fp, filename, "r") != 0) {
			return NULL;
		}

		xml_t* xml = (xml_t*)XML_REALLOC(NULL, NULL, sizeof(xml_t));
		if (xml == NULL) {
			fprintf(stderr, "PANIC: Failed to allocate memory for svg structure.");
			exit(-1);
		}

		xml->stack = (uint8_t*)calloc(STACK_SIZE, sizeof(uint8_t));
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
		xml->stack_capacity = STACK_SIZE;

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
				if (xml__strncmp(xml__pop_str(xml), "xml", 3) != 0) JMP(xml__error);
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
			TOK(xml__t9, XML_START_DOCUMENT);
			xml->ra = RET_COMMENT_OR_DOCTYPE;
			while (xml->ra == RET_COMMENT_OR_DOCTYPE) {
				CALL(xml__c12, xml__tag);
			}
		}
		else JMP(xml__error);
		for (;;) TOK(xml__t1, XML_END_DOCUMENT);

		LABEL(xml__padding);
		while (xml->ch == ' ' || xml->ch == '\r' || xml->ch == '\n' || xml->ch == '\t' || xml->ch == '\f') NEXTCH();
		RET();

		LABEL(xml__name);
		{
			enum xml__label lc = *((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
			int sc = xml->sc;
			if (xml->ch > 127 || xml__isalnum(xml->ch) || xml->ch == '_') {
				uint8_t ch = xml->ch;
				xml__push(xml, &ch, sizeof(uint8_t));
				NEXTCH();
			}
			else JMP(xml__error);
			while (xml->ch > 127 || xml__isalnum(xml->ch) || xml->ch == '_' || xml->ch == ':' || xml->ch == '-' || xml->ch == '.') {
				uint8_t ch = xml->ch;
				xml__push(xml, &ch, sizeof(uint8_t));
				NEXTCH();
			}
			uint8_t n = '\0';
			uint8_t prefix = 'n';
			xml__push(xml, &n, sizeof(uint8_t));
			int len = (int)(xml->sc - sc);
			xml__push(xml, &len, sizeof(int));
			xml__push(xml, &prefix, sizeof(uint8_t));
			xml__push(xml, &lc, sizeof(enum xml__label));
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
					char ch = xml->ch;
					xml__push(xml, &ch, sizeof(uint8_t));
					NEXTCH();
				}
			}
			NEXTCH();
			uint8_t n = '\0';
			uint8_t postfix = 'v';
			enum xml__label lc = (enum xml__label)xml->ra;
			xml__push(xml, &n, sizeof(uint8_t));
			int len = (int)(xml->sc - xml->rc);
			xml__push(xml, &len, sizeof(int));
			xml__push(xml, &postfix, sizeof(uint8_t));
			xml__push(xml, &lc, sizeof(enum xml__label));
			RET();
		}

		LABEL(xml__attr);
		{
			CALL(xml__c4, xml__name);
			{
				const char* name = xml__pop_str(xml);
				enum xml__label lc = *((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
				xml__push_str(xml, name, 'n');
				xml__push(xml, &lc, sizeof(enum xml__label));
			}
			CALL(xml__c5, xml__padding);
			if (xml->ch == '=') {
				NEXTCH();
				CALL(xml__c6, xml__padding);
				CALL(xml__c7, xml__value);
				{
					const char* value = xml__pop_str(xml);
					enum xml__label lc = *((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
					xml__push_str(xml, value, 'v');
					xml__push(xml, &lc, sizeof(enum xml__label));
				}
				CALL(xml__c8, xml__padding);
			}
			else {
				enum xml__label lc = *((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
				int len = 2;
				uint8_t postfix = 'v';
				xml__push(xml, "1", sizeof("1"));
				xml__push(xml, &len, sizeof(int));
				xml__push(xml, &postfix, sizeof(uint8_t));
				xml__push(xml, &lc, sizeof(enum xml__label));
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
						xml->ra = RET_COMMENT_OR_DOCTYPE;
						RET();
					}
					else JMP(xml__error);
				}
				else if (xml->ch == '[') {
					enum xml__label lc = *((enum xml__label*)xml__pop(xml, sizeof(enum xml__label)));
					NEXTCH();
					int sc = xml->sc;
					while (xml->ch != '[') {
						uint8_t ch = xml->ch;
						xml__push(xml, &ch, sizeof(uint8_t));
						NEXTCH();
					}
					NEXTCH();
					uint8_t n = '\0';
					xml__push(xml, &n, sizeof(uint8_t));
					int cnt = xml->sc - sc;
					xml->sc = sc;
					if (xml__strncmp((const char*)&xml->stack[xml->sc], "CDATA", 5) == 0) {
						uint8_t m1 = xml->ch;
						NEXTCH();
						uint8_t m2 = xml->ch;
						NEXTCH();
						sc = xml->sc;
						while (!(m1 == ']' && m2 == ']' && xml->ch == '>')) {
							xml__push(xml, &m1, sizeof(uint8_t));
							m1 = m2;
							m2 = xml->ch;
							NEXTCH();
						}
						uint8_t n = '\0';
						uint8_t postfix = 't';
						enum xml__label llc = lc;
						xml__push(xml, &n, sizeof(uint8_t));
						int len = (int)(xml->sc - sc);
						xml__push(xml, &len, sizeof(int));
						xml__push(xml, &postfix, sizeof(uint8_t));
						xml__push(xml, &llc, sizeof(enum xml__label));
						xml->ra = RET_CDATA;
						RET();
					}
					else JMP(xml__error);
				}
				else if (xml->ch == 'D') {
					{
						const char doctype[] = "OCTYPE";
						for (int i = 0; i < sizeof(doctype) - 1; i++) {
							NEXTCH();
							if (xml->ch != doctype[i]) JMP(xml__error);
						}
					}
					NEXTCH();
					while (xml->ch != '>') {
						if (xml->ch == '[') {
							NEXTCH();
							while (xml->ch != ']') {
								NEXTCH();
							}
						}
						NEXTCH();
					}
					NEXTCH();
					CALL(xml__c21, xml__padding);
					if (xml->ch != '<') JMP(xml__error);
					NEXTCH();
					xml->ra = RET_COMMENT_OR_DOCTYPE;
					RET();
				}
				else JMP(xml__error);
			}
			CALL(xml__c13, xml__name);
			xml->level++;
			TOK(xml__t3, XML_START_TAG);
			CALL(xml__c14, xml__padding);
			TOK(xml__t10, XML_START_ATTRIBUTES);
			while (xml->ch != '>' && xml->ch != '/') {
				CALL(xml__c15, xml__attr);
				if (xml__strncmp(xml_get_name(xml), "xml:space", 9) == 0) {
					if (xml->xml_space_count > (XML_SPACE_STACK_SIZE - 1)) {
						fprintf(stderr, "PANIC Maximum %d number of xml:space attribute have been reached.", XML_SPACE_STACK_SIZE);
						exit(-1);
					}
					if (xml__strncmp(xml_get_value(xml), "preserve", 8) == 0) {
						struct xml__xml_space xsp = { xml->level, (xml->flags & (1 << FLAG_PRESERVE)) > 0 };
						xml->xml_space_stack[xml->xml_space_count++] = xsp;
						xml->flags |= (1 << FLAG_PRESERVE);
					}
					else {
						struct xml__xml_space xsp = { xml->level, (xml->flags & (1 << FLAG_PRESERVE)) > 0 };
						xml->xml_space_stack[xml->xml_space_count++] = xsp;
						xml->flags &= ~(1 << FLAG_PRESERVE);
					}
				}
				else {
					TOK(xml__t4, XML_ATTRIBUTE);
				}
				xml__pop_str(xml);
				xml__pop_str(xml);
				CALL(xml__c16, xml__padding);
			}
			TOK(xml__t11, XML_END_ATTRIBUTES);
			if (xml->ch == '/') {
				TOK(xml__t5, XML_END_TAG);
				xml__restore_xml_space_stack(xml);
				xml__pop_str(xml);
				RET();
			}
			xml__pop_str(xml);
			xml->ra = xml->sc;
			LABEL(xml__tag_loop);
			if (xml->ch == '>') NEXTCH();
			if (xml->flags & (1 << FLAG_TRIM) && ((xml->flags & (1 << FLAG_PRESERVE)) == 0)) {
				CALL(xml__c24, xml__padding); // Padding
			}
			LABEL(xml__tag_loop_no_trim);
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
							uint8_t ch = xml->ch;
							xml__push(xml, &ch, sizeof(uint8_t));
						}
						xml->rb = xml->ch;
					}
					else {
						uint8_t ch = xml->ch;
						xml__push(xml, &ch, sizeof(uint8_t));
					}
					NEXTCH();
				}
			}
			NEXTCH();
			if (xml->ch != '!' && (xml->flags & (1 << FLAG_TRIM) && ((xml->flags & (1 << FLAG_PRESERVE)) == 0) && xml->sc != xml->ra)) {
				char ch = xml->stack[xml->sc - 1];
				while (xml->sc > 0 && (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\t')) {
					ch = xml->stack[--(xml->sc) - 1];
				}
			}
			if (xml->ch == '/') {
				{
					uint8_t n = '\0';
					uint8_t postfix = 't';
					xml__push(xml, &n, sizeof(uint8_t));
					int len = (int)(xml->sc - xml->ra);
					xml__push(xml, &len, sizeof(int));
					xml__push(xml, &postfix, sizeof(uint8_t));
				}
				if (((size_t)xml->sc - xml->ra) > (sizeof(int) + 2 * sizeof(uint8_t))) TOK(xml__t6, XML_TEXT);
				xml__pop_str(xml);
				NEXTCH();
				CALL(xml__c18, xml__name);
				TOK(xml__t7, XML_END_TAG);
				xml__restore_xml_space_stack(xml);
				xml__pop_str(xml);
				CALL(xml__c17, xml__padding);
				if (xml->ch != '>') JMP(xml__error);
				xml->ra = RET_TAG_END;
				RET();
			}
			else {
				xml__push(xml, &xml->ra, sizeof(int));
				CALL(xml__c19, xml__tag);
				if (xml->ra == RET_CDATA) {
					const char* text = xml_get_text(xml);
					xml__pop_str(xml);
					xml->ra = *(int*)xml__pop(xml, sizeof(int));
					while (*text != '\0') {
						xml__push(xml, text, sizeof(uint8_t));
						++text;
					}
					NEXTCH();
					JMP(xml__tag_loop_no_trim);
				}
				else {
					xml->ra = *(int*)xml__pop(xml, sizeof(int));
					NEXTCH();
				}
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
				uint8_t ch = xml->ch;
				xml__push(xml, &ch, sizeof(uint8_t));
				NEXTCH();
			}
			NEXTCH();
			uint8_t n = '\0';
			xml__push(xml, &n, sizeof(uint8_t));
			int cnt = xml->sc - sc;
			xml->sc -= cnt;
			if (base == 'd') {
				if (cnt == 2) {
					uint8_t ch = xml->stack[xml->sc] - '0';
					xml__push(xml, &ch, sizeof(uint8_t));
				}
				else if (cnt == 3) {
					uint8_t ch = (xml->stack[xml->sc] - '0') * 10 + xml->stack[xml->sc + 1] - '0';
					xml__push(xml, &ch, sizeof(uint8_t));
				}
				else if (cnt == 4) {
					uint8_t ch = (xml->stack[xml->sc] - '0') * 100 + (xml->stack[xml->sc + 1] - '0') * 10 + xml->stack[xml->sc + 2] - '0';
					xml__push(xml, &ch, sizeof(uint8_t));
				}
				else JMP(xml__error);
			}
			else if (base == 'x') {
				if (cnt == 2) {
					char a = xml__toupper(xml->stack[xml->sc]);
					uint8_t ch = a >= 'A' ? (a - 'A' + 10) : (a - '0');
					xml__push(xml, &ch, sizeof(uint8_t));
				}
				else if (cnt == 3) {
					char a = xml__toupper(xml->stack[xml->sc]);
					char b = xml__toupper(xml->stack[xml->sc + 1]);
					char ch = (a >= 'A' ? (a - 'A' + 10) : (a - '0') << 4) + (b >= 'A' ? (b - 'A' + 10) : (b - '0'));
					xml__push(xml, &ch, sizeof(uint8_t));
				}
				else JMP(xml__error);
			}
			else if (xml__strncmp((const char*)&xml->stack[xml->sc], "amp", 3) == 0) {
				uint8_t ch = '&';
				xml__push(xml, &ch, sizeof(uint8_t));
			}
			else if (xml__strncmp((const char*)&xml->stack[xml->sc], "apos", 4) == 0) {
				uint8_t ch = '\'';
				xml__push(xml, &ch, sizeof(uint8_t));
			}
			else if (xml__strncmp((const char*)&xml->stack[xml->sc], "lt", 2) == 0) {
				uint8_t ch = '<';
				xml__push(xml, &ch, sizeof(uint8_t));
			}
			else if (xml__strncmp((const char*)&xml->stack[xml->sc], "gt", 2) == 0) {
				uint8_t ch = '>';
				xml__push(xml, &ch, sizeof(uint8_t));
			}
			else if (xml__strncmp((const char*)&xml->stack[xml->sc], "quot", 4) == 0) {
				uint8_t ch = '\"';
				xml__push(xml, &ch, sizeof(uint8_t));
			}
			else JMP(xml__error);
			enum xml_label llc = (enum xml_label)lc;
			xml__push(xml, &llc, sizeof(enum xml__label));
			RET();
		}

		LABEL(xml__error);
		{
			char buf[32];
			int sc = xml->sc;
			uint8_t comma = ',';
			uint8_t prefix = 'e';
			xml__push(xml, xml__error_prefix, sizeof(xml__error_prefix) - 1);
			const char* rowstr = xml__itoa(buf, sizeof(buf), xml->row, 10);
			xml__push(xml, rowstr, xml__strlen(rowstr));
			xml__push(xml, &comma, sizeof(uint8_t));
			const char* colstr = xml__itoa(buf, sizeof(buf), xml->col, 10);
			xml__push(xml, colstr, xml__strlen(colstr));
			xml__push(xml, xml__unexpected_sign, sizeof(xml__unexpected_sign));
			int len = (int)(xml->sc - sc);
			xml__push(xml, &len, sizeof(int));
			xml__push(xml, &prefix, sizeof(uint8_t));
		}
		for (;;) TOK(xml__error_loop, XML_ERROR);
	}
	return XML_ERROR;
	}

	const char* xml_get_error(xml_t* xml) {
		if (xml->stack[xml->sc - sizeof(uint8_t)] == 'e') {
			int cnt = *(int*)xml__peek(xml, sizeof(int), sizeof(uint8_t));
			return (const char*)&xml->stack[(size_t)xml->sc - cnt - sizeof(int) - sizeof(uint8_t)];
		}
		return NULL;
	}

	const char* xml_get_name(xml_t* xml) {
		if (xml->stack[xml->sc - sizeof(uint8_t)] == 'n') {
			uint32_t cnt = *(int*)xml__peek(xml, sizeof(int), sizeof(uint8_t));
			return (const char*)&xml->stack[(size_t)xml->sc - cnt - sizeof(int) - sizeof(uint8_t)];
		}
		else if (xml->stack[xml->sc - sizeof(uint8_t)] == 'v') {
			uint32_t cnt = *((int*)xml__peek(xml, sizeof(int), sizeof(uint8_t)));
			cnt += *((int*)xml__peek(xml, sizeof(int), cnt + sizeof(int) + 2 * sizeof(uint8_t)));
			return (const char*)&xml->stack[(size_t)xml->sc - cnt - 2 * sizeof(int) - 2 * sizeof(uint8_t)];
		}
		return NULL;
	}

	const char* xml_get_value(xml_t* xml) {
		if (xml->stack[xml->sc - sizeof(uint8_t)] == 'v') {
			uint32_t cnt = *(int*)xml__peek(xml, sizeof(int), sizeof(uint8_t));
			return (const char*)&xml->stack[(size_t)xml->sc - cnt - sizeof(int) - sizeof(uint8_t)];
		}
		return NULL;
	}

	const char* xml_get_text(xml_t* xml)
	{
		if (xml->stack[xml->sc - sizeof(uint8_t)] == 't') {
			uint32_t cnt = *(int*)xml__peek(xml, sizeof(int), sizeof(uint8_t));
			return (const char*)&xml->stack[(size_t)xml->sc - cnt - sizeof(int) - sizeof(uint8_t)];
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
		XML_FCLOSE(xml->fp);
		XML_FREE(NULL, xml->stack);
		XML_FREE(NULL, xml);
	}

#undef STACK_SIZE
#undef XML_SPACE_STACK_SIZE
#undef LABEL
#undef JMP
#undef CALL
#undef RET
#undef TOK
#undef NEXTCH
#undef FLAG_TRIM
#undef FLAG_COLLAPSE
#undef FLAG_PRESERVE
#undef RET_COMMENT_OR_DOCTYPE
#undef RET_TAG_END
#undef RET_CDATA
#undef XML_PARSER_IMPLEMENTATION

#endif
#ifdef __cplusplus
}
#endif
#endif

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2023 Stefan Elmlund
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/