#include "reflection.h" 
#include "person.h"
enum PersonNames {
	steve = 69,
	peter = 48
};


#include <iostream>

int main()
{
	constexpr person p2{3, "asa"};

	constexpr auto num = 1;
	constexpr auto res = reflect<person>::value<num>(p2);
	
	std::cout << reflect<person>::value<0>(p2) << " : ";
	std::cout << res << '\n';
	
	//std::cout << reflect<PersonNames>::name[0] << " : ";
	//std::cout << reflect<PersonNames>::value[0] << '\n';

	std::cin.get();
}