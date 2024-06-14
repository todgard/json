// json.cpp : Defines the entry point for the application.
//

#include "printer.h"
#include "value.h"

using namespace std;

int main()
{
	using value = tdg::json::value;
	using array = tdg::json::array;
	using object = tdg::json::object;

	value v{ "abc", 2.0, 3, true };
	COUT("After v creation");
	value v2{ array{"abc", true}, {"xzy", 2} };

	char char_arr[] = { 'a', 'b', 'c', 'd' };
	value v3{ char_arr, nullptr };
	value v4{ {"abc", array{2}}, {"xyz", -2} };

	tdg::json::printer<std::fixed, 9> p(std::cout);

	p.print(v);
	std::cout << std::endl;
	p.print(v2);
	std::cout << std::endl;
	p.print(v3);
	std::cout << std::endl;
	p.print(v4);
	std::cout << std::endl;
	
	return 0;
}
