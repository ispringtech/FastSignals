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

unsigned packed_function_storage::add_impl(packed_function function)
{
	std::lock_guard lock(m_mutex);
	unsigned id = ++m_nextId;
	m_data.emplace_back(packed_function_with_id{ std::move(function), id });

	return id;
}

void packed_function_storage::remove(unsigned id)
{
	std::lock_guard lock(m_mutex);
	auto it = std::find_if(m_data.begin(), m_data.end(), [id](auto&& fn) {
		return fn.id == id;
	});
	if (it != m_data.end())
	{
		m_data.erase(it);
	}
}

void packed_function_storage::remove_all()
{
	std::lock_guard lock(m_mutex);
	m_data.clear();
}

std::vector<packed_function> packed_function_storage::get_functions() const
{
	std::lock_guard lock(m_mutex);
	std::vector<packed_function> result(m_data.size());
	std::transform(m_data.begin(), m_data.end(), result.begin(), [](auto&& fnWithid) {
		return fnWithid.function;
	});
	return result;
}

}
