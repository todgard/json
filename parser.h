#pragma once

#include <iomanip>
#include <sstream>
#include <stack>
#include <string>

#include "json_errors.h"
#include "value.h"


namespace tdg::json
{
	class parser final
	{
		using parsing_func_t = void (parser::*)(std::istream&, char);

		enum class FINALIZE_TYPE {
			OBJECT,
			ARRAY,
			ITEM
		};

		enum : char {
			OBJECT_START_BRACE			= '{',
			OBJECT_END_BRACE			= '}',
			ARRAY_START_BRACE			= '[',
			ARRAY_END_BRACE				= ']',
			VALUE_SEPARATOR				= ',',
			KEY_VALUE_SEPARATOR			= ':',
			QUOTE						= '"',
			COMMA						= ','
		};

	public:
		value parse(const std::string& input)
		{
			auto iss = std::istringstream(input);

			return parse(iss);
		}

		value parse(std::istream& istr)
		{
			char current_char;

			while (m_continuation_func && istr.get(current_char))
			{
				if (std::isspace(current_char))
				{
					continue;
				}

				(this->*m_continuation_func)(istr, current_char);
			}

			if (m_stack.size() != 1u)
			{
				m_stack.empty()
					? throw invalid_json_exception("Nothing to parse")
					: throw invalid_json_exception("Unclosed array/object after parsing available data");
			}

			auto value = std::move(m_stack.top());

			return value;
		}

	private:

		template <typename T>
		void push_aggregate() requires std::is_same_v<object, T> || std::is_same_v<array, T>
		{
			if (!m_stack.empty())
			{
				const auto& parent_value = m_stack.top();

				if (!parent_value.is_array() && !parent_value.is_string())
				{
					throw invalid_json_exception("Array/object can only be included in another array or object");
				}
			}

			m_stack.emplace(T());
		}

		void finalize(FINALIZE_TYPE type)
		{
			/*
			* When finalizing non-empty array or array item, the expected stack layout is:		root_value -> ... -> array -> current_value
			* After finalize current_value should be contained in array and stack should be:	root_value -> ... -> array
			* For object, the expected layout is:												root_value -> ... -> object -> key_value -> current_value
			* After finalize object should contain {key_value, current_value} pair and stack:	root_value -> ... -> object
			*/

			auto current_value = std::move(m_stack.top());

			m_stack.pop();

			//TODO: fix error handling for situation when the stack becomes empty here (basically extra ] or } characters)
			// maybe three separate methods for item, array and object to simplify logic
			assert(!m_stack.empty());

			if (m_stack.top().is_array())
			{
				if (type == FINALIZE_TYPE::OBJECT)
				{
					throw invalid_json_exception("Found unexpected object-closing character '}'");
				}

				m_stack.top().get<array>().emplace_back(std::move(current_value));
			}
			else if (m_stack.top().is_string())
			{
				if (type == FINALIZE_TYPE::ARRAY)
				{
					throw invalid_json_exception("Found unexpected array-closing character ']'");
				}

				auto key_value = std::move(m_stack.top());

				m_stack.pop();

				auto& parent_object = m_stack.top();

				if (!m_stack.top().is_object())
				{
					throw invalid_json_exception("Unable to finalize object");
				}

				parent_object[std::move(key_value.get<std::string>())] = std::move(current_value);
			}
		}

		void parse_scalar(std::istream& istr, const char current_char)
		{
			if (current_char == '"')
			{
				push_string(istr);
			}
			else if (current_char == 't')
			{
				push_true(istr);
			}
			else if (current_char == 'f')
			{
				push_false(istr);
			}
			else if (current_char == 'n')
			{
				push_null(istr);
			}
			else if (current_char == '-' || std::isdigit(current_char))
			{
				push_number(istr, current_char);
			}
			else
			{
				throw invalid_json_exception(tdg::eh::make_error_msg("Unexpected start character for JSON scalar; stream pos: ", istr.tellg()));
			}
		}

		void push_string(std::istream& istr)
		{
			char current_char{};
			std::string parsed_string;
			auto is_escaped = false;
			auto missing_closing_quote = true;

			while (istr.get(current_char))
			{
				if (current_char == '\\')
				{
					is_escaped = !is_escaped;
				}
				else if (current_char == '"' && !is_escaped)
				{
					missing_closing_quote = false;
					break;
				}
				else if (std::iscntrl(current_char))
				{
					throw invalid_json_exception(
						tdg::eh::make_error_msg("Control characters are not allowed in JSON string, possibly missing closing quote; stream pos: ", istr.tellg()));
				}
				else
				{
					is_escaped = false;
				}

				parsed_string.push_back(current_char);
			}

			//std::cout << "Last string: " << parsed_string << std::endl;

			if (missing_closing_quote)
			{
				throw invalid_json_exception(tdg::eh::make_error_msg("Closing quote not found for JSON string", istr.tellg()));
			}

			m_stack.emplace(std::move(parsed_string));
		}

		template <std::size_t CNUM>
		void extract_characters(std::istream& istr, char(&arr)[CNUM])
		{
			if (!istr.get(arr, CNUM))
			{
				throw invalid_json_exception("Unexpected EOF or read failure while trying extract characters from stream");
			}
		}

