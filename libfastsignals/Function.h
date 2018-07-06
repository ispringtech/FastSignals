#pragma once

#include "Function_detail.h"

namespace is::signals
{

template<class Signature>
class Function;

// Compact function class - causes minimal code bloat when compiled.
// Replaces std::function in this library.
template <class Return, class... Arguments>
class Function<Return(Arguments...)>
{
public:
	Function() = default;

	template <class Fn>
	Function(Fn&& function)
	{
		m_packed.Init<Fn, Return, Arguments...>(std::forward<Fn>(function));
	}

	Return operator()(Arguments&&... args) const
	{
		auto& proxy = m_packed.Get<Return(Arguments...)>();
		return proxy(std::forward<Arguments>(args)...);
	}

private:
	detail::PackedFunction m_packed;
};

}
