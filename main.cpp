#include <stdio.h>

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
// TODO: Wrap stdlib calls in Macro (done)
// TODO: Add XML_START_DOCUMENT, XML_START_ATTRIBUTES and XML_END_ATTRIBUTES

#define XML_PARSER_IMPLEMENTATION
#include "xml-parser.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <optional>

struct book_t {
	std::string id;
	std::string author;
	std::string title;
	std::string genre;
	std::string price;
	std::string public_date;
	std::string description;
};

using catalog_t = std::vector<book_t>;

struct xml_parser {
	using attr_t = std::optional<std::pair<std::string, std::string>>;
	using text_t = std::string;

	xml_parser(const char* filename);
	~xml_parser() { xml_close(m_xml); }
	bool find_start_tag(const std::string& name);
	attr_t find_next_attr();
	text_t find_text();
	bool find_end_tag(const std::string& name);

private:
	xml_t* m_xml;
	std::string m_stored_start_tag;
};

xml_parser::xml_parser(const char* filename)
{
	m_xml = xml_fopen(filename);
	if (m_xml == nullptr) {
		throw std::runtime_error(std::string("Failed to open: ") + filename);
	}
}

bool xml_parser::find_start_tag(const std::string& name)
{
	xml_token_t tok = xml_next_token(m_xml);
	while (!(tok == XML_START_TAG && name == xml_get_name(m_xml))) {
		if (tok == XML_END_DOCUMENT) return false;
		if (tok == XML_ERROR) throw std::runtime_error(xml_get_error(m_xml));
		tok = xml_next_token(m_xml);
	}
	return true;
}

bool xml_parser::find_end_tag(const std::string& name)
{
	if (!m_stored_start_tag.empty()) {
		bool ret_value = m_stored_start_tag == name;
		m_stored_start_tag.clear();
		return ret_value;
	}
	else {
		xml_token_t tok = xml_next_token(m_xml);
		while (!(tok == XML_START_TAG && name == xml_get_name(m_xml))) {
			if (tok == XML_END_DOCUMENT) return false;
			if (tok == XML_ERROR) throw std::runtime_error(xml_get_error(m_xml));
			tok = xml_next_token(m_xml);
		}
	}
	return true;
}

xml_parser::attr_t xml_parser::find_next_attr()
{
	xml_token_t tok = xml_next_token(m_xml);
	if (tok == XML_ATTRIBUTE) {
		return attr_t{ {
				std::string(xml_get_name(m_xml)),
				std::string(xml_get_value(m_xml))
		} };
	}
	else if (tok == XML_ERROR) {
		throw std::runtime_error(xml_get_error(m_xml));
	}
	else if (tok == XML_START_TAG) {
		m_stored_start_tag = xml_get_name(m_xml);
		return {};
	}
	return {};
}

xml_parser::text_t xml_parser::find_text()
{
	xml_token_t tok = xml_next_token(m_xml);
	while (tok != XML_TEXT) {
		if (tok == XML_END_DOCUMENT) return "";
		tok = xml_next_token(m_xml);
	}
	return xml_get_text(m_xml);
}

int main()
{
	xml_parser test = xml_parser("test.xml");

	test.find_start_tag("catalog");

	catalog_t catalog;

	for (;;) {
		book_t book;
		xml_parser::attr_t attr;

		if(!test.find_start_tag("book")) break;

		attr = test.find_next_attr();
		while (attr != std::nullopt) {
			if (attr.value().first == "id") {
				book.id = attr.value().second;
			}
			attr = test.find_next_attr();
		}
		
		book.author = test.find_text();

		test.find_start_tag("title");
		book.title = test.find_text();

		test.find_start_tag("genre");
		book.genre = test.find_text();

		test.find_start_tag("price");
		book.price = test.find_text();

		test.find_start_tag("publish_date");
		book.public_date = test.find_text();

		test.find_start_tag("description");
		book.description = test.find_text();

		test.find_end_tag("book");

		catalog.push_back(book);
	}

	for (auto book : catalog) {
		std::cout << "book id: " << book.id << "\n";
		std::cout << "  author: " << book.author << "\n";
		std::cout << "  title: " << book.title << "\n";
		std::cout << "  genre: " << book.genre << "\n";
		std::cout << "  price: " << book.price << "\n";
		std::cout << "  publish-date: " << book.public_date << "\n";
		std::cout << "  description: " << book.description << "\n";
	}

	return 0;
}

std::string read_tag(xml_t* xml, const char* name)
{
	std::string tag_name = xml_get_name(xml);
	if (tag_name != name) throw std::runtime_error(std::string("Failed to find tag: ") + name);

	xml_token_t tok = xml_next_token(xml);
	while (!(tok == XML_TEXT || tok == XML_END_TAG)) {
		tok = xml_next_token(xml);
	}

	if (tok == XML_TEXT) {
		std::string text = xml_get_text(xml);
		tok = xml_next_token(xml);
		return text;
	}
	return "";
}