		void push_true(std::istream& istr)
		{
			char arr[4] = {};

			extract_characters(istr, arr);

			if (arr[0] == 'r' && arr[1] == 'u' && arr[2] == 'e')
			{
				m_stack.emplace(true);
			}
			else
			{
				throw invalid_json_exception(tdg::eh::make_error_msg("Expected 'true' literal", istr.tellg()));
			}
		}

		void push_false(std::istream& istr)
		{
			char arr[5] = {};

			extract_characters(istr, arr);

			if (arr[0] == 'a' && arr[1] == 'l' && arr[2] == 's' && arr[3] == 'e')
			{
				m_stack.emplace(false);
			}
			else
			{
				throw invalid_json_exception(tdg::eh::make_error_msg("Expected 'false' literal", istr.tellg()));
			}
		}

		void push_null(std::istream& istr)
		{
			char arr[4] = {};

			extract_characters(istr, arr);

			if (arr[0] == 'u' && arr[1] == 'l' && arr[2] == 'l')
			{
				m_stack.emplace(nullptr);
			}
			else
			{
				throw invalid_json_exception(tdg::eh::make_error_msg("Expected 'null' literal", istr.tellg()));
			}
		}

		void push_number(std::istream& istr, char first_char)
		{
			//TODO: add handling of digit sequences starting with 0 and -0 cases
			// and fix error handling

			char current_char{};
			auto is_float = false;
			auto is_unsigned_int = (first_char != '-');

			std::string buffer;
			buffer.reserve(16);
			buffer.push_back(first_char);

			while (istr.get(current_char))
			{
				if ((current_char >= '0' && current_char <= '9')
					|| current_char == '+'
					|| current_char == '-'
					|| current_char == 'e'
					|| current_char == 'E'
					|| current_char == '.')
				{
					buffer.push_back(current_char);
					
					if (current_char == '.')
					{
						is_float = true;
						is_unsigned_int = false;
					}
				}
				else
				{
					istr.unget();
					break;
				}
			}

			if (istr)
			{
				std::size_t pos = 0;

				if (is_unsigned_int)
				{
					m_stack.emplace(std::stoull(buffer, &pos));
				}
				else if (is_float)
				{
					m_stack.emplace(std::stod(buffer, &pos));
				}
				else
				{
					m_stack.emplace(std::stoll(buffer, &pos));
				}

				if (pos != buffer.size())
				{
					throw invalid_json_exception(
						tdg::eh::make_error_msg("Invalid numerical value: ", buffer, "; stream pos: ", istr.tellg()));
				}
			}
			else
			{
				throw invalid_json_exception(
					tdg::eh::make_error_msg("Unexpected EOF or stream failure while reading a numerical value: ", buffer, "; stream pos: ", istr.tellg()));
			}
		}

		void parse_value_start(std::istream& istr, char current_char)
		{
			if (current_char == OBJECT_START_BRACE)
			{
				push_aggregate<object>();
				m_is_empty_aggregate = true;
				m_continuation_func = &parser::parse_object_key_or_end;
			}
			else if (current_char == ARRAY_START_BRACE)
			{
				push_aggregate<array>();
				m_is_empty_aggregate = true;
				m_continuation_func = &parser::parse_array_item_or_end;
			}
			else
			{
				parse_scalar(istr, current_char);
				m_continuation_func = &parser::parse_value_end;
			}
		}

		void parse_object_key_or_end(std::istream& istr, char current_char)
		{
			if (current_char == QUOTE)
			{
				parse_scalar(istr, current_char);

				m_is_empty_aggregate = false;
				m_continuation_func = &parser::parse_value_end;
			}
			else if (current_char == OBJECT_END_BRACE)
			{
				parse_value_end(istr, current_char);
			}
			else
			{
				throw invalid_json_exception(
					tdg::eh::make_error_msg("Invalid character '", current_char, "' after JSON object starting brace; stream pos: ", istr.tellg()));
			}
		}

		void parse_array_item_or_end(std::istream& istr, char current_char)
		{
			if (current_char == ARRAY_END_BRACE)
			{
				parse_value_end(istr, current_char);
			}
			else
			{
				m_is_empty_aggregate = false;

				parser::parse_value_start(istr, current_char);
			}
		}

		void parse_value_end(std::istream& istr, char current_char)
		{
			if (current_char == COMMA)
			{
				finalize(FINALIZE_TYPE::ITEM);

				m_continuation_func = &parser::parse_value_start;
			}
			else if (current_char == KEY_VALUE_SEPARATOR)
			{
				m_continuation_func = &parser::parse_value_start;
			}
			else if (current_char == OBJECT_END_BRACE || current_char == ARRAY_END_BRACE)
			{
				if (!m_is_empty_aggregate)
				{
					finalize(current_char == OBJECT_END_BRACE ? FINALIZE_TYPE::OBJECT : FINALIZE_TYPE::ARRAY);
				}

                m_is_empty_aggregate = false;
				m_continuation_func = &parser::parse_value_end;
			}
			else
			{
				throw invalid_json_exception(
					tdg::eh::make_error_msg("Unexpected character '", current_char, "' after a JSON value, expected one of \", ] }\"; stream pos: ", istr.tellg()));
			}
		}

		// member data

		std::stack<value>		m_stack;
		bool					m_is_empty_aggregate = false;
		parsing_func_t			m_continuation_func = &parser::parse_value_start;
	};
}
