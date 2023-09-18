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

#define XML_IMPLEMENTATION
#include "xml.h"

#include "print_xml.h"

int main()
{
	xml_t* xml = xml_fopen("test.xml");
	if (xml == NULL) {
		fprintf(stderr, "Failed to open the file xml.xml.\n");
		exit(-1);
	}

	print_xml(xml);

	xml_close(xml);

	return 0;
}