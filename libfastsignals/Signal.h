#pragma once

#include "Function_detail.h"

namespace is::signals
{
class Connection
{
public:
	Connection() = default;

	explicit Connection(detail::PackedFunctionsStorage& storage, unsigned id)
		: m_storage(&storage)
		, m_id(id)
	{
	}

	Connection(Connection&& other)
		: m_storage(other.m_storage)
		, m_id(other.m_id)
	{
		other.m_storage = nullptr;
		other.m_id = 0;
	}

	Connection& operator=(Connection&& other)
	{
		m_storage = other.m_storage;
		m_id = other.m_id;
		other.m_storage = nullptr;
		other.m_id = 0;
	}

	void Disconnect()
	{
		if (m_storage != nullptr)
		{
			m_storage->Remove(m_id);
		}
	}

protected:
	detail::PackedFunctionsStorage* m_storage = nullptr;
	unsigned m_id = 0;
};

class ScopedConnection : public Connection
{
public:
	ScopedConnection(const ScopedConnection&) = delete;
	ScopedConnection& operator =(const ScopedConnection&) = delete;
	ScopedConnection(ScopedConnection&& other) = default;

	ScopedConnection& operator =(ScopedConnection&& other)
	{
		Disconnect();
		static_cast<Connection&>(*this) = std::move(other);
	}

	~ScopedConnection()
	{
		Disconnect();
	}
};

template<class Signature>
class Signal
{
public:
	template<class Function>
	Connection Add(Function&& function)
	{
		const unsigned id = m_slots.Add(std::forward<Function>(function)));
		return Connection(m_slots, id);
	}

	void DisconnectAll()
	{
#error TODO
	}

private:
	detail::PackedFunctionsStorage m_slots;
};
}
