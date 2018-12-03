#include "../include/function_detail.h"
#include <cstddef>
#include <functional>

namespace is::signals::detail
{

packed_function::packed_function(packed_function&& other) noexcept
{
	m_isBufferAllocated = other.m_isBufferAllocated;
	if (m_isBufferAllocated)
	{
		m_proxy = other.m_proxy->clone(&m_buffer);
	}
	else
	{
		m_proxy = other.m_proxy;
		other.m_proxy = nullptr;
	}
	other.reset();
}

packed_function::packed_function(const packed_function& other)
{
	m_isBufferAllocated = other.m_isBufferAllocated;
	m_proxy = other.m_proxy->clone(&m_buffer);
}

packed_function& packed_function::operator=(packed_function&& other) noexcept
{
	if (other.m_isBufferAllocated)
	{
		// There are no strong exception safety since we cannot allocate
		//  new object on buffer and than reset same buffer.
		reset();
		m_proxy = other.m_proxy->clone(&m_buffer);
	}
	else
	{
		reset();
		std::swap(m_proxy, other.m_proxy);
	}
	m_isBufferAllocated = other.m_isBufferAllocated;
	return *this;
}

packed_function& packed_function::operator=(const packed_function& other)
{
	auto* proxy = other.m_proxy->clone(&m_buffer);
	reset();
	m_proxy = proxy;
	m_isBufferAllocated = other.m_isBufferAllocated;
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
		if (m_isBufferAllocated)
		{
			m_proxy->~base_function_proxy();
		}
		else
		{
			delete m_proxy;
		}
		m_proxy = nullptr;
	}
	m_isBufferAllocated = false;
}

base_function_proxy& packed_function::unwrap() const
{
	if (m_proxy == nullptr)
	{
		throw std::bad_function_call();
	}
	return *m_proxy;
}

} // namespace is::signals::detail
