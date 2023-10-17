#include <stdio.h>

#include <iostream>

#include "example/read_catalog.h"
#include "example/xml_dom.hpp"

int main()
{
	/*
	*  Example of parse the book catalog with C
	*/
	constexpr size_t max_books_in_catalog = 32;

	book_t catalog[max_books_in_catalog];
	size_t number_of_books = read_catalog("book_catalog.xml", catalog, max_books_in_catalog);

	std::cout << "*******************************************************\n";
	std::cout << "*                                                     *\n";
	std::cout << "*   Example of parser the book_catalog.xml using C.   *\n";
	std::cout << "*                                                     *\n";
	std::cout << "*******************************************************\n\n";

	for (size_t i = 0; i < number_of_books; i++) {
		std::cout << "book: " << catalog[i].id << "\n";
		std::cout << "  author: " << catalog[i].author << "\n";
		std::cout << "  title: " << catalog[i].title << "\n";
		std::cout << "  genre: " << catalog[i].genre << "\n";
		std::cout << "  price: " << catalog[i].price << "\n";
		std::cout << "  public_data: " << catalog[i].public_date << "\n";
		std::cout << "  description: " << catalog[i].description << "\n";
	}

	/*
	*  Example of parse the book catalog with C++11
	*/
	std::cout << "\n\n";
	std::cout << "***********************************************************\n";
	std::cout << "*                                                         *\n";
	std::cout << "*   Example of parser the book_catalog.xml using C++11.   *\n";
	std::cout << "*                                                         *\n";
	std::cout << "***********************************************************\n\n";
	try {
		xml_dom test("book_catalog.xml");

		std::cout << "version: " << test.get_declaration("version") << "\n";
		std::cout << "encoding: " << test.get_declaration("encoding") << "\n";

		std::cout << "root: " << test.get_root().get_name() << "\n";

		for (auto& book : test.get_root().get_children()) {
			std::cout << "book id: " << book.get_attribute("id") << "\n";
			for(auto& info : book.get_children()) {
				std::cout << "  " << info.get_name() << ": " << info.get_text() << "\n";
			}
		}
	}
	catch (std::exception e) {
		std::cout << e.what() << "\n";
	}

	return 0;
}
