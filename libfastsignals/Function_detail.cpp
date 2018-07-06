#include "stdafx.h"
#include "Function_detail.h"
#include <algorithm>

namespace is::signals::detail
{

PackedFunction::PackedFunction(PackedFunction&& other)
	: m_proxy(std::move(other.m_proxy))
{
}

PackedFunction::PackedFunction(const PackedFunction& other)
	: m_proxy(other.m_proxy->Clone())
{
}

PackedFunction& PackedFunction::operator=(PackedFunction&& other)
{
	m_proxy = std::move(other.m_proxy);
	return *this;
}

PackedFunction& PackedFunction::operator=(const PackedFunction& other)
{
	m_proxy = other.m_proxy->Clone();
	return *this;
}

unsigned PackedFunctionsStorage::AddImpl(PackedFunction function)
{
	std::lock_guard lock(m_mutex);
	unsigned id = ++m_nextId;
	m_data.emplace_back(PackedFunctionWithId{ std::move(function), id });

	return id;
}

void PackedFunctionsStorage::Remove(unsigned id)
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

std::vector<PackedFunction> PackedFunctionsStorage::GetFunctions() const
{
	std::lock_guard lock(m_mutex);
	std::vector<PackedFunction> result(m_data.size());
	std::transform(m_data.begin(), m_data.end(), result.begin(), [](auto&& fnWithid) {
		return fnWithid.function;
	});
	return result;
}

}
