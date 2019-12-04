#pragma once

#pragma once

namespace ownstl
{
	//
	//	Usage:
	//
	//  TreeMap<int, int> map;
	//  for (int i = 0; i < 10; i++)
	//  	map.insertPair(i, i);
	//  
	//  map.insertPair(0, 0);
	//  
	//  struct array {
	//  	int i = 0;
	//  
	//  	struct kv
	//  	{
	//  		int key;
	//  		int val;
	//  	}p[10];
	//  
	//  } arr;
	//  
	//  map.forEach<array>(&arr,
	//  [](array* arr, const int& key, const int& val) {
	//  	int i = arr->i;
	//  	arr->p[i].key = key;
	//  	arr->p[i].val = val;
	//  	arr->i++;
	//  });


	template<typename TKey, typename TVal>
	class TreeMap
	{
	private:

		struct node
		{
			TKey key;
			TVal val;
			unsigned char height;
			node* left;
			node* right;
			node(const TKey& k, const TVal& v) {
				key = k; val = v;
				left = right = nullptr;
				height = 1;
			}
		};

		node* m_root = nullptr;

	private:

		unsigned char height(node* p)
		{
			return p ? p->height : 0;
		}

		int bfactor(node* p)
		{
			return height(p->right) - height(p->left);
		}

		void fixheight(node* p)
		{
			unsigned char hl = height(p->left);
			unsigned char hr = height(p->right);
			p->height = (hl > hr ? hl : hr) + 1;
		}

		node* rotateright(node* p) // правый поворот вокруг p
		{
			node* q = p->left;
			p->left = q->right;
			q->right = p;
			fixheight(p);
			fixheight(q);
			return q;
		}

		node* rotateleft(node* q) // левый поворот вокруг q
		{
			node* p = q->right;
			q->right = p->left;
			p->left = q;
			fixheight(q);
			fixheight(p);
			return p;
		}

		node* balance(node* p) // балансировка узла p
		{
			fixheight(p);
			if (bfactor(p) == 2)
			{
				if (bfactor(p->right) < 0)
					p->right = rotateright(p->right);
				return rotateleft(p);
			}
			if (bfactor(p) == -2)
			{
				if (bfactor(p->left) > 0)
					p->left = rotateleft(p->left);
				return rotateright(p);
			}
			return p; // балансировка не нужна
		}

		node* findmin(node* p) // поиск узла с минимальным ключом в дереве p 
		{
			node* tmp = p;
			while (tmp->left)
				tmp = tmp->left;

			return tmp;
		}

		node* removemin(node* p) // удаление узла с минимальным ключом из дерева p
		{
			if (p->left == 0)
				return p->right;
			p->left = removemin(p->left);
			return balance(p);
		}

		node* _insert(node* p, const TKey& k, const TVal& v)
		{
			if (p && p->key == k)
				return p;
			if (!p)
				return new node(k, v);
			if (k < p->key)
				p->left = _insert(p->left, k, v);
			else 
				p->right = _insert(p->right, k, v);

			return balance(p);
		}

		node* _remove(node* p, const TKey& k)
		{
			if (!p) return 0;
			if (k < p->key)
				p->left = _remove(p->left, k);
			else if (k > p->key)
				p->right = _remove(p->right, k);
			else //  k == p->key 
			{
				node* q = p->left;
				node* r = p->right;

				if (p == m_root)
				{
					delete p;
					m_root = nullptr;
				}

				if (!r)
					return q;

				node* min = findmin(r);
				min->right = removemin(r);
				min->left = q;
				return balance(min);
			}
			return balance(p);
		}

	public:

		TreeMap() = default;

		const TVal& insert(const TKey& key, const TVal& val)
		{
			m_root = _insert(m_root, key, val);
			return val;
		}

		TVal* find(const TKey& key, node* p = nullptr)
		{
			if (p == nullptr)
				p = m_root;

			if (key == p->key)
				return &p->val;

			else if (key < p->key && p->left)
				return find(key, p->left);

			else if (key > p->key && p->right)
				return find(key, p->right);

			else
				return nullptr;
		}

		void remove(const TKey& key)
		{
			_remove(m_root, key);
		}

		template <typename C>
		void forEach(C* p_user_data, void (*f)(C*, const TKey&, const TVal&), node* p = nullptr)
		{
			if (p == nullptr)
				p = m_root;

			if (p->left)
				forEach(p_user_data, f, p->left);

			f(p_user_data, p->key, p->val);

			if (p->right)
				forEach(p_user_data, f, p->right);
		}

		void forEachReverse(void* container, void (*f)(void*, const TKey&, const TVal&), node* p = nullptr)
		{
			if (p == nullptr)
				p = m_root;

			if (p->right)
				forEach(container, f, p->right);

			f(container, p->key, p->val);

			if (p->left)
				forEach(container, f, p->left);
		}
	};
}

