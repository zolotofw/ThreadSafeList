#pragma once
#include <memory>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <algorithm>
#include <initializer_list> 

static int counter = 0;

template<typename Type>
class ThreadSafeList
{
	friend std::ostream& operator<<(std::ostream& cout, ThreadSafeList& list)
	{
		if (!list.m_head)
		{
			return cout;
		}

		for (auto iter_node = list.m_head; iter_node != nullptr; iter_node = iter_node->next)
		{
			std::shared_lock<std::shared_mutex> lock(iter_node->node_mutex);
			cout << "thr: " << std::this_thread::get_id() << " value = " << *iter_node->data << std::endl;
			++counter;
		}

		return cout;
	}

	struct Node
	{
		Node(const Type& value, Node* prev_n = nullptr, Node* next_n = nullptr)
			:
			data(std::make_shared<Type>(value)),
			prev(prev_n),
			next(next_n)
		{

		}
		~Node()
		{
			std::cout << "node destructor = " << *data << std::endl;
		}

		std::shared_ptr<Type> data;
		Node* prev;// unique
		Node* next;// raw pointer
		std::shared_mutex node_mutex;
	};

public:
	ThreadSafeList() = default;
	ThreadSafeList(const std::initializer_list<Type>& values);
	~ThreadSafeList()
	{
		while (m_head != nullptr)
		{
			Node* remove_node = nullptr;
			{
				std::unique_lock<std::shared_mutex> lock(m_head->node_mutex);
				remove_node = m_head;
				m_head = m_head->next;
			}

			delete remove_node;
		}
	}
	inline int size() const
	{
		return m_size;
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
	std::atomic_int m_size;
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
		m_tail->next = new Node{ *begin, m_tail };
		m_tail = m_tail->next;
	}
}

template<typename Type>
inline Type ThreadSafeList<Type>::front()
{
	if (!m_head)
	{
		return {};
	}

	std::shared_lock<std::shared_lock> lock(m_head->node_mutex);

	return *m_head->data;
}

template<typename Type>
inline const Type& ThreadSafeList<Type>::front() const
{
	if (m_size <= 0)
	{
		return {};
	}

	std::shared_lock<std::shared_lock> lock(m_head->node_mutex);

	return *m_head->data;
}

template<typename Type>
inline Type ThreadSafeList<Type>::back()
{
	if (m_size <= 0)
	{
		return {};
	}

	std::shared_lock<std::shared_mutex> lock(m_tail->node_mutex);
	
	return *m_tail->data;
}

template<typename Type>
inline const Type& ThreadSafeList<Type>::back() const
{
	if (m_size <= 0)
	{
		return {};
	}

	std::unique_lock<std::shared_mutex> lock(m_tail->node_mutex);

	return *m_tail->data;
}

template<typename Type>
inline void ThreadSafeList<Type>::push_front(const Type& value)
{
	if (m_size <= 0)
	{
		m_head = new Node{ value };
		m_tail = m_head;
		++m_size;

		return;
	}

	std::unique_lock<std::shared_mutex> lock(m_head->node_mutex);

	m_head->prev = new Node{ value, nullptr, m_head };
	m_head = m_head->prev;

	++m_size;
}

template<typename Type>
inline void ThreadSafeList<Type>::push_back(const Type& value)
{
	if (m_size <= 0)
	{
		m_head = new Node{ value };
		m_tail = m_head;
		++m_size;

		return;
	}

	std::unique_lock<std::shared_mutex> lock(m_tail->node_mutex);

	m_tail->next = new Node(value, m_tail, nullptr);
	m_tail = m_tail->next;

	++m_size;
}

template<typename Type>
inline void ThreadSafeList<Type>::pop_front()
{
	if (m_size <= 0)
	{
		return;
	}

	--m_size;

	Node* remove_head = nullptr;

	{
		std::unique_lock<std::shared_mutex> lock(m_head->node_mutex);
		remove_head = m_head;
		m_head = m_head->next;
	}

	delete remove_head;
}

template<typename Type>
inline void ThreadSafeList<Type>::pop_back()
{
	if (m_size == 0)
	{
		return;
	}

	--m_size;

	Node* remove_tail = nullptr;

	{
		std::unique_lock<std::shared_mutex> lock(m_tail->node_mutex);
		remove_tail = m_tail;
		m_tail = m_tail->prev;
	}

	delete remove_tail;
}
