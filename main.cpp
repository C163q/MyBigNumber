#include<iostream>
#include"integer_container.h"
#include"integer.h"

int main() {
	C163q::integer a("13525435464521371294749172397");
	C163q::integer b("-13525435464521376294749972397");
	::std::cout << (b * b).ToString() << ::std::endl;
}