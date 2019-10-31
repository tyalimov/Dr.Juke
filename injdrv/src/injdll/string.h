#pragma once

//
// Include NTDLL-related headers.
//
#define NTDLL_NO_INLINE_INIT_STRING
#include <ntdll.h>

namespace ownstl
{
	void* __cdecl memcpy(void* dst, const void* src, size_t num_bytes)
	{
		size_t num_4byte = num_bytes / sizeof(DWORD);
		size_t end_4byte = num_4byte * sizeof(DWORD);
		size_t num_1byte = num_bytes - end_4byte;

		for (size_t i = 0; i < num_4byte; i++)
			((DWORD*)dst)[i] = ((DWORD*)src)[i];

		for (size_t i = end_4byte; i < num_bytes; i++)
			((BYTE*)dst)[i] = ((BYTE*)src)[i];

		return dst;
	}

	void* __cdecl memset(void* dst, const char val, size_t num_bytes)
	{
		size_t num_4byte = num_bytes / sizeof(DWORD);
		size_t end_4byte = num_4byte * sizeof(DWORD);
		size_t num_1byte = num_bytes - end_4byte;

		DWORD dw_val = (val << 24) + (val << 16) + (val << 8) + val;
		for (size_t i = 0; i < num_4byte; i++)
			((DWORD*)dst)[i] = dw_val;

		for (size_t i = end_4byte; i < num_bytes; i++)
			((BYTE*)dst)[i] = val;

		return dst;
	}

	int __cdecl memcmp(const void* left, const void* right, size_t num_bytes)
	{
		size_t num_4byte = num_bytes / sizeof(DWORD);
		size_t end_4byte = num_4byte * sizeof(DWORD);
		size_t num_1byte = num_bytes - end_4byte;

		for (size_t i = 0; i < num_4byte; i++)
		{
			if (((DWORD*)left)[i] != ((DWORD*)right)[i])
			{
				if (((DWORD*)left)[i] < ((DWORD*)right)[i])
					return -1;
				else
					return 1;
			}
		}

		for (size_t i = end_4byte; i < num_bytes; i++)
		{
			if (((BYTE*)left)[i] != ((BYTE*)right)[i])
			{
				if (((BYTE*)left)[i] < ((BYTE*)right)[i])
					return -1;
				else
					return 1;
			}
		}

		return 0;
	}

	template<typename char_t>
	class basic_string
	{
	protected: // fields

		char_t* m_buffer = nullptr;
		size_t m_length = 0;

	public: // fields

		static const size_t npos = (size_t)-1;

	protected: // methods

		static void m_alloc_and_copy(
			char_t** out_buf,
			size_t* out_len,
			const char_t* str,
			size_t n = npos)
		{
			size_t len_total;
			if (n == npos)
				len_total = basic_string::strlen(str) + 1;
			else
				len_total = n + 1;

			char_t* buffer = new char_t[len_total];
			memcpy(buffer, str, len_total * sizeof(char_t));
			buffer[len_total - 1] = 0;

			*out_buf = buffer;
			*out_len = len_total - 1;
		}

		static basic_string m_empty_string()
		{
			basic_string str(true);
			str.m_buffer = new char_t[1];
			str.m_buffer[0] = 0;
			str.m_length = 0;
			return str;
		}

	public: // methods

		// static methods
		static size_t strlen(const char_t* buffer)
		{
			char_t* pch = (char_t*)buffer;
			while (*pch++) {}
			return pch - buffer - 1;
		}

		static int strcmp(const char_t* left, const char_t* right)
		{
			char_t* p_left = (char_t*)left;
			char_t* p_right = (char_t*)right;
			while (*p_left && *p_right)
			{
				if (*p_left != *p_right)
					break;

				p_left++;
				p_right++;
			}

			if (*p_left == *p_right)
				return 0;

			return *p_left > * p_right ? 1 : -1;
		}

		// constructors and destructors
		basic_string(bool empty = false)
		{
			if (!empty)
				* this = m_empty_string();
		}

		basic_string(const char_t* str)
		{
			m_alloc_and_copy(&m_buffer, &m_length, str);
		}

		basic_string(const char_t* buf, size_t n)
		{
			m_alloc_and_copy(&m_buffer, &m_length, buf, n);
		}

		basic_string(const basic_string& str)
		{
			m_alloc_and_copy(&m_buffer, &m_length, str.m_buffer, str.m_length);
		}

