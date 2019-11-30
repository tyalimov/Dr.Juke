#pragma once

#pragma warning( disable: 4522 )
#include <iostream>
#include <memory_resource>
#define MAX_ELEMENTS 4294967295
namespace stl
{
	
	template <class DataType> class Iterator;
	template <class DataType> class ReverseIterator;

	typedef  std::size_t								size_type;
	typedef  std::ptrdiff_t								difference_type;

	template<class DataType> class Vector
	{
		public:
			typedef		  Iterator				<DataType>		iterator;
			typedef		  ReverseIterator		<DataType>		reverse_iterator;
			typedef		  DataType								value_type;
			typedef		  DataType&								reference;
			typedef const DataType&								const_reference;
			

		private:
			DataType* p_container;		//container to keep data
			size_type m_size;			//container size
			size_type m_extra_size;
			void init();

		public:

			Vector();
			Vector(size_type _num_elements);
			Vector(size_type _num_elements, DataType initial_value);

			~Vector();

			Vector<DataType>& operator = (Vector<DataType>&& other) noexcept;//
			Vector<DataType>& operator = (const Vector<DataType>& other);//
			DataType& operator[] (size_type ind);//
			inline bool		  operator== (Vector& other);
			inline bool		  operator!= (Vector& other);
			
			DataType& at(size_type ind);//
			DataType& front();//
			DataType& back();//
			size_type size();//
			size_type max_size();//
			size_type capacity();//
			void	  assign(size_type count, const DataType& value);//
			void	  reserve(size_type size);//
			void	  resize(size_type new_size);//
			void	  push_back(DataType element);//
			void	  shrink_to_fit();//
			void	  pop_back();//
			void	  clear();//
			bool	  empty();//

			

	};

	template <class DataType> DataType& Vector<DataType>::operator[](size_type i)//overload of indexing operator //works
	{
		return p_container[i];
	}

	template <class DataType> bool Vector<DataType>::operator==(Vector& other)
	{
		if (this->size() != other.size())
		{
			return false;
		}
		for (size_type i = 0; i < this->size(); ++i)
		{
			if (p_container[i] != other[i])
			{
				return false;
			}
		}
		return true;
	}

	template <class DataType> bool Vector<DataType>::operator!=(Vector& other)
	{
		if (this->size() != other.size())
		{
			return true;
		}
		for (size_type i = 0; i < this->size(); ++i)
		{
			if (p_container[i] != other[i])
			{
				return true;
			}
		}
		return false;
	}

	template <class DataType> void Vector<DataType>::init()
	{
		p_container = nullptr;
		m_size		 = 0;
		m_extra_size = 10;
	}

	template <class DataType> Vector <DataType>::Vector()
	{
		init();
	}

	template <class DataType> Vector <DataType>::Vector(size_type num_elements)
	{
		init();
		m_size = num_elements;
		m_extra_size = 10;
		p_container = new DataType[m_size + m_extra_size];
	}

	template <class DataType> Vector <DataType>::Vector(size_type num_elements, DataType initial_value)
	{
		init();
		m_size = num_elements;
		p_container = new DataType[m_size + m_extra_size];
		for(auto i = 0; i < m_size + m_extra_size; ++i)
		{
			p_container[i] = initial_value;
		}
	}

	template <class DataType> Vector <DataType>::~Vector()
	{
		delete[] p_container;
		m_size = 0;
	}

	template <class DataType> DataType& Vector<DataType>::at(size_type ind)
	{
		try
		{
			return p_container[ind];
		
		}
		catch (int * ex)
		{
			std::cout << "Error " << ex << std::endl;
		}
	
	}

	template <class DataType> DataType& Vector<DataType>::front()
	{
		try
		{
			return p_container[0];
		
		}
		catch (int *ex)
		{
			std::cout << "Error " << ex << std::endl;
		}
	}
	template <class DataType> DataType& Vector<DataType>::back()
	{
		try
		{
			return p_container[m_size-1];
		
		}
		catch (int *ex)
		{
			std::cout << "Error "<< ex << std::endl;
		}

	}
	
	template <class DataType> size_type Vector<DataType>::size()
	{
		return m_size;
	}
	
	template <class DataType> size_type Vector<DataType>::max_size()
	{
		return std::numeric_limits<size_type>::max();
	}


