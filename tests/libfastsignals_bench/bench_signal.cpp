#include "stdafx.h"
#include <benchmark/benchmark.h>
#include "libfastsignals/signal.h"
#include <boost/signals2.hpp>

void emit_fastsignals(benchmark::State& state)
{
	using namespace is::signals;
	for (auto _ : state)
	{
		// Do not modify
		volatile int savedValue = 0;
		signal<void(int)> valueChanged;
		for (unsigned i = 0; i < 10; ++i)
		{
			valueChanged.connect([&savedValue](int value) {
				savedValue = value;
			});
		}
		for (unsigned i = 0; i < 1000; ++i)
		{
			valueChanged(i);
		}
	}
}

void emit_boost(benchmark::State& state)
{
	using namespace boost::signals2;
	for (auto _ : state)
	{
		// Do not modify
		volatile int savedValue = 0;
		signal<void(int)> valueChanged;
		for (unsigned i = 0; i < 10; ++i)
		{
			valueChanged.connect([&savedValue](int value) {
				savedValue = value;
			});
		}
		for (unsigned i = 0; i < 1000; ++i)
		{
			valueChanged(i);
		}
	}
}

BENCHMARK(emit_fastsignals)->Iterations(20);
BENCHMARK(emit_boost)->Iterations(20);
