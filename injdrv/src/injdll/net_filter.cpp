#include "net_filter.h"
#include "synch.h"

extern _snwprintf_fn_t _snwprintf;
extern _inet_ntop_t _inet_ntop;
extern _ntohs_t _ntohs;

namespace netfilter
{
	PacketBuffer& PacketBuffer::operator=(const PacketBuffer& other)
	{
		if (this == &other) {
			return *this;
		}

		this->clear();
		this->allocate(other.m_len_total);
		this->append(other.m_buf, other.m_offset);
		return *this;
	}


	data_t* PacketBuffer::allocate(len_t len_total)
	{
		RTL_ASSERT(this->isEmpty());
		m_buf = new data_t[len_total];
		m_len_total = len_total;
		return m_buf;
	}

	len_t PacketBuffer::append(const data_t* src, len_t length)
	{
		RTL_ASSERT(m_len_total - m_offset >= length);
		memcpy(m_buf + m_offset, src, length);
		m_offset += length;
		return length;
	}

	void PacketBuffer::clear()
	{
		if (m_buf)
		{
			delete[] m_buf;
			m_buf = nullptr;
		}

		m_offset = 0;
		m_len_total = 0;
	}

	ChunkedBuffer& ChunkedBuffer::operator=(const ChunkedBuffer& other)
	{
		if (this == &other) {
			return *this;
		}

		m_chunks = other.m_chunks;
		m_total_size = other.m_total_size;
		m_max_size = other.m_max_size;
		return *this;
	}

	bool ChunkedBuffer::addChunk(const data_t* src, len_t length)
	{
		// size quota exceeded
		if (m_max_size - m_total_size < length)
			return false;

		PacketBuffer pb;
		pb.allocate(length);
		pb.append(src, length);

		m_total_size += length;
		m_chunks.push_back(pb);
		return true;
	}

	PacketBuffer ChunkedBuffer::toPacketBuffer() const
	{
		PacketBuffer pb;
		pb.allocate(m_total_size);
		for (const auto& chunk : m_chunks) {
			pb.append(chunk.getData(), chunk.getLength());
		}

		RTL_ASSERT(pb.isFull());
		return pb;
	}

	namespace ws2_32
	{
		WinSockInfo& WinSockInfo::operator=(const WinSockInfo& other)
		{
			if (this == &other) {
				return *this;
			}

			m_port = other.m_port;
			m_ip = other.m_ip;
			m_pkt_buf = other.m_pkt_buf;
			return *this;
		}

		void WinSockInfo::setSockAddr(const sockaddr* addr, int addr_len)
		{
			if (addr_len == sizeof(sockaddr_in))
			{
				sockaddr_in* sa = (sockaddr_in*)addr;
				char ip_str[INET_ADDRSTRLEN];
				_inet_ntop(AF_INET, &sa->sin_addr, ip_str, INET_ADDRSTRLEN);

				m_ip = ToWString(string(ip_str));
				m_port = _ntohs(sa->sin_port);
			}
			else
			{
				sockaddr_in6* sa6 = (sockaddr_in6*)addr;
				char ip6_str[INET6_ADDRSTRLEN];
				_inet_ntop(AF_INET6, &sa6->sin6_addr, ip6_str, INET6_ADDRSTRLEN);

				m_ip = ToWString(string(ip6_str));
				m_port = _ntohs(sa6->sin6_port);
			}
		}

		TreeMap<SOCKET, WinSockInfo>* g_ws2_32 = nullptr;
		CriticalSection* cs = nullptr;

		void ws2_32_init()
		{
			g_ws2_32 = new TreeMap<SOCKET, WinSockInfo>();
			cs = new CriticalSection();
		}

		void ws2_32_exit()
		{
			if (g_ws2_32)
				delete g_ws2_32;
			if (cs)
				delete cs;
		}

		inline void remove_atomic(SOCKET s)
		{
			cs->acquire();
			g_ws2_32->remove(s);
			cs->release();
		}

		// post call handlers

		void on_socket(SOCKET s, int af, int type, int prot)
		{
			if (af == AF_INET || af == AF_INET6)
			{
				if (type == SOCK_STREAM)
				{
					if (prot == IPPROTO_TCP || prot == 0)
					{
						cs->acquire();

						WinSockInfo wsi;
						g_ws2_32->insert(s, wsi);

						cs->release();
					}
				}
			}
		}

		void on_closesocket(SOCKET s) {
			remove_atomic(s);
		}

		void on_connect(SOCKET s, const sockaddr* name, int addr_len)
		{
			cs->acquire();

			WinSockInfo* wsi = g_ws2_32->find(s);
			if (wsi != nullptr)
				wsi->setSockAddr(name, addr_len);

			cs->release();
		}

		WinSockInfo* on_recv(SOCKET s)
		{
			cs->acquire();
			WinSockInfo* wsi = g_ws2_32->find(s);
			cs->release();
			if (wsi == nullptr)
				return nullptr;

			return wsi;
		}
	}
}

