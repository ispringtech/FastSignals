#pragma once

namespace is::signals
{
namespace detail
{
template <typename Signature>
struct signature_result_t;

template <class Return, class... Arguments>
struct signature_result_t<Return(Arguments...)>
{
	using type = Return;
};

template <typename T>
struct signal_arg
{
	using type = const T&;
};

template <typename U>
struct signal_arg<U&>
{
	using type = U&;
};
} // namespace detail

template <typename T>
using signal_arg_t = typename detail::signal_arg<T>::type;

template <class Signature>
using signature_result_t = typename detail::signature_result_t<Signature>::type;

} // namespace is::signals
