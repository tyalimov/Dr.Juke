
//
// Include NTDLL-related headers.
//
#define NTDLL_NO_INLINE_INIT_STRING
#include <ntdll.h>

#include "mstl_new.h"

namespace abc
{

  void* __memcpy(void* dst, const void* src, size_t num_bytes)
  {
    size_t num_4byte = num_bytes / sizeof(DWORD);
    size_t end_4byte = num_4byte * sizeof(DWORD);
    size_t num_1byte = num_bytes - end_4byte;

    for (int i = 0; i < num_4byte; i++)
      ((DWORD*)dst)[i] = ((DWORD*)src)[i];

    for (int i = end_4byte; i < num_bytes; i++)
      ((BYTE*)dst)[i] = ((BYTE*)src)[i];
  }

  void* __memset(void* dst, const char val, size_t num_bytes)
  {
    size_t num_4byte = num_bytes / sizeof(DWORD);
    size_t end_4byte = num_4byte * sizeof(DWORD);
    size_t num_1byte = num_bytes - end_4byte;

    DWORD dw_val = (val << 24) + (val << 16) + (val << 8) + val;
    for (int i = 0; i < num_4byte; i++)
      ((DWORD*)dst)[i] = dw_val;

    for (int i = end_4byte; i < num_bytes; i++)
      ((BYTE*)dst)[i] = val;
  }

  class wstring
  {
    wstring(const WCHAR*);
    wstring(UNICODE_STRING);

    ~wstring();
  };

  template<typename char_t>
  class basic_string
  {
  protected:

    T* m_buffer = nullptr;

    size_t length()
    {
      return m_strlen(m_buffer);
    }

    static size_t m_strlen(const char_t* buffer)
    {
      char_t* pch = (char_t*)buffer;
      while (*pch++);
      return pch - buffer;
    }

    string() = default;

  public:

    basic_string(const char_t* str)
    {
      size_t len_total = m_strlen(str) + 1;
      m_buffer = new char_t[len_total];
      memcpy(m_buffer, str, len_total * sizeof(char_t));
    }

    basic_string(basic_string& str)
    {
      size_t len_total = str.length() + 1;
      m_buffer = new char_t[len_total];
      memcpy(m_buffer, str.c_str(), len_total * sizeof(char_t));
    }

    ~basic_string()
    {
      if (m_buffer != nullptr)
      {
        delete[] m_buffer;
        m_buffer = nullptr;
      }
    }

    // utility functions

    const char_t* c_str() const
    {
      return m_buffer;
    }

    basic_string substr(size_t begin, size_t end)
    {
      basic_string str;
      size_t len_total = end - begin + 1;

      str.m_buffer = new char_t[len_total];
      memcpy(str.m_buffer, m_buffer + begin, len_total * sizeof(char_t));
      return str;
    }

    basic_string append(const char_t* other)
    {
      basic_string str;
      size_t len_this = this->length();
      size_t len_other = m_strlen(other);
      size_t len_total = len_this + len_other + 1;

      str.m_buffer = new char_t[len_total];
      len_this *= sizeof(char_t);
      len_other *= sizeof(char_t);

      memcpy(str.m_buffer, m_buffer, len_this);
      memcpy(str.m_buffer + len_this, other.m_buffer, len_other);

      delete[] m_buffer;
      m_buffer = str.m_buffer;
    }

    basic_string append(const basic_string& other)
    {
      this->append(other.m_buffer);
    }

    size_t find(const char_t ch)
    {
      for (char_t* pch = m_buffer; *pch && *pch != ch; pch++);
      return *pch ? pch - m_buffer : -1;
    }

    size_t find(const char_t* str)
    {
      // Function to implement strstr() function using KMP algorithm
      const char_t* X = m_buffer;
      const char_t* Y = str;
      size_t m = m_strlen(X);
      size_t n = m_strlen(Y);

      // Base Case 1: Y is NULL or empty
      if (*Y == 0 || n == 0)
        return X;

      // Base Case 2: X is NULL or X's length is less than that of Y's
      if (*X == 0 || n > m)
        return NULL;

      // next[i] stores the index of next best partial match
      int next[n + 1];

      for (int i = 0; i < n + 1; i++)
        next[i] = 0;

      for (int i = 1; i < n; i++)
      {
        int j = next[i + 1];

        while (j > 0 && Y[j] != Y[i])
          j = next[j];

        if (j > 0 || Y[j] == Y[i])
          next[i + 1] = j + 1;
      }

      for (int i = 0, j = 0; i < m; i++)
      {
        if (*(X + i) == *(Y + j))
        {
          if (++j == n)
            return (X + i - j + 1);
        }
        else if (j > 0) {
          j = next[j];
          i--;	// since i will be incremented in next iteration
        }
      }

      return -1;
    }
  };


  class string
  {
  private:

    char* m_buffer = nullptr;

    size_t length()
    {
      m_strlen(m_buffer);
    }

    static size_t m_strlen(const char* buffer)
    {
      char* pch = (char*)buffer;
      while (*pch++);
      return pch - buffer;
    }

    string() = default;

  public:

    string(const char* str)
    {
      size_t len_total = m_strlen(str) + sizeof(char);
      m_buffer = new char[len_total];
      memcpy(m_buffer, str, len_total);
    }

    string(string& str)
    {
      size_t len_total = str.length() + sizeof(char);
      m_buffer = new char[len_total];
      memcpy(m_buffer, str.c_str(), len_total);
    }

    string(ANSI_STRING ansi_str)
    {
      size_t len_total = ansi_str.Length;
      m_buffer = new char[len_total];
      memcpy(m_buffer, ansi_str.Buffer, len_total);
    }

    ~string()
    {
      if (m_buffer != nullptr)
      {
        delete[] m_buffer;
        m_buffer = nullptr;
      }
    }

    // utility functions

    const char* c_str() const
    {
      return m_buffer;
    }

    string substr(size_t begin, size_t end)
    {
      string str;
      size_t len_total = end - begin + sizeof(char);

      str.m_buffer = new char[len_total];
      memcpy(str.m_buffer, m_buffer + begin, len_total);
      return str;
    }
  };
}
