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

	tdg::json::printer<std::fixed, 9> p(std::cout);

	value v{ "abc", 2.0, 3, true, nullptr, value{"abc", true, "xyz", value{1, 2, false}} };
	v[2] = 5;
	p.print(v);
	std::cout << std::endl;

	value v2{"abc", true};
	p.print(v2);
	std::cout << std::endl;

	value v6 = std::move(v2);
	v6["abc"] = false;
	v6["def"] = 8;
	p.print(v6);
	std::cout << std::endl;

	/*
	char char_arr[] = { 'a', 'b', 'c', 'd' };
	value v3{ char_arr, nullptr };
	value v4{ value{"abc", array{2}}, value{"xyz", -2} };

	std::string s = "asdfadf";
	value v5(s);


	p.print(v);
	std::cout << std::endl;
	p.print(v2);
	std::cout << std::endl;
	p.print(v3);
	std::cout << std::endl;
	p.print(v4);
	std::cout << std::endl;
	p.print(v5);
	std::cout << std::endl;
	*/
	
	return 0;
}
