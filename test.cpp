#include <iostream>

template<typename T>
void echo(const T& str) {
	std::cout << str;
}

class A {
public:
	A() {}
	A(const A&) { echo(1); }
	A(const A&&) { echo(2); }
	A& operator=(const A&) { echo(3); }
	A& operator=(const A&&) { echo(4); }
};

int main(int, char**) {
	A a, b;
	a = b;
	a = A();
	A c(a);
	A d(std::move(a));
	A e = a;
	A f = std::move(a);
	std::cout << std::endl;
}