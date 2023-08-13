#pragma once
#include <memory>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <algorithm>
#include <initializer_list> 
#include <syncstream>

static int counter = 0;

template<typename Type>
class ThreadSafeList
{
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
			sync_out << *iter_node->data << std::endl;
			++counter;
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
			std::cout <<"node destructor = " << *data << std::endl;
		}
		std::shared_ptr<Type> data;
		Node* prev;// unique
		std::unique_ptr<Node> next;// raw pointer
		std::shared_mutex node_mutex;
	};

public:
	ThreadSafeList() = default;
	ThreadSafeList(const std::initializer_list<Type>& values);
	~ThreadSafeList()
	{
		delete m_head;
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

	std::unique_lock<std::shared_mutex> lock(m_tail->node_mutex);

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

	std::unique_lock<std::shared_mutex> lock(m_head->node_mutex);

	Node* new_head = new Node{ value };
	new_head->next.reset(m_head);
	m_head = new_head;

	++m_size;
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

	std::unique_lock<std::shared_mutex> lock(m_tail->node_mutex);
	std::cout  << " push_back " << std::endl;

	Node* new_tail = new Node{ value };
	new_tail->prev = m_tail;

	m_tail->next.reset(new_tail);
	m_tail = m_tail->next.get();

	++m_size;
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
		std::unique_lock<std::shared_mutex> lock(m_head->node_mutex);
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
	{
		std::unique_lock<std::shared_mutex> lock(m_tail->node_mutex);
		remove_tail = m_tail;
		m_tail = m_tail->prev;

		--m_size;
	}

	delete remove_tail;
}
