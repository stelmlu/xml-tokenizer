#include <stdio.h>

#include "xml.h"

void print_xml(xml_t* xml)
{
	xml_token_t tok = xml_next_token(xml);
	while (tok != XML_DOCUMENT_END) {
		switch (tok) {
		case XML_DECLARATION:
			printf("Declaration: %s: \'%s\'\n", xml_get_name(xml), xml_get_value(xml));
			break;
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
}