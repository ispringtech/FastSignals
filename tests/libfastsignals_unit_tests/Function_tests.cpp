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

TEST_CASE("Can copy function", "[function]")
{
	unsigned calledCount = 0;
	bool value = false;
	function<void(bool)> callback = [&](bool gotValue) {
		++calledCount;
		value = gotValue;
	};
	auto callback2 = callback;
	REQUIRE(calledCount == 0);
	CHECK(!value);
	callback(true);
	REQUIRE(calledCount == 1);
	CHECK(value);
	callback2(false);
	REQUIRE(calledCount == 2);
	CHECK(!value);
}

TEST_CASE("Can move function", "[function]")
{
	bool called = false;
	function<void()> callback = [&] {
		called = true;
	};
	auto callback2(std::move(callback));
	REQUIRE_THROWS(callback());
	REQUIRE(!called);
	callback2();
	REQUIRE(called);
}

TEST_CASE("Can release packed function", "[function]")
{
	function<int()> iota = [v = 0]() mutable {
		return v++;
	};
	REQUIRE(iota() == 0);

	auto packedFn = std::move(iota).release();
	REQUIRE_THROWS_AS(iota(), std::bad_function_call);

	auto&& proxy = packedFn.get<int()>();
	REQUIRE(proxy() == 1);
	REQUIRE(proxy() == 2);
}

TEST_CASE("Function copy has its own packed function", "[function]")
{
	function<int()> iota = [v = 0]() mutable {
		return v++;
	};

	REQUIRE(iota() == 0);

	auto iotaCopy(iota);

	REQUIRE(iota() == 1);
	REQUIRE(iota() == 2);

	REQUIRE(iotaCopy() == 1);
	REQUIRE(iotaCopy() == 2);
}

TEST_CASE("can work with callables that have vtable", "[function]")
{
	class Base
	{
	};

	class Interface : public Base
	{
	public:
		virtual ~Interface() = default;
		virtual void operator()() const = 0;
	};
	class Class : public Interface
	{
	public:
		Class(bool* destructorCalled)
			: m_destructorCalled(destructorCalled)
		{
		}

		~Class()
		{
			*m_destructorCalled = true;
		}

		void operator()() const override
		{
		}

		bool* m_destructorCalled = nullptr;
	};
	bool destructorCalled = false;
	{
		function<void()> f = Class(&destructorCalled);
		f();
		auto packed = f.release();
		destructorCalled = false;
	}
	CHECK(destructorCalled);
}

TEST_CASE("can work with callables with virtual inheritance", "[function]")
{
	struct A
	{
		void operator()() const
		{
			m_called = true;
		}

		~A()
		{
			*m_destructorCalled = true;
		}

		mutable bool m_called = false;
		bool* m_destructorCalled = nullptr;
	};
	struct B : public virtual A
	{
	};
	struct C : public virtual A
	{
	};
	struct D : virtual public B, virtual public C
	{
		D(bool* destructorCalled)
		{
			m_destructorCalled = destructorCalled;
		}

		using A::operator();
	};
	bool destructorCalled = false;
	{
		function<void()> f = D(&destructorCalled);
		f();
		auto packed = f.release();
		destructorCalled = false;
	}
	CHECK(destructorCalled);
}