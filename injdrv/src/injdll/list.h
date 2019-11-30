#pragma once

namespace ownstl
{
	template <typename T>
	class list
	{
	private:

		struct node
		{
			T data;
			node* next = NULL;
		};

		friend class iterator;

		class iterator
		{
		private:
			node* m_node;

		public:
			iterator(node* _node) : m_node(_node) {}

			T& operator*() const
			{
				return m_node->data;
			}

			iterator& operator++()
			{
				m_node = m_node->next;
				return *this;
			}

			bool operator!=(const iterator& other) const
			{
				return m_node != other.m_node;
			}
		};

		node* head = nullptr;
		node* tail = nullptr;
		int m_size = 0;

	public:
		list() = default;

		~list()
		{
			while (m_size > 0)
				pop_first();
		}

		list(const list& other)
		{
			for (const auto& data : other)
				push_back(data);
		}

		list& operator=(const list& other)
		{
			if (this == &other) {
				return *this;
			}

			for (const auto& data : other)
				push_back(data);

			return *this;
		}

		iterator begin() const {
			return iterator(head);
		}

		iterator end() const {
			return iterator(tail->next);
		}

		void push_back(const T& value)
		{
			node* temp = new node;
			m_size++;
			temp->data = value;
			temp->next = nullptr;
			if (head == nullptr)
			{
				head = temp;
				tail = temp;
				temp = nullptr;
			}
			else
			{
				tail->next = temp;
				tail = temp;
			}
		}

		int size()
		{
			return m_size;
		}

		void push_front(const T& value)
		{
			node* temp = new node;
			m_size++;
			temp->data = value;
			temp->next = head;
			head = temp;
		}

		void pop_first()
		{
			RTL_ASSERT(m_size-- > 0);

			node* temp = new node;
			temp = head;
			head = head->next;
			delete temp;

			if (m_size == 0)
			{
				head = nullptr;
				tail = nullptr;
			}
		}

		const T& get_first()
		{
			RTL_ASSERT(m_size > 0);
			return head->data;
		}

		void pop_last()
		{
			RTL_ASSERT(m_size-- > 0);

			node* current = new node;
			node* previous = new node;
			current = head;
			while (current->next != nullptr)
			{
				previous = current;
				current = current->next;
			}
			tail = previous;
			previous->next = nullptr;
			delete current;

			if (m_size == 0)
			{
				head = nullptr;
				tail = nullptr;
			}
		}
	};
}
