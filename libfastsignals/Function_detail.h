#pragma once

#include <cstdint>
#include <type_traits>

namespace is::signals::detail
{
static constexpr size_t function_buffer_size = 4 * sizeof(void*);

class base_function_proxy
{
public:
	virtual ~base_function_proxy() = default;
	virtual base_function_proxy* clone(void* buffer) const = 0;
};

template <class Signature>
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

	template <class FunctionObject>
	explicit function_proxy_impl(FunctionObject&& function)
		: m_function(std::forward<FunctionObject>(function))
	{
	}

	Return operator()(Arguments... args) const final
	{
		return std::invoke(m_function, args...);
	}

	base_function_proxy* clone(void* buffer) const final
	{
		if constexpr (sizeof(*this) > function_buffer_size)
		{
			return new function_proxy_impl(*this);
		}
		else
		{
			return new (buffer) function_proxy_impl(*this);
		}
	}

private:
	mutable Function m_function{};
};

template <class Function, class Return, class... Arguments>
class free_function_proxy_impl final : public function_proxy<Return(Arguments...)>
{
public:
	static_assert(std::is_same_v<std::invoke_result_t<Function, Arguments...>, Return>, "cannot construct function from non-callable or callable with different signature");

	template <class FunctionObject>
	explicit free_function_proxy_impl(FunctionObject&& function)
		: m_function(std::forward<FunctionObject>(function))
	{
	}

	Return operator()(Arguments... args) const final
	{
		return std::invoke(m_function, args...);
	}

	base_function_proxy* clone(void* buffer) const final
	{
		if constexpr (sizeof(*this) > function_buffer_size)
		{
			return new free_function_proxy_impl(*this);
		}
		else
		{
			return new (buffer) free_function_proxy_impl(*this);
		}
	}

private:
	Function m_function{};
};

class packed_function
{
public:
	packed_function() = default;
	packed_function(packed_function&& other);
	packed_function(const packed_function& other);
	packed_function& operator=(packed_function&& other);
	packed_function& operator=(const packed_function& other);
	~packed_function() noexcept;

	template <class Function, class Return, class... Arguments>
	void init(Function&& function)
	{
		using DecayFunction = typename std::remove_reference_t<std::remove_cv_t<Function>>;
		using ProxyType = typename std::conditional_t<std::is_function_v<DecayFunction>,
			free_function_proxy_impl<Function, Return, Arguments...>,
			function_proxy_impl<DecayFunction, Return, Arguments...>>;

		base_function_proxy* proxy = nullptr;
		if constexpr (sizeof(ProxyType) > function_buffer_size)
		{
			proxy = new ProxyType{ std::forward<Function>(function) };
		}
		else
		{
			proxy = new (&m_buffer) ProxyType{ std::forward<Function>(function) };
		}
		reset();
		m_proxy = proxy;
	}

	template <class Signature>
	const function_proxy<Signature>& get() const
	{
		return static_cast<const function_proxy<Signature>&>(unwrap());
	}

private:
	void reset() noexcept;
	base_function_proxy& unwrap() const;
	bool is_buffer_allocated() const noexcept;

	base_function_proxy* m_proxy = nullptr;
	std::aligned_storage_t<function_buffer_size> m_buffer;
};

} // namespace is::signals::detail
