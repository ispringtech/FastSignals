#pragma once

#include "Function.h"
#include <type_traits>

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

	explicit connection(detail::packed_function_storage_weak_ptr storage, uint64_t id)
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

	void disconnect()
	{
		if (auto storage = m_storage.lock())
		{
			storage->remove(m_id);
			m_storage.reset();
		}
	}

protected:
	detail::packed_function_storage_weak_ptr m_storage;
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
};

template <typename T>
struct signal_arg
{
	using type = const T&;
};

template <typename U>
struct signal_arg<U&>
{
	using type = U&;
};

template <typename T>
using signal_arg_t = typename signal_arg<T>::type;

template <class Signature>
class signal;

// Signal allows to fire events to many subscribers (slots).
// In other words, it implements one-to-many relation between event and listeners.
// Signal implements observable object from Observable pattern.
template <class Return, class... Arguments>
class signal<Return(Arguments...)>
{
public:
	using signature_type = Return(signal_arg_t<Arguments>...);
	using slot_type = function<signature_type>;

	signal()
		: m_slots(std::make_shared<detail::packed_function_storage>())
	{
	}

	/**
	 * connect(slot) method subscribes slot to signal emission event.
	 * Each time you call signal as functor, all slots are also called with given arguments.
	 * @returns connection - object which manages signal-slot connection lifetime
	 */
	connection connect(slot_type slot)
	{
		const uint64_t id = m_slots->add(slot.pack());
		return connection(m_slots, id);
	}

	/**
	 * disconnect_all() method disconnects all slots from signal emission event.
	 */
	void disconnect_all()
	{
		m_slots->remove_all();
	}

	/**
	 * operator(args...) calls all slots connected to this signal.
	 * Logically, it fires signal emission event.
	 */
	void operator()(signal_arg_t<Arguments>... args) const
	{
		m_slots->invoke<signature_type, signal_arg_t<Arguments>...>(args...);
	}

private:
	detail::packed_function_storage_ptr m_slots;
};
} // namespace is::signals
