#pragma once

#ifdef __cplusplus 
extern "C" {
#endif

#define MAX_ID_STR_LEN (7)
#define MAX_AUTHOR_STR_LEN (63)
#define MAX_TITLE_STR_LEN (31)
#define MAX_GENRE_STR_LEN (15)
#define MAX_PRICE_STR_LEN (7)
#define MAX_PUBLIC_DATA_STR_LEN (10)
#define MAX_DESCRIPTION_STR_LEN (127)

typedef struct {
	char id[MAX_ID_STR_LEN + 1];
	char author[MAX_AUTHOR_STR_LEN + 1];
	char title[MAX_TITLE_STR_LEN + 1];
	char genre[MAX_GENRE_STR_LEN + 1];
	char price[MAX_PRICE_STR_LEN + 1];
	char public_date[MAX_PUBLIC_DATA_STR_LEN + 1];
	char description[MAX_DESCRIPTION_STR_LEN + 1];
} book_t;

size_t read_catalog(const char* catalog_filename, book_t* catalog, size_t max_books);

#ifdef __cplusplus
}
#endif
