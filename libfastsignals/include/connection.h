#pragma once

#include "signal_impl.h"

namespace is::signals
{

// Connection keeps link between signal and slot and can disconnect them.
// Disconnect operation is thread-safe: any thread can disconnect while
//  slots called on other thread.
// This class itself is not thread-safe: you can't use the same connection
//  object from different threads at the same time.
class connection
{
public:
	connection() = default;

	explicit connection(detail::signal_impl_weak_ptr storage, uint64_t id)
		: m_storage(std::move(storage))
		, m_id(id)
	{
	}

	connection(const connection& other) = default;
	connection& operator=(const connection& other) = default;

	connection(connection&& other)
		: m_storage(other.m_storage)
		, m_id(other.m_id)
	{
		other.m_storage.reset();
		other.m_id = 0;
	}

	connection& operator=(connection&& other)
	{
		m_storage = other.m_storage;
		m_id = other.m_id;
		other.m_storage.reset();
		other.m_id = 0;
		return *this;
	}

	bool connected() const noexcept
	{
		return (m_id != 0);
	}

	void disconnect()
	{
		if (auto storage = m_storage.lock())
		{
			storage->remove(m_id);
		}
		m_storage.reset();
		m_id = 0;
	}

protected:
	detail::signal_impl_weak_ptr m_storage;
	uint64_t m_id = 0;
};

// Scoped connection keeps link between signal and slot and disconnects them in destructor.
// Scoped connection is movable, but not copyable.
class scoped_connection : public connection
{
public:
	scoped_connection() = default;

	scoped_connection(const connection& conn)
		: connection(conn)
	{
	}

	scoped_connection(connection&& conn)
		: connection(std::move(conn))
	{
	}

	scoped_connection(const scoped_connection&) = delete;
	scoped_connection& operator=(const scoped_connection&) = delete;
	scoped_connection(scoped_connection&& other) = default;

	scoped_connection& operator=(scoped_connection&& other)
	{
		disconnect();
		static_cast<connection&>(*this) = std::move(other);
		return *this;
	}

	~scoped_connection()
	{
		disconnect();
	}

	connection release()
	{
		connection conn = std::move(static_cast<connection&>(*this));
		return conn;
	}
};

} // namespace is::signals
