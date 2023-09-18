#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// TODO: Support escape signs (done)
// TODO: Reallocate stack on the stack overflow, instead of overflow error (done)
// TODO: Ignore <!DOCTYPE ... > tags. (done)
// TODO: Support trim, collapse and xml:space (done)
// TODO: Create custom function for strcmp, itoa and toupper. (done)
// TODO: Function document (done).
// TODO: Add to a own header file. (done)
// TODO: README.md
// TODO: xml with different xml files.
// BUG: Trim away space after a CDATA, ex: USA <![CDATA[(USA)]]> sould be -> "USA (USA)" and NOT "USA(USA)" (done)
// TODO: Fix warning when compiling for x64 code. (done)
// TODO: Compile with C++ (done)

#define XML_IMPLEMENTATION
#include "xml.h"

int main()
{
	xml_t* xml = xml_fopen("test.xml");
	if (xml == NULL) {
		fprintf(stderr, "Failed to open the file xml.xml.\n");
		exit(-1);
	}

	xml_token_t tok = xml_next_token(xml);
	while (tok != XML_DOCUMENT_END) {
		switch (tok) {
		case XML_DECLARATION: {
			const char* name = xml_get_name(xml);
			const char* value = xml_get_value(xml);
			printf("Declaration: %s: \'%s\'\n", name, value);
		}	break;
		case XML_TAG_START:
			printf("Tag start: %s\n", xml_get_name(xml));
			break;
		case XML_TAG_END:
			printf("Tag end: %s\n", xml_get_name(xml));
			break;
		case XML_TEXT:
			printf("Text: \"%s\"\n", xml_get_text(xml));
			break;
		case XML_ATTRIBUTE:
			printf("Attribute: %s: \'%s\'\n", xml_get_name(xml), xml_get_value(xml));
			break;
		case XML_ERROR:
			fprintf(stderr, "%s\n", xml_get_error(xml));
			exit(-1);
		}
		tok = xml_next_token(xml);
	}

	xml_close(xml);

	return 0;
}