	template <class DataType> size_type Vector<DataType>::capacity()
	{
		return m_size + m_extra_size - 1;
	}

	template <class DataType> void Vector<DataType>::resize(size_type new_size)
	{
		DataType* p_local_container;
		if (new_size > MAX_ELEMENTS - 10)
		{
			return;
		}
		if (new_size == m_size)
		{
			return;
		}
		p_local_container = new DataType[new_size + m_extra_size];

		if (new_size > m_size)
		{
			for (auto i = 0; i < m_size; ++i)
			{
				p_local_container[i] = p_container[i];
			}
			delete[] p_container;
			p_container = new DataType[new_size + m_extra_size];
			memcpy_s(p_container, new_size, p_local_container, m_size);
		}
		else if (new_size < m_size)
		{
			for (auto i = 0; i < new_size; ++i)
			{
				p_local_container[i] = p_container[i];
			}
			delete[] p_container;
			p_container = new DataType[new_size + m_extra_size];
			memcpy_s(p_container, new_size, p_local_container, new_size);
		}
		
		m_size = new_size;
		m_extra_size = 10;
		delete[] p_local_container;
	}

	template <class DataType> void Vector<DataType>::assign(size_type count, const DataType& value)
	{
		if (m_size < count) { resize(count); }

		for (auto i = 0; i < count; ++i)
		{
			p_container[i] = value;
		}
	}

	template <class DataType> void Vector<DataType>::reserve(size_type size) // TODO: DO IT
	{
		DataType* p_local_contrainer = new DataType[size + m_extra_size];
		memcpy_s(p_local_contrainer, size * sizeof(DataType), p_container, size * sizeof(DataType));
		delete[] p_container;
		p_container = new DataType[size + m_extra_size];
		p_container = p_local_contrainer;
		delete[] p_local_contrainer;
		m_size = size;
		m_extra_size = 10;

	}

	template <class DataType> void Vector<DataType>::push_back(DataType element)
	{
		if (!m_extra_size){ resize(m_size+m_extra_size); }

		m_size+=1;
		p_container[m_size] = element;
		m_extra_size-=1;
	}

	template <class DataType> void Vector<DataType>::shrink_to_fit()
	{
		DataType* p_local_container = new DataType[m_size];

		for (auto i = 0; i < m_size; ++i)
		{
			p_local_container[i] = p_container[i];
		}

		delete[] p_container;
		p_container = new DataType[m_size+1];
		memcpy(p_container, p_local_container, m_size);
		delete[] p_local_container;
		m_extra_size = 1;
	}

	template <class DataType> void Vector<DataType>::pop_back()
	{
		if (m_size == 0) { return; }

		memset(p_container + (m_size - 1), 0, sizeof(DataType));
		m_size -= 1;
		m_extra_size += 1;
	}

	template <class DataType> void Vector<DataType>::clear()
	{
		memset(p_container, 0, sizeof(DataType) * m_size + m_extra_size - 2);
		m_extra_size = m_size + m_extra_size;
		m_size = 0;
	}

	template <class DataType> bool Vector<DataType>::empty()
	{
		return !m_size;
	}





	template<class DataType> class Iterator
	{
	    
	    public:
			friend Vector<DataType>;
	    	typedef Iterator <DataType> iterator;
	    	using value_type = DataType;
	    	using reference = value_type&;
	    	using pointer = value_type*;
	    	using difference_type = std::ptrdiff_t;
	    	using iterator_category = std::random_access_iterator_tag;
	    
	    private:
	    	DataType* p_current_value;
	    	DataType* p_container;
	    	size_type m_index;
	    
	    public:
	    	Iterator();
	    	Iterator(DataType* initial_value, DataType* initial_container,size_type initial_index);
	    	
	    	~Iterator();
	    
			typename Vector<DataType>::Iterator& operator++();
			typename Vector<DataType>::Iterator operator++(int);
			typename Vector<DataType>::Iterator& operator--();
			typename Vector<DataType>::Iterator operator--(int);
			typename Vector<DataType>::Iterator& operator+=(size_type value);
			typename Vector<DataType>::Iterator& operator+(size_type value);
			typename Vector<DataType>::Iterator& operator-=(size_type value);
			typename Vector<DataType>::Iterator& operator-(size_type value);
			typename Vector<DataType>::difference_type operator-(Iterator other);
			typename Vector<DataType>::DataType& operator*();
	    	bool operator==(Iterator& other);
	    	bool operator!=(Iterator& other);
	    	bool operator<(Iterator other);

			typename Vector<DataType>::Iterator erase(Iterator position);
			typename Vector<DataType>::Iterator insert(Iterator position, const DataType& value);
			typename Vector<DataType>::Iterator begin();
			typename Vector<DataType>::Iterator end();

	};
	template<class DataType> Iterator<DataType>::Iterator()
	{
		p_current_value = nullptr;
		p_container		= nullptr;
		m_index			= 0;
	}
	template<class DataType> Iterator<DataType>::Iterator(DataType* initial_value, DataType* initial_container, size_type initial_index)
	{
		p_container = initial_container;
		p_current_value = initial_value;
		m_index = initial_index;
	}

