#include "catch2/catch.hpp"
#include "libfastsignals/include/function.h"

using namespace is::signals;

namespace
{
int Abs(int x)
{
	return x >= 0 ? x : -x;
}

int Sum(int a, int b)
{
	return a + b;
}

void InplaceAbs(int& x)
{
	x = Abs(x);
}

std::string GetStringHello()
{
	return "hello";
}

class AbsFunctor
{
public:
	int operator()(int x) const
	{
		return Abs(x);
	}
};

class SumFunctor
{
public:
	int operator()(int a, int b) const
	{
		return Sum(a, b);
	}
};

class InplaceAbsFunctor
{
public:
	void operator()(int& x) /* non-const */
	{
		if (m_calledOnce)
		{
			abort();
		}
		m_calledOnce = true;
		InplaceAbs(x);
	}

private:
	bool m_calledOnce = false;
};

class GetStringFunctor
{
public:
	explicit GetStringFunctor(const std::string& value)
		: m_value(value)
	{
	}

	std::string operator()() /* non-const */
	{
		if (m_calledOnce)
		{
			abort();
		}
		m_calledOnce = true;
		return m_value;
	}

private:
	bool m_calledOnce = false;
	std::string m_value;
};
} // namespace

TEST_CASE("Can use free function with 1 argument", "[function]")
{
	function<int(int)> fn = Abs;
	REQUIRE(fn(10) == 10);
	REQUIRE(fn(-10) == 10);
	REQUIRE(fn(0) == 0);
}

TEST_CASE("Can use free function with 2 arguments", "[function]")
{
	function<int(int, int)> fn = Sum;
	REQUIRE(fn(10, 5) == 15);
	REQUIRE(fn(-10, 0) == -10);
}

TEST_CASE("Can use free function without arguments", "[function]")
{
	function<std::string()> fn = GetStringHello;
	REQUIRE(fn() == "hello");
}

TEST_CASE("Can use free function without return value", "[function]")
{
	function<void(int&)> fn = InplaceAbs;
	int a = -10;
	fn(a);
	REQUIRE(a == 10);
}

TEST_CASE("Can use lambda with 1 argument", "[function]")
{
	function<int(int)> fn = [](int value) {
		return Abs(value);
	};
	REQUIRE(fn(10) == 10);
	REQUIRE(fn(-10) == 10);
	REQUIRE(fn(0) == 0);
}

TEST_CASE("Can use lambda with 2 arguments", "[function]")
{
	function<int(int, int)> fn = [](auto&& a, auto&& b) {
		return Sum(a, b);
	};
	REQUIRE(fn(10, 5) == 15);
	REQUIRE(fn(-10, 0) == -10);
}

TEST_CASE("Can use lambda without arguments", "[function]")
{
	function<std::string()> fn = [] {
		return GetStringHello();
	};
	REQUIRE(fn() == "hello");
}

TEST_CASE("Can use lambda without return value", "[function]")
{
	bool calledOnce = false;
	function<void(int&)> fn = [calledOnce](auto& value) mutable {
		if (calledOnce)
		{
			abort();
		}
		calledOnce = true;
		InplaceAbs(value);
	};
	int a = -10;
	fn(a);
	REQUIRE(a == 10);
}

TEST_CASE("Can use functor with 1 argument", "[function]")
{
	function<int(int)> fn = AbsFunctor();
	REQUIRE(fn(10) == 10);
	REQUIRE(fn(-10) == 10);
	REQUIRE(fn(0) == 0);
}

TEST_CASE("Can use functor with 2 arguments", "[function]")
{
	function<int(int, int)> fn = SumFunctor();
	REQUIRE(fn(10, 5) == 15);
	REQUIRE(fn(-10, 0) == -10);
}

TEST_CASE("Can use functor without arguments", "[function]")
{
	function<std::string()> fn = GetStringFunctor("hello");
	REQUIRE(fn() == "hello");
}

TEST_CASE("Can use functor without return value", "[function]")
{
	function<void(int&)> fn = InplaceAbsFunctor();
	int a = -10;
	fn(a);
	REQUIRE(a == 10);
}

TEST_CASE("Can construct function with cons std::function<>&", "[function]")
{
	using BoolCallback = std::function<void(bool succeed)>;
	bool value = false;
	const BoolCallback& cb = [&value](bool succeed) {
		value = succeed;
	};

	function<void(bool)> fn = cb;
	fn(true);
	REQUIRE(value == true);
	fn(false);
	REQUIRE(value == false);
	fn(true);
	REQUIRE(value == true);
}
