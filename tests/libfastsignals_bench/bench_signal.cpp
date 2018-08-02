#include "stdafx.h"
#include "libfastsignals/signal.h"
#include <benchmark/benchmark.h>
#include <boost/signals2.hpp>

namespace
{
unsigned kBenchMinConnections = 0;
unsigned kBenchMaxConnections = 8;
unsigned kBenchEmitCount = 987;
} // namespace

void emit_fastsignals(benchmark::State& state)
{
	using namespace is::signals;
	for (auto _ : state)
	{
		volatile int savedValue = 0;
		for (unsigned connCount = 0; connCount < state.range(); ++connCount)
		{
			signal<void(int)> valueChanged;
			for (unsigned i = 0; i < connCount; ++i)
			{
				valueChanged.connect([&savedValue](int value) {
					savedValue = value;
				});
			}
			for (unsigned i = 0; i < kBenchEmitCount; ++i)
			{
				valueChanged(i);
			}
		}
	}
}

void emit_boost(benchmark::State& state)
{
	using namespace boost::signals2;
	for (auto _ : state)
	{
		volatile int savedValue = 0;
		for (unsigned connCount = 0; connCount < state.range(); ++connCount)
		{
			signal<void(int)> valueChanged;
			for (unsigned i = 0; i < connCount; ++i)
			{
				valueChanged.connect([&savedValue](int value) {
					savedValue = value;
				});
			}
			for (unsigned i = 0; i < kBenchEmitCount; ++i)
			{
				valueChanged(i);
			}
		}
	}
}

BENCHMARK(emit_boost)->Range(kBenchMinConnections, kBenchMaxConnections)->Iterations(150);
BENCHMARK(emit_fastsignals)->Range(kBenchMinConnections, kBenchMaxConnections)->Iterations(150);