	template<class DataType> typename Vector<DataType>::Iterator& Iterator<DataType>::operator ++()
	{
		m_index ++;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::Iterator Iterator<DataType>::operator ++(int)
	{
		Iterator tmp = *this;
		m_index++;
		p_current_value = p_container + m_index;
		return tmp;
	}

	template<class DataType> typename Vector<DataType>::Iterator& Iterator<DataType>::operator --()
	{
		m_index--;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::Iterator Iterator<DataType>::operator --(int)
	{
		Iterator tmp = *this;
		m_index--;
		p_current_value = p_container + m_index;
		return tmp;
	}

	template<class DataType> typename Vector<DataType>::Iterator& Iterator<DataType>::operator+=(size_type value)
	{
		m_index += value;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::Iterator& Iterator<DataType>::operator+(size_type value)
	{
		m_index += value;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::Iterator& Iterator<DataType>::operator-=(size_type value)
	{
		m_index -= value;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::Iterator& Iterator<DataType>::operator-(size_type value)
	{
		m_index -= value;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::difference_type Iterator<DataType>::operator-(Iterator other)
	{
		return this->p_current_value - other.p_current_value;
	}

	template<class DataType> typename Vector<DataType>::DataType& Iterator<DataType>::operator*()
	{
		return *p_current_value;
	}

	template<class DataType> bool Iterator<DataType>::operator==(Iterator& other)
	{
		return !(p_current_value == other.p_current_value) || !(m_index == other.m_index);
	}

	template<class DataType> bool Iterator<DataType>::operator!=(Iterator& other)
	{
		return (p_current_value != other.p_current_value) && (m_index != other.m_index);
	}

	template<class DataType> bool Iterator<DataType>::operator<(Iterator other)
	{
		return this->p_current_value < other.p_current_value || (this->p_current_value == other.p_current_value && this->m_index < other.m_index);
	}
	template<class DataType> typename Vector<DataType>::Iterator Iterator<DataType>::insert(Iterator position, const DataType& value)//works
	{
		Vector obj;
		size_type index = position.m_index();
		size_type index_for_return = index;
		if (obj.m_extra_size < 1)
		{
			Vector::resize(obj.m_size);
		}
		DataType tmp_first = p_container[index];
		DataType tmp_second;
		p_container[index] = value;
		for (index += 1; index <= obj.m_size; ++index)
		{
			tmp_second = p_container[index];
			p_container[index] = tmp_first;
			tmp_first = tmp_second;
		}
		obj.m_size++;
		obj.m_extra_size--;
		typename Vector<DataType>::Iterator it(p_container + index_for_return, p_container, index_for_return);
		
		return it;
	}
	template<class DataType> typename Vector<DataType>::Iterator Iterator<DataType>::erase(Iterator position)//works
	{	
		Vector obj;
		size_type index = position.m_index;
		size_type index_for_return = index;
		for (; index < obj.m_size - 1; ++index)
		{
			p_container[index] = p_container[index + 1];
		}
		obj.m_size--;
		obj.m_extra_size++;
		typename Vector<DataType>::Iterator it(p_container + index_for_return, p_container, index_for_return);
		return it;
	}
	template<class DataType> typename Vector<DataType>::Iterator Iterator<DataType>::begin()
	{
		typename Vector<DataType>::Iterator it(&p_container[0], p_container, 0);
		return it;
	}

	template<class DataType> typename Vector<DataType>::Iterator Iterator<DataType>::end()
	{
		Vector obj;
		typename Vector<DataType>::Iterator it(&p_container[obj.m_size], p_container, obj.m_size);
		return it;
	}



	template<class DataType> class ReverseIterator
	{
		public:
			friend Vector<DataType>;

	    private:
	    	DataType* p_current_value;
	    	DataType* p_container;
	    	size_type m_index;
	    
	    public:
	    	ReverseIterator();
	    	ReverseIterator(DataType* initial_value, DataType* initial_container, size_type initial_index);
	    
	    	~ReverseIterator();
	    
			typename Vector<DataType>::ReverseIterator& operator++();
			typename Vector<DataType>::ReverseIterator operator++(int);
			typename Vector<DataType>::ReverseIterator& operator--();
			typename Vector<DataType>::ReverseIterator operator--(int);
			typename Vector<DataType>::ReverseIterator& operator+=(size_type value);
			typename Vector<DataType>::ReverseIterator& operator+(size_type value);
			typename Vector<DataType>::ReverseIterator& operator-=(size_type value);
			typename Vector<DataType>::ReverseIterator& operator-(size_type value);
	    	DataType& operator*();
	    	bool operator==(ReverseIterator& other);
	    	bool operator!=(ReverseIterator& other);

			typename Vector<DataType>::ReverseIterator rbegin();
			typename Vector<DataType>::ReverseIterator rend();


	};

	template<class DataType> ReverseIterator<DataType>::ReverseIterator()
	{
		p_current_value = nullptr;
		p_container = nullptr;
		m_index = 0;
	}
	template<class DataType> ReverseIterator<DataType>::ReverseIterator(DataType* initial_value, DataType* initial_container, size_type initial_index)
	{
		p_container = initial_container;
		p_current_value = initial_value;
		m_index = initial_index;
	}
	template<class DataType> ReverseIterator<DataType>::~ReverseIterator()
	{
		p_container = nullptr;
		p_current_value = nullptr;
		m_index = 0;
	}
	template<class DataType> typename Vector<DataType>::ReverseIterator& ReverseIterator<DataType>::operator ++()
	{
		m_index--;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::ReverseIterator ReverseIterator<DataType>::operator ++(int)
	{
		Iterator tmp = *this;
		m_index--;
		p_current_value = p_container + m_index;
		return tmp;
	}

	template<class DataType> typename Vector<DataType>::ReverseIterator& ReverseIterator<DataType>::operator --()
	{
		m_index++;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::ReverseIterator ReverseIterator<DataType>::operator --(int)
	{
		Iterator tmp = *this;
		m_index++;
		p_current_value = p_container + m_index;
		return tmp;
	}

	template<class DataType> typename Vector<DataType>::ReverseIterator& ReverseIterator<DataType>::operator+=(size_type value)
	{
		m_index -= value;
		p_current_value = p_container - m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::ReverseIterator& ReverseIterator<DataType>::operator+(size_type value)
	{
		m_index -= value;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::ReverseIterator& ReverseIterator<DataType>::operator-=(size_type value)
	{
		m_index += value;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> typename Vector<DataType>::ReverseIterator& ReverseIterator<DataType>::operator-(size_type value)
	{
		m_index += value;
		p_current_value = p_container + m_index;
		return *this;
	}

	template<class DataType> DataType& ReverseIterator<DataType>::operator*()
	{
		return *p_current_value;
	}

	template<class DataType> bool ReverseIterator<DataType>::operator==(ReverseIterator& other)
	{
		return !(p_current_value == other.p_current_value) || !(m_index == other.m_index);
	}

	template<class DataType> bool ReverseIterator<DataType>::operator!=(ReverseIterator& other)
	{
		return (p_current_value != other.p_current_value) && (m_index != other.m_index);
	}

	template<class DataType> typename Vector<DataType>::ReverseIterator ReverseIterator<DataType>::rbegin()
	{
		typename Vector<DataType>::ReverseIterator it(&p_container[0], p_container, 0);
		return it;
	}

	template<class DataType> typename Vector<DataType>::ReverseIterator ReverseIterator<DataType>::rend()
	{
		Vector obj;
		typename Vector<DataType>::ReverseIterator it(&p_container[obj.m_size], p_container, obj.m_size);
		return it;
	}
}

