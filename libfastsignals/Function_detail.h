#pragma once

#include <memory>
#include <atomic>
#include <vector>
#include <mutex>

namespace is::signals::detail
{
class base_function_proxy
{
public:
	virtual ~base_function_proxy() = default;
	virtual std::unique_ptr<base_function_proxy> Clone() const = 0;
};

template<class Signature>
class function_proxy;

template <class Return, class... Arguments>
class function_proxy<Return(Arguments...)> : public base_function_proxy
{
public:
	virtual Return operator()(Arguments...) const = 0;
};

template <class Function, class Return, class... Arguments>
class function_proxy_impl final : public function_proxy<Return(Arguments...)>
{
public:
	static_assert(std::is_same_v<std::invoke_result_t<Function, Arguments...>, Return>,
		"cannot construct function from non-callable or callable with different signature");

	template<class Function>
	explicit function_proxy_impl(Function&& function)
		: m_function(std::forward<Function>(function))
	{
	}

	Return operator()(Arguments... args) const final
	{
		return std::invoke(m_function, args...);
	}

	std::unique_ptr<base_function_proxy> Clone() const final
	{
		return std::make_unique<function_proxy_impl<Function, Return, Arguments...>>(*this);
	}

private:
	mutable Function m_function{};
};

template <class Function, class Return, class... Arguments>
class free_function_proxy_impl final : public function_proxy<Return(Arguments...)>
{
public:
	static_assert(std::is_same_v<std::invoke_result_t<Function, Arguments...>, Return>, "cannot construct function from non-callable or callable with different signature");

	template<class Function>
	explicit free_function_proxy_impl(Function&& function)
		: m_function(std::forward<Function>(function))
	{
	}

	Return operator()(Arguments... args) const final
	{
		return std::invoke(m_function, args...);
	}

	std::unique_ptr<base_function_proxy> Clone() const final
	{
		return std::make_unique<free_function_proxy_impl<Function, Return, Arguments...>>(*this);
	}

private:
	Function m_function{};
};

class packed_function
{
public:
	packed_function() = default;

	template <class Function, class Return, class... Arguments>
	void init(Function&& function)
	{
		using DecayFunction = typename std::remove_reference_t<std::remove_cv_t<Function>>;
		using ProxyType = typename std::conditional_t<std::is_function_v<DecayFunction>,
			free_function_proxy_impl<Function, Return, Arguments...>,
			function_proxy_impl<DecayFunction, Return, Arguments...>>;

		auto proxy = std::make_unique<ProxyType>(std::forward<Function>(function));
		m_proxy.reset(proxy.release());
	}

	template<class Signature>
	const function_proxy<Signature>& get() const
	{
		return static_cast<const function_proxy<Signature>&>(*m_proxy);
	}

	packed_function(packed_function&& other);
	packed_function(const packed_function& other);
	packed_function& operator=(packed_function&& other);
	packed_function& operator=(const packed_function& other);

private:
	std::unique_ptr<base_function_proxy> m_proxy;
};

class packed_function_storage
{
public:
	template <class Function, class Return, class ...Args>
	unsigned add(Function&& function)
	{
		packed_function packed;
		packed.init<Function, Return, Args...>(std::forward<Function>(function));
		return add_impl(packed);
	}

	void remove(unsigned id);

	void remove_all();

	template<class Signature, class ...Args>
	void invoke(Args... args) const
	{
		// TODO: (feature) add result combiners
		// TODO: (optimization) implement copy-on-write for functions array to avoid unnecessary copying on emit.
		// TODO: (optimization) remove loop traversing from template code (use FunctionView).
		for (auto&& function : get_functions())
		{
			function.get<Signature>()(args...);
		}
	}

private:
	unsigned add_impl(packed_function function);
	std::vector<packed_function> get_functions() const;

	struct packed_function_with_id
	{
		packed_function function;
		unsigned id = 0;
	};

	mutable std::mutex m_mutex;
	std::vector<packed_function_with_id> m_data;
	unsigned m_nextId = 0;
};

}
