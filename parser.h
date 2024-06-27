#pragma once

#include <stack>
#include <string>
#include <sstream>

#include "json_errors.h"
#include "value.h"


namespace tdg::json
{
	class parser final
	{
		enum class state {
			WITHIN_OBJECT,
			WITHIN_ARRAY
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

			while (istr.get(current_char))
			{
				if (std::isspace(current_char) || current_char == ':')
				{
					continue;
				}
				else if (current_char == '{')
				{
					push_aggregate<object>();
				}
				else if (current_char == '}')
				{
					finalize();
				}
				else if (current_char == '[')
				{
					push_aggregate<array>();
				}
				else if (current_char == ']')
				{
					finalize();
				}
				else if (current_char == ',')
				{
					finalize();
				}
				else if (current_char == '"')
				{
					if (istr.unget())
					{
						process_string(istr);
					}
				}
				else if (current_char == 't')
				{
					process_true(istr);
				}
				else if (current_char == 'f')
				{
					process_false(istr);
				}
				else if (current_char == 'n')
				{
					process_null(istr);
				}
				else if (current_char == '-' || (std::isdigit(current_char) && current_char != '0'))
				{
					if (istr.unget())
					{
						process_number(istr);
					}
				}
			}

			auto value = std::move(m_stack.top());
			m_stack.pop();

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

		void finalize()
		{
			auto current_value = std::move(m_stack.top());

			m_stack.pop();

			if (!m_stack.empty())
			{
				if (m_stack.top().is_array())
				{
					m_stack.top().get<array>().emplace_back(std::move(current_value));
				}
				else if (m_stack.top().is_string())
				{
					auto parent_value = std::move(m_stack.top());

					m_stack.pop();

					auto& parent_object = m_stack.top();
					assert(parent_object.is_object());
					parent_object[std::move(parent_value.get<std::string>())] = std::move(current_value);
				}
			}
		}

		void process_string(std::istream& istr)
		{
			std::string parsed_string;

			istr >> std::quoted(parsed_string);

			m_stack.emplace(std::move(parsed_string));
		}

		void process_true(std::istream& istr)
		{
			char arr[4];

			istr.get(arr, 4);

			if (arr[0] == 'r' && arr[1] == 'u' && arr[2] == 'e')
			{
				m_stack.emplace(true);
			}
			else
			{
				throw invalid_json_exception("Expected 'true' constant");
			}
		}

		void process_false(std::istream& istr)
		{
			char arr[5];

			istr.get(arr, 5);

			if (arr[0] == 'a' && arr[1] == 'l' && arr[2] == 's' && arr[3] == 'e')
			{
				m_stack.emplace(false);
			}
			else
			{
				throw invalid_json_exception("Expected 'false' constant");
			}
		}

		void process_null(std::istream& istr)
		{
			char arr[4];

			istr.get(arr, 4);

			if (arr[0] == 'u' && arr[1] == 'l' && arr[2] == 'l')
			{
				m_stack.emplace(nullptr);
			}
			else
			{
				throw invalid_json_exception("Expected 'null' constant");
			}
		}

		void process_number(std::istream& istr)
		{
			static const std::string allowed_chars("-+0123456789.eE");
			char current_char;
			auto is_signed = false;
			auto is_float = false;

			std::string buffer;
			buffer.reserve(32);

			if (istr.get(current_char))
			{
				is_signed = (current_char == '-');
				buffer.push_back(current_char);

				while (istr.get(current_char))
				{
					if (allowed_chars.find(current_char) != std::string::npos)
					{
						buffer.push_back(current_char);
						
						if (current_char == '.')
						{
							is_float = true;
						}
					}
					else
					{
						istr.unget();
						break;
					}
				}
			}

			if (istr)
			{
				std::istringstream iss(buffer);

				if (is_float)
				{
					double d = 0;
					iss >> d;
					m_stack.emplace(d);
				}
				else if (is_signed)
				{
					int64_t si = 0;
					iss >> si;
					m_stack.emplace(si);
				}
				else
				{
					uint64_t ui;
					iss >> ui;
					m_stack.emplace(ui);
				}
			}
			else
			{
				throw invalid_json_exception("Invalid number: " + buffer);
			}
		}

		std::stack<value> m_stack;
	};
}
