#include "stdafx.h"
#include "Function_detail.h"
#include <algorithm>

namespace is::signals::detail
{

packed_function::packed_function(packed_function&& other)
	: m_proxy(std::move(other.m_proxy))
{
}

packed_function::packed_function(const packed_function& other)
	: m_proxy(other.m_proxy->Clone())
{
}

packed_function& packed_function::operator=(packed_function&& other)
{
	m_proxy = std::move(other.m_proxy);
	return *this;
}

packed_function& packed_function::operator=(const packed_function& other)
{
	m_proxy = other.m_proxy->Clone();
	return *this;
}

uint64_t packed_function_storage::add(packed_function fn)
{
	std::lock_guard lock(m_mutex);
	uint64_t id = ++m_nextId;
	m_functions.emplace_back(std::move(fn));
	m_ids.emplace_back(id);

	return id;
}

void packed_function_storage::remove(uint64_t id)
{
	std::lock_guard lock(m_mutex);
	for (size_t i = 0, n = m_ids.size(); i < n; ++i)
	{
		if (m_ids[i] == id)
		{
			m_ids.erase(m_ids.begin() + i);
			m_functions.erase(m_functions.begin() + i);
			break;
		}
	}
}

void packed_function_storage::remove_all()
{
	std::lock_guard lock(m_mutex);
	m_functions.clear();
	m_ids.clear();
}

std::vector<packed_function> packed_function_storage::get_functions() const
{
	std::lock_guard lock(m_mutex);
	return m_functions;
}

}
