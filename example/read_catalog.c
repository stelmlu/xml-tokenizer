#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define XML_TOKENIZER_IMPLEMENTATION
#include "../xml-tokenizer.h"

#include "read_catalog.h"


static void panic_parse_xml_failed(xml_t* xml) {
	fprintf(stderr, "%s\n", xml_get_error(xml));
	exit(-1);
}

static xml_token_t next_text_token(xml_token_t tok, xml_t* xml)
{
	while (tok != XML_TEXT) {
		if (tok == XML_ERROR) panic_parse_xml_failed(xml);
		tok = xml_next_token(xml);
	}
	return tok;
}

size_t read_catalog(const char* catalog_filename, book_t* catalog, size_t max_books)
{
	size_t book_index = 0;

	xml_t* test = xml_fopen(catalog_filename);

	if(test == NULL) {
		printf("Failed to open: %s\n", catalog_filename);
		exit(-1);
	}

	for (xml_token_t tok = xml_next_token(test); tok != XML_END_DOCUMENT; )
	{
		while (!(tok == XML_END_TAG && strcmp(xml_get_name(test), "catalog") == 0)) {
			if (tok == XML_START_TAG && strcmp(xml_get_name(test), "book") == 0) {
				book_t book = { 0 };

				// Read the attributes from the book
				while (tok != XML_END_ATTRIBUTES) {
					if (tok == XML_ATTRIBUTE && strcmp(xml_get_name(test), "id") == 0) {
						strncpy(book.id, xml_get_value(test), MAX_ID_STR_LEN);
					}
					else if (tok == XML_ERROR) panic_parse_xml_failed(test);
					tok = xml_next_token(test);
				}

				// Read the for the book
				while (!(tok == XML_END_TAG && strcmp(xml_get_name(test), "book") == 0)) {
					if (tok == XML_START_TAG && strcmp(xml_get_name(test), "author") == 0) {
						tok = next_text_token(tok, test);
						strncpy(book.author, xml_get_text(test), MAX_AUTHOR_STR_LEN);
					}
					else if (tok == XML_START_TAG && strcmp(xml_get_name(test), "title") == 0) {
						tok = next_text_token(tok, test);
						strncpy(book.title, xml_get_text(test), MAX_TITLE_STR_LEN);
					}
					else if (tok == XML_START_TAG && strcmp(xml_get_name(test), "genre") == 0) {
						tok = next_text_token(tok, test);
						strncpy(book.genre, xml_get_text(test), MAX_GENRE_STR_LEN);
					}
					else if (tok == XML_START_TAG && strcmp(xml_get_name(test), "price") == 0) {
						tok = next_text_token(tok, test);
						strncpy(book.price, xml_get_text(test), MAX_PRICE_STR_LEN);
					}
					else if (tok == XML_START_TAG && strcmp(xml_get_name(test), "publish_date") == 0) {
						tok = next_text_token(tok, test);
						strncpy(book.public_date, xml_get_text(test), MAX_PUBLIC_DATA_STR_LEN);
					}
					else if (tok == XML_START_TAG && strcmp(xml_get_name(test), "description") == 0) {
						tok = next_text_token(tok, test);
						strncpy(book.description, xml_get_text(test), MAX_DESCRIPTION_STR_LEN);
					}
					else if (tok == XML_ERROR) panic_parse_xml_failed(test);
					tok = xml_next_token(test);
				}

				if (book_index < max_books) {
					memcpy(&catalog[book_index], &book, sizeof(book_t));
				}
				else {
					fprintf(stderr, "PANIC: Array with books is too small to contain the books in %s!\n", catalog_filename);
					exit(-1);
				}

				book_index++;
			}
			else if (tok == XML_ERROR) panic_parse_xml_failed(test);
			tok = xml_next_token(test);
		}

		tok = xml_next_token(test);
	}

	xml_close(test);

	return book_index;
}
