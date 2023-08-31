#pragma once
#include <memory>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <algorithm>
#include <initializer_list> 
#include <syncstream>
#include <string>

class Testing;

template<typename Type>
class ThreadSafeList
{
	friend class Testing;
	friend std::ostream& operator<<(std::ostream& cout, const ThreadSafeList& list)
	{
		if (list.m_head == nullptr)
		{
			return cout;
		}
		auto sync_out = std::osyncstream(cout);
		for (auto iter_node = list.m_head; iter_node != nullptr; iter_node = iter_node->next.get())
		{
			std::shared_lock<std::shared_mutex> lock(iter_node->node_mutex);
			sync_out << std::to_string(*iter_node->data) + "\n";
		}

		return cout;
	}

	struct Node
	{
		Node() = default;
		Node(const Type& value)
			:
			data(std::make_shared<Type>(value)),
			prev(nullptr)
		{

		}
		~Node()
		{

			//debug_utils::sync_print(std::this_thread::get_id(), std::string{ " node destructor = " } + std::to_string(*data));
		}
		std::shared_ptr<Type> data;
		Node* prev;// unique
		std::unique_ptr<Node> next;// raw pointer
		std::shared_mutex node_mutex;
	};

public:
	ThreadSafeList()
		:
		m_size(0),
		m_head(nullptr),
		m_tail(nullptr)
	{

	}
	ThreadSafeList(const std::initializer_list<Type>& values);
	~ThreadSafeList()
	{
		delete m_head;
	}
	inline int size() const
	{
		return m_size;
	}
	inline bool empty() const
	{
		return m_head == nullptr;
	}
	Type front();
	const Type& front() const;
	Type back();
	const Type& back() const;
	void push_front(const Type& value);
	void push_back(const Type& value);
	void pop_front();
	void pop_back();

private:
	size_t m_size;
	Node* m_head;
	Node* m_tail;
};

template<typename Type>
inline ThreadSafeList<Type>::ThreadSafeList(const std::initializer_list<Type>& values) 
{
	if (values.size() <= 0)
	{
		return;
	}

	m_size = values.size();
	auto begin = values.begin();
	auto end = values.end();

	m_head = new Node{ *begin++ };
	m_tail = m_head;

	for( ; begin != end; ++begin)
	{
		Node* new_tail = new Node{ *begin };
		new_tail->prev = m_tail;

		m_tail->next.reset(new_tail);
		m_tail = m_tail->next.get();
	}
}

template<typename Type>
inline Type ThreadSafeList<Type>::front()
{
	if (m_head == nullptr)
	{
		return {};
	}

	std::shared_lock<std::shared_lock> lock(m_head->node_mutex);

	return *m_head->data;
}

template<typename Type>
inline const Type& ThreadSafeList<Type>::front() const
{
	if (m_head == nullptr)
	{
		return {};
	}

	std::shared_lock<std::shared_lock> lock(m_head->node_mutex);

	return *m_head->data;
}

template<typename Type>
inline Type ThreadSafeList<Type>::back()
{
	if (m_tail == nullptr)
	{
		return {};
	}

	std::shared_lock<std::shared_mutex> lock(m_tail->node_mutex);
	
	return *m_tail->data;
}

template<typename Type>
inline const Type& ThreadSafeList<Type>::back() const
{
	if (m_tail == nullptr)
	{
		return {};
	}

	std::lock_guard<std::shared_mutex> lock(m_tail->node_mutex);

	return *m_tail->data;
}

template<typename Type>
inline void ThreadSafeList<Type>::push_front(const Type& value)
{
	if (m_head == nullptr)
	{
		m_head = new Node{ value };
		m_tail = m_head;
		++m_size;

		return;
	}

	{
		std::lock_guard<std::shared_mutex> lock(m_head->node_mutex);

		Node* new_head = new Node{ value };
		new_head->next.reset(m_head);
		m_head = new_head;

		++m_size;
	}
}

template<typename Type>
inline void ThreadSafeList<Type>::push_back(const Type& value)
{
	if (m_head == nullptr)
	{
		m_head = new Node{ value };
		m_tail = m_head;
		++m_size;

		return;
	}

	{
		std::lock_guard<std::shared_mutex> lock(m_tail->node_mutex);

		Node* new_tail = new Node{ value };
		new_tail->prev = m_tail;

		m_tail->next.reset(new_tail);
		m_tail = m_tail->next.get();

		++m_size;
	}
}

template<typename Type>
inline void ThreadSafeList<Type>::pop_front()
{
	if (m_head == nullptr)
	{
		return;
	}

	Node* remove_head = nullptr;

	{
		std::lock_guard<std::shared_mutex> lock(m_head->node_mutex);
		remove_head = m_head;
		m_head = m_head->next.release();

		--m_size;
	}

	delete remove_head;
}

template<typename Type>
inline void ThreadSafeList<Type>::pop_back()
{
	if (m_tail == nullptr)
	{
		return;
	}

	Node* remove_tail = nullptr;

	if(m_tail->prev != nullptr)
	{
		std::scoped_lock<std::shared_mutex, std::shared_mutex> lock(m_tail->node_mutex, m_tail->prev->node_mutex);
		m_tail = m_tail->prev;
		remove_tail = m_tail->next.release();
		--m_size;
	}
	else
	{
		std::lock_guard<std::shared_mutex> lk(m_tail->node_mutex);
		remove_tail = m_tail;
		m_tail = nullptr;
		m_head = nullptr;
		--m_size;
	}

	delete remove_tail;
}

class Testing
{
public:
	template<typename Type>
	static bool check_if_any(ThreadSafeList<Type>& test_list, const std::initializer_list<Type>& expected)
	{
		if (test_list.m_size != expected.size())
		{
			return false;
		}

		for (auto iter_node = test_list.m_head; iter_node != nullptr; iter_node = iter_node->next ? iter_node->next.get() : nullptr)
		{
			bool check = std::ranges::any_of(expected, [&current_value = *iter_node->data](const Type& exp_value)
			{
				return current_value == exp_value;
			});

			if (!check)
			{
				return false;
			}
		}

		return true;
	}
};