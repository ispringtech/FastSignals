#include "../include/connection.h"
#include <mutex>

namespace is::signals
{
namespace
{
inline void null_deleter(const void*) {}

auto get_advanced_connection_impl(const advanced_connection& connection)
{
	struct advanced_connection_impl_getter : public advanced_connection
	{
		advanced_connection_impl_getter(const advanced_connection& connection)
			: advanced_connection(connection)
		{}
		using advanced_connection::m_impl;
	};
	return advanced_connection_impl_getter(connection).m_impl;
}
}

connection::connection(connection&& other) noexcept
	: m_storage(other.m_storage)
	, m_id(other.m_id)
{
	other.m_storage.reset();
	other.m_id = 0;
}

connection::connection(detail::signal_impl_weak_ptr storage, uint64_t id)
	: m_storage(std::move(storage))
	, m_id(id)
{
}

connection::connection() = default;

connection::connection(const connection& other) = default;

connection& connection::operator=(connection&& other) noexcept
{
	m_storage = other.m_storage;
	m_id = other.m_id;
	other.m_storage.reset();
	other.m_id = 0;
	return *this;
}

connection& connection::operator=(const connection& other) = default;

bool connection::connected() const noexcept
{
	return (m_id != 0);
}

void connection::disconnect()
{
	if (auto storage = m_storage.lock())
	{
		storage->remove(m_id);
		m_storage.reset();
	}
	m_id = 0;
}

scoped_connection::scoped_connection(connection&& conn) noexcept
	: connection(std::move(conn))
{
}

scoped_connection::scoped_connection(const connection& conn)
	: connection(conn)
{
}

scoped_connection::scoped_connection() = default;

scoped_connection::scoped_connection(scoped_connection&& other) noexcept = default;

scoped_connection& scoped_connection::operator=(scoped_connection&& other) noexcept
{
	disconnect();
	static_cast<connection&>(*this) = std::move(other);
	return *this;
}

scoped_connection::~scoped_connection()
{
	disconnect();
}

connection scoped_connection::release()
{
	connection conn = std::move(static_cast<connection&>(*this));
	return conn;
}

bool advanced_connection::advanced_connection_impl::is_blocked() const noexcept
{
	std::lock_guard lk(m_blockerMutex);
	return !m_blocker.expired();
}

std::shared_ptr<void> advanced_connection::advanced_connection_impl::block()
{
	std::lock_guard lk(m_blockerMutex);
	auto blocker = m_blocker.lock();
	if (!blocker)
	{
		blocker = std::shared_ptr<void>(nullptr, null_deleter);
		m_blocker = blocker;
	}
	return blocker;
}

advanced_connection::advanced_connection() = default;

advanced_connection::advanced_connection(connection&& conn, impl_ptr&& impl) noexcept
	: connection(std::move(conn))
	, m_impl(std::move(impl))
{
}

advanced_connection::advanced_connection(const advanced_connection&) = default;

advanced_connection::advanced_connection(advanced_connection&& other) noexcept = default;

advanced_connection& advanced_connection::operator=(const advanced_connection&) = default;

advanced_connection& advanced_connection::operator=(advanced_connection&& other) noexcept = default;

shared_connection_block::shared_connection_block(const advanced_connection& connection, bool initially_blocked)
	: m_connection(get_advanced_connection_impl(connection))
{
	if (initially_blocked)
	{
		block();
	}
}

void shared_connection_block::block()
{
	if (!blocking())
	{
		if (auto connection = m_connection.lock())
		{
			m_block = connection->block();
		}
		else
		{
			// Make m_block non-empty so the blocking() method still returns the correct value
			// after the connection has expired.
			m_block = std::shared_ptr<void>(nullptr, null_deleter);
		}
	}
}

void shared_connection_block::unblock()
{
	m_block.reset();
}

bool shared_connection_block::blocking() const noexcept
{
	return m_block != nullptr;
}

advanced_scoped_connection::advanced_scoped_connection() = default;

advanced_scoped_connection::advanced_scoped_connection(const advanced_connection& conn)
	: advanced_connection(conn)
{
}

advanced_scoped_connection::advanced_scoped_connection(advanced_connection&& conn) noexcept
	: advanced_connection(std::move(conn))
{
}

advanced_scoped_connection::advanced_scoped_connection(advanced_scoped_connection&& other) noexcept = default;

advanced_scoped_connection& advanced_scoped_connection::operator=(advanced_scoped_connection&& other) noexcept = default;

advanced_scoped_connection::~advanced_scoped_connection()
{
	disconnect();
}

advanced_connection advanced_scoped_connection::release()
{
	advanced_connection conn = std::move(static_cast<advanced_connection&>(*this));
	return conn;
}

} // namespace is::signals
