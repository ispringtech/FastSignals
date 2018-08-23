#include "../include/connection.h"

namespace is::signals
{

connection::connection(connection&& other)
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

connection& connection::operator=(connection&& other)
{
	m_storage = other.m_storage;
	m_id = other.m_id;
	other.m_storage.reset();
	other.m_id = 0;
	return *this;
}

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

scoped_connection::scoped_connection(connection&& conn)
	: connection(std::move(conn))
{
}

scoped_connection::scoped_connection(const connection& conn)
	: connection(conn)
{
}

scoped_connection& scoped_connection::operator=(scoped_connection&& other)
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

} // namespace is::signals