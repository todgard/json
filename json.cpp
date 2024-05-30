// json.cpp : Defines the entry point for the application.
//

#include "Value.h"

using namespace std;

int main()
{
	tdg::json::value v{ "abc", 2.0, 3, true };
	tdg::json::value v2{ tdg::json::array{"abc", true}, {"xzy", 2} };

	char char_arr[] = { 'a', 'b', 'c', 'd' };
	tdg::json::value v3{char_arr, nullptr};

	return 0;
}
