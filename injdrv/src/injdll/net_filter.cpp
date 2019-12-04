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


		TreeMap<SOCKET, WinSockInfo> g_ws2_32;
		CriticalSection cs;

		inline void remove_atomic(SOCKET s)
		{
			cs.acquire();
			g_ws2_32.remove(s);
			cs.release();
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
						cs.acquire();

						WinSockInfo wsi;
						g_ws2_32.insert(s, wsi);

						cs.release();
					}
				}
			}
		}

		void on_closesocket(SOCKET s) {
			remove_atomic(s);
		}

		void on_connect(SOCKET s, const sockaddr* name, int addr_len)
		{
			cs.acquire();

			WinSockInfo* wsi = g_ws2_32.find(s);
			if (wsi != nullptr)
				wsi->setSockAddr(name, addr_len);

			cs.release();
		}

		WinSockInfo* on_recv(SOCKET s, char* buf, int bytes_read)
		{
			cs.acquire();
			WinSockInfo* wsi = g_ws2_32.find(s);
			cs.release();
			if (wsi == nullptr)
				return nullptr;

			ChunkedBuffer* cbuf = wsi->chunkedBuffer();
			if (bytes_read > 0)
			{
				// first chunk: check it's plain text data
				// if so then start tracking else remove
				if (cbuf->isEmpty())
				{
					string headers(buf, bytes_read);
					headers = ToLowerCase(headers);

					char ct[] = "content-type:";
					auto i = headers.find(ct);
					if (i == string::npos)
					{
						remove_atomic(s);
						return nullptr;
					}
					
					i += sizeof(ct);
					auto j = headers.find("\r\n", i);
					if (headers.substr(i, j).find("text/") != string::npos)
						cbuf->addChunk(buf, bytes_read);
					else
						remove_atomic(s);
				}
				else
				{
					// try to append chunked buffer
					// if max size limit reached then
					// stop tracking this file and delete buffer
					bool ok = cbuf->addChunk(buf, bytes_read);
					if (!ok)
						remove_atomic(s);
				}

				return nullptr;
			}

			// on recv completion
			PacketBuffer header = cbuf->getFirstChunk();
			PacketBuffer pbuf = cbuf->toPacketBuffer();
			header.getData(); header.getLength();
			pbuf.getData(); pbuf.getLength();
			bool status = true; // status = malware_filter()

			// cleanup and get ready
			// for another data
			cbuf->clear();
			return wsi;
		}

		wstring get_bad_sock_info(SOCKET s)
		{
			WinSockInfo* wsi = g_ws2_32.find(s);
			RTL_ASSERT(wsi != nullptr);

			wchar_t buf[256] = { 0 };
			const wchar_t* fmt = L"{'Detected':true, 'IoC':'%s', 'IP':'%s', 'Port':'%u'}";
			_snwprintf(buf, sizeof(buf), fmt,
				L"Mega IoC", wsi->getIP(), wsi->getPort());

			return wstring(buf);
		}
	}
}

