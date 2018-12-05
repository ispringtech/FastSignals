#include "../include/function_detail.h"
#include <cstddef>
#include <functional>

namespace is::signals::detail
{

packed_function::packed_function(packed_function&& other) noexcept
{
	m_proxy = other.m_proxy ? other.m_proxy->move(&m_buffer) : nullptr;
	other.m_proxy = nullptr;
}

packed_function::packed_function(const packed_function& other)
{
	m_proxy = other.m_proxy ? other.m_proxy->clone(&m_buffer) : nullptr;
}

packed_function& packed_function::operator=(packed_function&& other) noexcept
{
	reset();
	m_proxy = other.m_proxy ? other.m_proxy->move(&m_buffer) : nullptr;
	other.m_proxy = nullptr;
	return *this;
}

packed_function& packed_function::operator=(const packed_function& other)
{
	auto* proxy = other.m_proxy ? other.m_proxy->clone(&m_buffer) : nullptr;
	reset();
	m_proxy = proxy;
	return *this;
}

packed_function::~packed_function() noexcept
{
	reset();
}

void packed_function::reset() noexcept
{
	if (m_proxy != nullptr)
	{
		if (is_buffer_allocated())
		{
			m_proxy->~base_function_proxy();
		}
		else
		{
			delete m_proxy;
		}
		m_proxy = nullptr;
	}
}

base_function_proxy& packed_function::unwrap() const
{
	if (m_proxy == nullptr)
	{
		throw std::bad_function_call();
	}
	return *m_proxy;
}

bool packed_function::is_buffer_allocated() const noexcept
{
	return std::less_equal<const void*>()(&m_buffer[0], m_proxy)
		&& std::less<const void*>()(m_proxy, &m_buffer[1]);
}

} // namespace is::signals::detail
