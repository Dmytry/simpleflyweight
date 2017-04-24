#include "simple_flyweight.hpp"
#include <iostream>
#include <assert.h>

int main(int argc, char **argv) {
	SimpleFlyweight<std::string> a;
	a.New(std::string("hello"));// note: if you use a char literal, the parameter type is const char* and the resource will only be shared if that pointer is the same
	SimpleFlyweight<std::string> b;
	b.New(std::string("hello"));


	SimpleFlyweight<std::string> c;
	c.New(std::string(" world"));
	assert(a.GetPtr() == b.GetPtr());
	assert(a.GetPtr() != c.GetPtr());
	SimpleFlyweight<std::string, 1> d;

	d.New(std::string(" world"));
	assert(c.GetPtr() != d.GetPtr());

	//works thanks to use of std::make_tuple , which treats const & the same as a value
	SimpleFlyweight<std::string> e;
	const std::string &const_ref_test = a();
	e.New(const_ref_test);

	assert(a.GetPtr() == e.GetPtr());

	std::cout << b() << c() << std::endl;

	SimpleFlyweight<std::string> f(std::string("hello"));
	assert(a.GetPtr() == f.GetPtr());
	SimpleFlyweight<std::string> g(f);
	assert(a.GetPtr() == g.GetPtr());
	std::cout << "All passed" << std::endl;
}