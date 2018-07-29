#include "stdafx.h"
#include "Function_detail.h"
#include <algorithm>
#include <cstddef>

namespace is::signals::detail
{

packed_function::packed_function(packed_function&& other)
{
	if (other.is_buffer_allocated())
	{
		m_proxy = other.m_proxy->clone(&m_buffer);
	}
	else
	{
		m_proxy = other.m_proxy;
		other.m_proxy = nullptr;
	}
}

packed_function::packed_function(const packed_function& other)
{
	m_proxy = other.m_proxy->clone(&m_buffer);
}

packed_function& packed_function::operator=(packed_function&& other)
{
	if (other.is_buffer_allocated())
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
	return *this;
}

packed_function& packed_function::operator=(const packed_function& other)
{
	auto* proxy = other.m_proxy->clone(&m_buffer);
	reset();
	m_proxy = proxy;
	return *this;
}

packed_function::~packed_function() noexcept
{
	reset();
}

// TODO: remove pragmas when code generation bug will be fixed.
#if defined(_MSC_VER)
#pragma optimize("", off)
#endif
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
#if defined(_MSC_VER)
#pragma optimize("", on)
#endif

base_function_proxy& packed_function::unwrap() const
{
	if (!m_proxy)
	{
		throw std::bad_function_call();
	}
	return *m_proxy;
}

bool packed_function::is_buffer_allocated() const noexcept
{
	const std::byte* bufferStart = reinterpret_cast<const std::byte*>(&m_buffer);
	const std::byte* bufferEnd = reinterpret_cast<const std::byte*>(&m_buffer + 1);
	const std::byte* proxy = reinterpret_cast<const std::byte*>(m_proxy);
	return (proxy >= bufferStart && proxy < bufferEnd);
}

uint64_t packed_function_storage::add(packed_function fn)
{
	std::lock_guard lock(m_mutex);
	uint64_t id = ++m_nextId;
	m_functions.emplace_back(std::move(fn));
	m_ids.emplace_back(id);

	return id;
}

void packed_function_storage::remove(uint64_t id) noexcept
{
	// TODO: use binary search (ids array is always sorted)
	std::lock_guard lock(m_mutex);

	// We use binary search because ids array is always sorted.
	auto it = std::lower_bound(m_ids.begin(), m_ids.end(), id);
	if (it != m_ids.end() && *it == id)
	{
		size_t i = std::distance(m_ids.begin(), it);
		m_ids.erase(m_ids.begin() + i);
		m_functions.erase(m_functions.begin() + i);
	}
}

void packed_function_storage::remove_all() noexcept
{
	std::lock_guard lock(m_mutex);
	m_functions.clear();
	m_ids.clear();
}

packed_function_storage::VectorT packed_function_storage::get_functions() const
{
	std::lock_guard lock(m_mutex);
	return m_functions;
}

} // namespace is::signals::detail
