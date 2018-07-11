#pragma once

#include "Function_detail.h"

namespace is::signals
{

// TODO: keep weak_ptr on storage?
class connection
{
public:
	connection() = default;

	explicit connection(detail::packed_function_storage& storage, unsigned id)
		: m_storage(&storage)
		, m_id(id)
	{
	}

	connection(const connection& other) = default;
	connection& operator=(const connection& other) = default;

	connection(connection&& other)
		: m_storage(other.m_storage)
		, m_id(other.m_id)
	{
		other.m_storage = nullptr;
		other.m_id = 0;
	}

	connection& operator=(connection&& other)
	{
		m_storage = other.m_storage;
		m_id = other.m_id;
		other.m_storage = nullptr;
		other.m_id = 0;
	}

	void disconnect()
	{
		if (m_storage != nullptr)
		{
			m_storage->remove(m_id);
			m_storage = nullptr;
		}
	}

protected:
	detail::packed_function_storage* m_storage = nullptr;
	unsigned m_id = 0;
};

class scoped_connection : public connection
{
public:
	scoped_connection(const connection& conn)
		: connection(conn)
	{
	}

	scoped_connection(connection&& conn)
		: connection(std::move(conn))
	{
	}

	scoped_connection(const scoped_connection&) = delete;
	scoped_connection& operator =(const scoped_connection&) = delete;
	scoped_connection(scoped_connection&& other) = default;

	scoped_connection& operator =(scoped_connection&& other)
	{
		disconnect();
		static_cast<connection&>(*this) = std::move(other);
	}

	~scoped_connection()
	{
		disconnect();
	}
};

template<class Signature>
class signal;

template <class Return, class... Arguments>
class signal<Return(Arguments...)>
{
public:
	template<class Function>
	connection connect(Function&& function)
	{
		const unsigned id = m_slots.add<Function, Return, const Arguments&...>(std::forward<Function>(function));
		return connection(m_slots, id);
	}

	void disconnect_all()
	{
		m_slots.remove_all();
	}

	void operator()(const Arguments&... args) const
	{
		m_slots.invoke<Return(const Arguments&...), const Arguments&...>(args...);
	}

private:
	detail::packed_function_storage m_slots;
};
}