		~basic_string()
		{
			if (m_buffer != nullptr)
			{
				delete[] m_buffer;
				m_buffer = nullptr;
				m_length = 0;
			}
		}

		// operators
		basic_string& operator=(const basic_string& other)
		{
			if (this == &other) {
				return *this;
			}

			if (m_buffer != nullptr)
				delete[] m_buffer;

			m_alloc_and_copy(&m_buffer, &m_length, other.c_str(), other.length());
			return *this;
		}

		// utility functions
		const char_t* c_str() const
		{
			return m_buffer;
		}

		size_t length() const
		{
			return m_length;
		}

		basic_string substr(size_t begin, size_t end = npos) const
		{
			basic_string str(true);

			if (end == npos)
				end = this->length();

			size_t len_total = end - begin;
			if (len_total <= 0)
				return m_empty_string();

			str.m_length = len_total - 1;
			str.m_buffer = new char_t[len_total];
			memcpy(str.m_buffer, m_buffer + begin, len_total * sizeof(char_t));
			str.m_buffer[len_total] = 0;
			return str;
		}

		const basic_string& append(const char_t* other, size_t _len_other = npos)
		{
			// calculate total size
			size_t len_this = this->length();
			size_t len_other = _len_other == npos ? basic_string::strlen(other) : _len_other;
			size_t len_total = len_this + len_other + 1;

			// copy *this contents to new string
			basic_string<char_t> str(*this);
			delete[] m_buffer;

			// allocate buffer to hold concatenation string
			// and perform concatenation
			m_length = len_total - 1;
			m_buffer = new char_t[len_total];
			memcpy(m_buffer, str.m_buffer, len_this * sizeof(char_t));
			memcpy(m_buffer + len_this, other, len_other * sizeof(char_t));
			m_buffer[len_total - 1] = 0;
			return *this;
		}

		const basic_string& append(const basic_string& other)
		{
			return this->append(other.m_buffer, other.m_length);
		}

		size_t find(const char_t ch, size_t start = npos) const
		{
			char_t* pch = start == npos ? m_buffer : m_buffer + start;

			if (start > m_length)
				return npos;

			do
			{
				if (*pch == ch)
					break;
			} while (*(++pch));

			return *pch ? pch - m_buffer : -1;
		}

		size_t rfind(const char_t ch, size_t start = npos) const
		{
			char_t* pch = start == npos ? m_buffer + m_length : m_buffer + start;
			const char_t* pzero = m_buffer - 1;

			if (start > m_length)
				return npos;

			do
			{
				if (*pch == ch)
					break;
			} while (--pch != pzero);

			return pch == pzero ? -1 : pch - m_buffer;
		}

		size_t find(const char_t* str) const
		{
			// Function to implement strstr() function using KMP algorithm
			const char_t* X = m_buffer;
			const char_t* Y = str;
			size_t m = basic_string::strlen(X);
			size_t n = basic_string::strlen(Y);

			// Base Case 1: Y is NULL or empty
			if (*Y == 0 || n == 0)
				return 0;

			// Base Case 2: X is NULL or X's length is less than that of Y's
			if (n > m)
				return npos;

			// next[i] stores the index of next best partial match
			int* next = new int[n + 1];
			for (size_t i = 0; i < n + 1; i++)
				next[i] = 0;

			for (size_t i = 1; i < n; i++)
			{
				int j = next[i + 1];

				while (j > 0 && Y[j] != Y[i])
					j = next[j];

				if (j > 0 || Y[j] == Y[i])
					next[i + 1] = j + 1;
			}

			for (size_t i = 0, j = 0; i < m; i++)
			{
				if (*(X + i) == *(Y + j))
				{
					if (++j == n)
					{
						delete[] next;
						return i - j + 1;
					}
				}
				else if (j > 0) {
					j = next[j];
					i--;	// since i will be incremented in next iteration
				}
			}

			delete[] next;
			return npos;
		}

		size_t find(const basic_string& str) const
		{
			return this->find(str.c_str());
		}
	};

	template<typename T>
	const basic_string<T> operator+(const basic_string<T>& left, const basic_string<T>& right) {
		basic_string<T> res = left;
		return res.append(right);
	}

	template<typename T>
	const basic_string<T> operator+(const basic_string<T>& left, const T* right) {
		basic_string<T> res = left;
		return res.append(right);
	}

