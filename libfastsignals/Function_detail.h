#pragma once

#include <memory>
#include <atomic>
#include <vector>
#include <mutex>

namespace is::signals::detail
{
class IFunctionProxyData
{
public:
	virtual ~IFunctionProxyData() = default;
	virtual std::unique_ptr<IFunctionProxyData> Clone() const = 0;
};

template<class Signature>
class IFunctionProxy;

template <class Return, class... Arguments>
class IFunctionProxy<Return(Arguments...)> : public IFunctionProxyData
{
public:
	virtual Return operator()(Arguments...) const = 0;
};

template <class Function, class Return, class... Arguments>
class FunctionProxy final : public IFunctionProxy<Return(Arguments...)>
{
public:
	static_assert(std::is_same_v<std::invoke_result_t<Function, Arguments...>, Return>, "cannot construct function from non-callable or callable with different signature");

	template<class Function>
	explicit FunctionProxy(Function&& function)
		: m_function(std::forward<Function>(function))
	{
	}

	Return operator()(Arguments... args) const final
	{
		return std::invoke(m_function, args...);
	}

	std::unique_ptr<IFunctionProxyData> Clone() const final
	{
		return std::make_unique<FunctionProxy<Function, Return, Arguments...>>(*this);
	}

private:
	mutable Function m_function{};
};

template <class Function, class Return, class... Arguments>
class FreeFunctionProxy final : public IFunctionProxy<Return(Arguments...)>
{
public:
	static_assert(std::is_same_v<std::invoke_result_t<Function, Arguments...>, Return>, "cannot construct function from non-callable or callable with different signature");

	template<class Function>
	explicit FreeFunctionProxy(Function&& function)
		: m_function(std::forward<Function>(function))
	{
	}

	Return operator()(Arguments... args) const final
	{
		return std::invoke(m_function, args...);
	}

	std::unique_ptr<IFunctionProxyData> Clone() const final
	{
		return std::make_unique<FreeFunctionProxy<Function, Return, Arguments...>>(*this);
	}

private:
	Function m_function{};
};

class PackedFunction
{
public:
	PackedFunction() = default;

	template <class Function, class Return, class... Arguments>
	void Init(Function&& function)
	{
		using DecayFunction = typename std::remove_reference_t<std::remove_cv_t<Function>>;
		using ProxyType = typename std::conditional_t<std::is_function_v<DecayFunction>,
			FreeFunctionProxy<Function, Return, Arguments...>,
			FunctionProxy<DecayFunction, Return, Arguments...>>;

		auto proxy = std::make_unique<ProxyType>(std::forward<Function>(function));
		m_proxy.reset(proxy.release());
	}

	template<class Signature>
	const IFunctionProxy<Signature>& Get() const
	{
		return static_cast<const IFunctionProxy<Signature>&>(*m_proxy);
	}

	PackedFunction(PackedFunction&& other);
	PackedFunction(const PackedFunction& other);
	PackedFunction& operator=(PackedFunction&& other);
	PackedFunction& operator=(const PackedFunction& other);

private:
	std::unique_ptr<IFunctionProxyData> m_proxy;
};

class PackedFunctionsStorage
{
public:
	template <class Function, class Signature, class ...Args>
	unsigned Add(Function&& function)
	{
		PackedFunction packed;
		packed.Init<Function, Signature, Args...>(std::forward<Function>(function));
		AddImpl(packed);
	}

	void Remove(unsigned id);

	template<class Signature, class ...Args>
	void Invoke(Args... args) const
	{
		// TODO: (feature) add result combiners
		// TODO: (optimization) remove loop traversing from template code (use FunctionView).
		// TODO: (optimization) implement copy-on-write for functions array to avoid unnecessary copying on emit.
		for (auto&& function : GetFunctions())
		{
			function.Get<Signature>(args...);
		}
	}

private:
	unsigned AddImpl(PackedFunction function);
	std::vector<PackedFunction> GetFunctions() const;

	struct PackedFunctionWithId
	{
		PackedFunction function;
		unsigned id = 0;
	};

	mutable std::mutex m_mutex;
	std::vector<PackedFunctionWithId> m_data;
	unsigned m_nextId = 0;
};

}
