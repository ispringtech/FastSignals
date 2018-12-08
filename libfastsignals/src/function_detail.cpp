#include "../include/function_detail.h"
#include <cstddef>
#include <functional>

namespace is::signals::detail
{

packed_function::packed_function(packed_function&& other) noexcept
	: m_proxy(move_proxy_from(std::move(other)))
{
}

packed_function::packed_function(const packed_function& other)
	: m_proxy(copy_proxy_from(other))
{
}

packed_function& packed_function::operator=(packed_function&& other) noexcept
{
	assert(this != &other);
	reset();
	m_proxy = move_proxy_from(std::move(other));
	return *this;
}

base_function_proxy* packed_function::copy_proxy_from(const packed_function& other)
{
	return other.m_proxy ? other.m_proxy->clone(&m_buffer) : nullptr;
}

base_function_proxy* packed_function::move_proxy_from(packed_function&& other) noexcept
{
	auto proxy = other.m_proxy ? other.m_proxy->move(&m_buffer) : nullptr;
	other.m_proxy = nullptr;
	return proxy;
}

packed_function& packed_function::operator=(const packed_function& other)
{
	if (this != &other)
	{
		if (other.is_buffer_allocated() && m_proxy)
		{
			// other.m_proxy is not null
			function_buffer_t tempBuffer;
			auto proxy = other.m_proxy->clone(&tempBuffer);
			reset();
			m_proxy = proxy->move(&m_buffer);
		}
		else
		{
			auto newProxy = other.m_proxy ? other.m_proxy->clone(&m_buffer) : nullptr;
			reset();
			m_proxy = newProxy;
		}
	}
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