	template<typename T>
	const basic_string<T>& operator+=(basic_string<T>& left, const basic_string<T>& right) {
		return left.append(right);
	}

	template<typename T>
	const basic_string<T>& operator+=(basic_string<T>& left, const T* right) {
		return left.append(right);
	}

	template<typename T>
	bool operator==(const basic_string<T>& left, const basic_string<T>& right)
	{
		size_t right_length = right.length();
		size_t left_length = left.length();

		if (right_length != left_length)
			return false;

		return !memcmp(left.c_str(), right.c_str(), left_length * sizeof(T));
	}

	template<typename T>
	bool operator!=(const basic_string<T>& left, const basic_string<T>& right)
	{
		return !(left == right);
	}

	template<typename T>
	bool operator<(const basic_string<T>& left, const basic_string<T>& right)
	{
		return basic_string<T>::strcmp(left.c_str(), right.c_str()) < 0;
	}

	template<typename T>
	bool operator>(const basic_string<T>& left, const basic_string<T>& right)
	{
		return basic_string<T>::strcmp(left.c_str(), right.c_str()) > 0;
	}

	using string = basic_string<char>;
	using wstring = basic_string<wchar_t>;

	//
	// Warning!
	// Notice, that when string object is freed, the ANSI_STRING buffer is freed too
	// Use created ANSI_STRING object only when string object is present!
	//
	// Don't use it like this: ANSI_STRING us = mstl::StringToAnsiString("aaaa");
	//
	// Usage:
	//		string s = "12345";
	//		ANSI_STRING as = StringToAnsiString(s);
	//		...
	//
	ANSI_STRING ToAnsiString(const string& str)
	{
		ANSI_STRING ansi_str;
		ansi_str.Buffer = (PCHAR)str.c_str();
		ansi_str.Length = (USHORT)str.length();
		ansi_str.MaximumLength = ansi_str.Length + 1;
		return ansi_str;
	}

	// Usage:
	//		C_ANSI_STRING(as, "12345");
	//		string s = AnsiStringToString(as);
	//		...
	//
	string FromAnsiString(const ANSI_STRING& ansi_str)
	{
		return string(ansi_str.Buffer, ansi_str.Length);
	}

	//
	// Warning!
	// Notice, that when wstring object is freed, the UNICODE_STRING buffer is freed too
	// Use created UNICODE_STRING object only when wstring object is present!
	//
	// Don't use it like this: UNICODE_STRING us = mstl::WStringToUnicodeString(L"aaaa");
	//
	// Usage:
	//		string ws = L"12345";
	//		UNICODE_STRING us = WStringToUnicodeString(ws);
	//		...
	//
	UNICODE_STRING ToUnicodeString(const wstring& wstr)
	{
		UNICODE_STRING uni_str;
		uni_str.Buffer = (PWCHAR)wstr.c_str();
		uni_str.Length = (USHORT)wstr.length() * sizeof(wchar_t);
		uni_str.MaximumLength = uni_str.Length + 1 * sizeof(wchar_t);
		return uni_str;
	}

	// Usage:
	//
	//  C_UNICODE_STRING(us, L"12345");
	//	wstring ws = UnicodeStringToWString(us);
	//	...
	//
	wstring FromUnicodeString(const UNICODE_STRING& uni_str)
	{
		return wstring(uni_str.Buffer, uni_str.Length / sizeof(wchar_t));
	}

	// Usage:
	//
	//	string s = "12345";
	//	wstring ws = StringToWString(ws);
	//	...
	//
	wstring ToWString(const string& str)
	{
		UNICODE_STRING uni_str = { 0 };
		ANSI_STRING ansi_str = ToAnsiString(str);
		RtlAnsiStringToUnicodeString(&uni_str, &ansi_str, TRUE);

		wstring wstr = FromUnicodeString(uni_str);
		RtlFreeUnicodeString(&uni_str);
		return wstr;
	}

	// Usage:
	//
	//	wstring ws = L"12345";
	//	string s = WStringToString(ws);
	//	...
	//
	string ToString(const wstring& wstr)
	{
		ANSI_STRING ansi_str = { 0 };
		UNICODE_STRING uni_str = ToUnicodeString(wstr);
		RtlUnicodeStringToAnsiString(&ansi_str, &uni_str, TRUE);

		string str = FromAnsiString(ansi_str);
		RtlFreeAnsiString(&ansi_str);
		return str;
	}
}
