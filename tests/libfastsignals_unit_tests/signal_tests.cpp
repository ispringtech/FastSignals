#include "stdafx.h"

#include "catch2/catch.hpp"
#include "libfastsignals/Signal.h"
#include <string>

using namespace is::signals;

TEST_CASE("Can connect a few slots and emit", "[signal]")
{
	signal<void(int)> valueChanged;

	int value1 = 0;
	int value2 = 0;
	valueChanged.connect([&value1](int value) {
		value1 = value;
	});
	valueChanged.connect([&value2](int value) {
		value2 = value;
	});
	REQUIRE(value1 == 0);
	REQUIRE(value2 == 0);

	valueChanged(10);
	REQUIRE(value1 == 10);
	REQUIRE(value2 == 10);
}

TEST_CASE("Can safely pass rvalues", "[signal]")
{
	const std::string expected = "If the type T is a reference type, provides the member typedef type which is the type referred to by T. Otherwise type is T.";
	;
	std::string passedValue = expected;
	signal<void(std::string)> valueChanged;

	std::string value1;
	std::string value2;
	valueChanged.connect([&value1](std::string value) {
		value1 = value;
	});
	valueChanged.connect([&value2](std::string value) {
		value2 = value;
	});

	valueChanged(std::move(passedValue));
	REQUIRE(value1 == expected);
	REQUIRE(value2 == expected);
}
// TODO: remove pragmas when code generation bug will be fixed.
#if defined(_MSC_VER)
#pragma optimize("", off)
#endif
TEST_CASE("Can pass mutable ref", "[signal]")
{
	const std::string expected = "If the type T is a reference type, provides the member typedef type which is the type referred to by T. Otherwise type is T.";
	;
	signal<void(std::string&)> valueChanged;

	std::string passedValue;
	valueChanged.connect([expected](std::string& value) {
		value = expected;
	});
	valueChanged(passedValue);

	REQUIRE(passedValue == expected);
}
#if defined(_MSC_VER)
#pragma optimize("", on)
#endif

TEST_CASE("Can disconnect slot with explicit call", "[signal]")
{
	signal<void(int)> valueChanged;

	int value1 = 0;
	int value2 = 0;
	int value3 = 0;
	auto conn1 = valueChanged.connect([&value1](int value) {
		value1 = value;
	});
	auto conn2 = valueChanged.connect([&value2](int value) {
		value2 = value;
	});
	valueChanged.connect([&value3](int value) {
		value3 = value;
	});
	REQUIRE(value1 == 0);
	REQUIRE(value2 == 0);
	REQUIRE(value3 == 0);

	valueChanged(10);
	REQUIRE(value1 == 10);
	REQUIRE(value2 == 10);
	REQUIRE(value3 == 10);

	conn2.disconnect();
	valueChanged(-99);
	REQUIRE(value1 == -99);
	REQUIRE(value2 == 10);
	REQUIRE(value3 == -99);

	conn1.disconnect();
	valueChanged(17);
	REQUIRE(value1 == -99);
	REQUIRE(value2 == 10);
	REQUIRE(value3 == 17);
}

TEST_CASE("Can disconnect slot with scoped_connection", "[signal]")
{
	signal<void(int)> valueChanged;

	int value1 = 0;
	int value2 = 0;
	int value3 = 0;
	{
		scoped_connection conn1 = valueChanged.connect([&value1](int value) {
			value1 = value;
		});
		{
			scoped_connection conn2 = valueChanged.connect([&value2](int value) {
				value2 = value;
			});
			valueChanged.connect([&value3](int value) {
				value3 = value;
			});
			REQUIRE(value1 == 0);
			REQUIRE(value2 == 0);
			REQUIRE(value3 == 0);

			valueChanged(10);
			REQUIRE(value1 == 10);
			REQUIRE(value2 == 10);
			REQUIRE(value3 == 10);
		}

		// conn2 disconnected.
		valueChanged(-99);
		REQUIRE(value1 == -99);
		REQUIRE(value2 == 10);
		REQUIRE(value3 == -99);
	}

	// conn1 disconnected.
	valueChanged(17);
	REQUIRE(value1 == -99);
	REQUIRE(value2 == 10);
	REQUIRE(value3 == 17);
}

TEST_CASE("Can disconnect all", "[signal]")
{
	signal<void(int)> valueChanged;

	int value1 = 0;
	int value2 = 0;
	int value3 = 0;
	valueChanged.connect([&value1](int value) {
		value1 = value;
	});
	valueChanged.connect([&value2](int value) {
		value2 = value;
	});
	valueChanged.connect([&value3](int value) {
		value3 = value;
	});
	REQUIRE(value1 == 0);
	REQUIRE(value2 == 0);
	REQUIRE(value3 == 0);

	valueChanged(63);
	REQUIRE(value1 == 63);
	REQUIRE(value2 == 63);
	REQUIRE(value3 == 63);

	valueChanged.disconnect_all_slots();
	valueChanged(101);
	REQUIRE(value1 == 63);
	REQUIRE(value2 == 63);
	REQUIRE(value3 == 63);
}

TEST_CASE("Can disconnect inside slot", "[signal]")
{
	signal<void(int)> valueChanged;

	int value1 = 0;
	int value2 = 0;
	int value3 = 0;
	connection conn2;
	valueChanged.connect([&value1](int value) {
		value1 = value;
	});
	conn2 = valueChanged.connect([&](int value) {
		value2 = value;
		conn2.disconnect();
	});
	valueChanged.connect([&value3](int value) {
		value3 = value;
	});
	REQUIRE(value1 == 0);
	REQUIRE(value2 == 0);
	REQUIRE(value3 == 0);

	valueChanged(63);
	REQUIRE(value1 == 63);
	REQUIRE(value2 == 63);
	REQUIRE(value3 == 63);

	valueChanged(101);
	REQUIRE(value1 == 101);
	REQUIRE(value2 == 63); // disconnected in slot.
	REQUIRE(value3 == 101);
}

TEST_CASE("Disconnects OK if signal dead first", "[signal]")
{
	connection conn2;
	{
		scoped_connection conn1;
		{
			signal<void(int)> valueChanged;
			conn2 = valueChanged.connect([](int) {
			});
			// Just unused.
			valueChanged.connect([](int) {
			});
			conn1 = valueChanged.connect([](int) {
			});
		}
		conn2.disconnect();
	}
	conn2.disconnect();
}

TEST_CASE("Returns last called slot result with default combiner", "[signal]")
{
	connection conn2;
	{
		scoped_connection conn1;
		{
			signal<int(int)> absSignal;
			conn2 = absSignal.connect([](int value) {
				return value * value;
			});
			conn1 = absSignal.connect([](int value) {
				return abs(value);
			});

			REQUIRE(absSignal(45) == 45);
			REQUIRE(absSignal(-1) == 1);
			REQUIRE(absSignal(-177) == 177);
			REQUIRE(absSignal(0) == 0);
		}
		conn2.disconnect();
	}
	conn2.disconnect();
}
