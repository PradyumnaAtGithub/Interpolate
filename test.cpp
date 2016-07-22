/*
* Author: Professor Kenneth Chiu
* Binghamton University
*/

#include "Interpolate.hpp"
#include <iostream>
#include <typeinfo>
#include <locale>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cassert>
#include <ctime>
#include <cstring>
// Needed by {set,get}rlimit().
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

constexpr unsigned MEMORY_LIMIT = 1024*1024*30;

class A {
	friend int main(int argc, char **argv);
	friend std::ostream & operator<<(std::ostream &os, const A &a) {
		return os << a.i;
	}
	A(int i_) : i(i_) {}
	//A(const A &) = delete;
	//A &operator=(const A &) = delete;
	const int i;
};

class B {
	friend int main(int argc, char **argv);
	friend std::ostream & operator<<(std::ostream &os, const B &a) {
		return os << a.str;
	}
	B(std::string s) : str(std::move(s)) {}
	//B(const B &) = delete;
	//B &operator=(const B &) = delete;
	const std::string str;
};


// This class is used to test space efficiency.
class Out {
	public:
		Out(std::size_t mem_amount) : mem_amount(mem_amount) {}
		const std::size_t mem_amount;
};
std::ostream &
operator<<(std::ostream &os, const Out &o) {
	const std::string buf(1024*1024, 'A');
	for (std::size_t i = 0; i < o.mem_amount; i++) {
		os << buf;
	}
	return os;
}

template <typename... Ts>
void test(const char *func, int line_no, const std::string &cmp, const std::string &fmt, Ts &&...params) {
	std::stringstream s;
	s << cs540::Interpolate(fmt.c_str(), std::forward<Ts>(params)...);
	if (s.str() != cmp) {
		std::cerr << "    Comparison failed at " << func << ":" << line_no << ":\n";
		std::cerr << "    Correct result: \"" << cmp << "\"\n";
		std::cerr << "    Actual result: \"" << s.str() << "\"\n";
	}
}
#define CS540_TEST(...) test(__FUNCTION__, __LINE__, __VA_ARGS__)

int
main(int argc, char **argv) {

	//Test case 1: Example test case in description.
	int i = 1234;
	double x = 3.14;
	std::string str("foo");
	std::cout << cs540::Interpolate(R"(i=%, x1=%, x2=%\%, str1=%, str2=%)", i, x, 1.001, str, "hello");

	int rv;
	bool do_mem_test = false;

	{
		int c;
		while((c = getopt(argc, argv, "m")) != -1) {
			switch(c) {
				case 'm' :
					do_mem_test = true;
					break;
				case '?' :
					std::cout << "Unrecognized option " << isprint(optopt) << ". The valid options are:\n"
						"\t-m\t:\tEnables large memory test.\n";
					return 1;
					break;
				default :
					abort();
					break;
			}
		}
	}

	if (do_mem_test) {

		// Limit the amount of memory that can be used, so as not to kill the
		// machine when running the space efficiency test.
		struct rlimit limit;
		rv = getrlimit(RLIMIT_AS, &limit); assert(rv == 0);
		limit.rlim_cur = MEMORY_LIMIT;
		rv = setrlimit(RLIMIT_AS, &limit); assert(rv == 0);
	}

	using namespace cs540;

	CS540_TEST("", "");

	CS540_TEST(R"(\)", R"(\)");
	CS540_TEST(R"(%)", R"(\%)");
	CS540_TEST(R"(\\)", R"(\\)");
	CS540_TEST(R"(foo)", R"(foo)");
	CS540_TEST("\n", "\n");
	CS540_TEST(R"(\%)", R"(\\%)");

	// Test if it returns ostream.
	{
		std::stringstream s;
		s << Interpolate("i=%", 1234) << ", foo" << std::endl;
		assert(s.str() == "i=1234, foo\n");
	}
	CS540_TEST("1234", "%", 1234);
	CS540_TEST(" 1234", " %", 1234);
	CS540_TEST("1234 ", "% ", 1234);
	CS540_TEST(" 1234 ", " % ", 1234);

	{
		int i1 = 12376;
		const int i2 = 778;
		CS540_TEST("i=8798, j=12376, k=778", "i=%, j=%, k=%", 8798, i1, i2);
	}

	CS540_TEST(
			"56789 3.14 1234 Z hello 313 goodbye -31 1.99 0xffff7777 "
			"56789 3.14 1234 Z hello 313 goodbye -31 1.99 0xffff7777 "
			"56789 3.14 1234 Z hello 313 goodbye -31 1.99 0xffff7777 "
			"56789 3.14 1234 Z hello 313 goodbye -31 1.99 0xffff7777 "
			"56789 3.14 1234 Z hello 313 goodbye -31 1.99 0xffff7777",
			"% % % % % % % % % % "
			"% % % % % % % % % % "
			"% % % % % % % % % % "
			"% % % % % % % % % % "
			"% % % % % % % % % %",
			56789, 3.14, short(1234), 'Z', "hello", A(313), B("goodbye"), -31, 1.99F, (void *)0xffff7777,
			56789, 3.14, short(1234), 'Z', "hello", A(313), B("goodbye"), -31, 1.99F, (void *)0xffff7777,
			56789, 3.14, short(1234), 'Z', "hello", A(313), B("goodbye"), -31, 1.99F, (void *)0xffff7777,
			56789, 3.14, short(1234), 'Z', "hello", A(313), B("goodbye"), -31, 1.99F, (void *)0xffff7777,
			56789, 3.14, short(1234), 'Z', "hello", A(313), B("goodbye"), -31, 1.99F, (void *)0xffff7777);

	try {
		std::stringstream s;
		s << Interpolate("i=%, j=%", 1, 2, 3);
		assert(false);
	} catch (cs540::WrongNumberOfArgs) {
		// std::cout << "Caught exception due to too many args." << std::endl;
	}

	// Test too few.
	try {
		std::stringstream s;
		s << Interpolate("i=%, j=%, k=%", 1, 2);
		assert(false);
	} catch (cs540::WrongNumberOfArgs) {
		// std::cout << "Caught exception due to few args." << std::endl;
	}

	CS540_TEST("abc, 1234", "%, %",
			std::resetiosflags(std::ios_base::basefield),
			std::setiosflags(std::ios_base::hex),
			0xabc,
			std::resetiosflags(std::ios_base::basefield),
			1234);

	CS540_TEST("aa, 123, 999", "%, %, %",
			std::setbase(16), 0xaa,
			std::setbase(8), 0123,
			std::setbase(10), 999);

	CS540_TEST("--1.234567899", "%", std::setw(13), std::setprecision(10), std::setfill('-'), 1.234567899);	
	{
		std::fstream out("/dev/null");
		if (do_mem_test)
			out << cs540::Interpolate("%", Out{4*512*1024});
		else
			out << cs540::Interpolate("%", Out{1024});
	}

}

