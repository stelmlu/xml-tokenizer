#pragma once

#include "../xml-tokenizer.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <memory>

class xml_dom {
public:
	struct xml_deleter {
		void operator()(xml_t* xml) const {
			xml_close(xml);
		}
	};

	using attribute_t = std::map<std::string, std::string>;
	using xml_ptr_t = std::unique_ptr<xml_t, xml_deleter>;

	struct element_t {
		element_t() {}
		element_t(xml_t* xml, std::string name) : m_name(name) {
			xml_token_t tok = xml_next_token(xml);
			while (!(tok == XML_END_TAG && name == xml_get_name(xml))) {
				switch (tok) {
				case XML_ATTRIBUTE:
					m_attribute_lookup[xml_get_name(xml)] = xml_get_value(xml);
					break;
				case XML_START_TAG:
					m_children.push_back(element_t(xml, xml_get_name(xml)));
					break;
				case XML_TEXT:
					m_text = xml_get_text(xml);
					break;
				case XML_ERROR:
					throw std::runtime_error(xml_get_error(xml));
				}
				tok = xml_next_token(xml);
			}
		}

		using children_t = std::vector<element_t>;

		std::string get_name() {
			return m_name;
		}

		std::string get_text() {
			return m_text;
		}

		bool has_attribute(std::string name) {
			return m_attribute_lookup.find(name) != m_attribute_lookup.end();
		}

		std::string get_attribute(std::string name) {
			auto ret = m_attribute_lookup.find(name);
			if (ret == m_attribute_lookup.end()) throw std::runtime_error(std::string("Failed to get attribute: ") + name);
			return ret->second;
		}

		children_t get_children() {
			return m_children;
		}

		element_t get_first_child(std::string name) {
			for (auto element : m_children) {
				if (name == element.get_name()) {
					return element;
				}
			}
			throw std::runtime_error(std::string("Failed to find first element: ") + name);
		}


	private:
		std::string m_name;
		std::string m_text;
		attribute_t m_attribute_lookup;
		children_t m_children;
	};

private:
	xml_ptr_t m_xml_ptr;
	element_t m_root;
	attribute_t m_declaration_lookup;

public:
	xml_dom(const char* filename) : m_xml_ptr(xml_fopen(filename)) {
		if (m_xml_ptr == nullptr) throw std::runtime_error(std::string("Failed to open: ") + filename);

		xml_token_t tok = xml_next_token(m_xml_ptr.get());
		while (tok != XML_END_DOCUMENT) {
			switch (tok) {
			case XML_DECLARATION:
				m_declaration_lookup[xml_get_name(m_xml_ptr.get())] = xml_get_value(m_xml_ptr.get());
				break;
			case XML_START_TAG:
				m_root = element_t(m_xml_ptr.get(), xml_get_name(m_xml_ptr.get()));
				break;
			case XML_ERROR:
				throw std::runtime_error(xml_get_error(m_xml_ptr.get()));
				break;
			}
			tok = xml_next_token(m_xml_ptr.get());
		}
	}

	bool has_declaration(std::string name) {
		return m_declaration_lookup.find(name) != m_declaration_lookup.end();
	}

	std::string get_declaration(std::string name) {
		auto ret = m_declaration_lookup.find(name);
		if (ret == m_declaration_lookup.end()) throw std::runtime_error(std::string("Failed to get declaration: ") + name);
		return ret->second;
	}

	element_t get_root() {
		return m_root;
	}
};
