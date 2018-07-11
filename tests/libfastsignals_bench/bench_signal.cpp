#include "stdafx.h"
#include <benchmark/benchmark.h>
#include "libfastsignals/signal.h"

using namespace is::signals;

void connect_and_emit(benchmark::State& state)
{
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

BENCHMARK(connect_and_emit)->Iterations(20);
