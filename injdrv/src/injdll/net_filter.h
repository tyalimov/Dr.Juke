#pragma once

#include "map.h"
#include "mem_util.h"
#include "list.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <evntprov.h>

using namespace ownstl;

extern _snwprintf_fn_t _snwprintf;

namespace netfilter
{

	class PacketBuffer
	{
	public:

		using len_t = size_t;
		using data_t = char;

	private:

		data_t* m_buf = nullptr;
		len_t m_len_total = 0;
		len_t m_offset = 0;


	public:

		PacketBuffer() = default;

		PacketBuffer(const PacketBuffer& other) {
			*this = other;
		}

		PacketBuffer& operator=(const PacketBuffer& other)
		{
			if (this == &other) {
				return *this;
			}

			this->clear();
			this->allocate(other.m_len_total);
			this->append(other.m_buf, other.m_offset);
			return *this;
		}

		~PacketBuffer() {
			this->clear();
		}

		len_t allocate(len_t len_total)
		{
			RTL_ASSERT(this->isEmpty());
			m_buf = new data_t[len_total];
			m_len_total = len_total;
		}

		len_t append(const data_t* src, len_t length)
		{
			RTL_ASSERT(m_len_total - m_offset > length);
			ownstl::memcpy(m_buf + m_offset, src, length);
			m_offset += length;
		}

		void clear()
		{
			if (m_buf)
			{
				delete[] m_buf;
				m_buf = nullptr;
			}

			m_offset = 0;
			m_len_total = 0;
		}

		bool isFull() const {
			return m_offset == m_len_total;
		}

		bool isEmpty() const {
			return m_len_total == 0;
		}

		len_t getLength() const {
			return m_len_total;
		}

		const data_t* getData() const {
			return m_buf;
		}
	};

	namespace ws2_32
	{
		class WinSockInfo
		{

		private:

			const int MAX_SIZE = 3 * 1024 * 1024;
			USHORT m_port = 0;
			wstring m_ip;
			PacketBuffer m_pkt_buf;

		public:

			WinSockInfo() = default;
			~WinSockInfo() = default;

			void setSockAddr(const sockaddr* addr, int addr_len)
			{
				if (addr_len == sizeof(sockaddr_in))
				{
					sockaddr_in* sa = (sockaddr_in*)addr;
					char ip_str[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &sa->sin_addr, ip_str, INET_ADDRSTRLEN);

					m_ip = ToWString(string(ip_str));
					m_port = ntohs(sa->sin_port);
				}
				else
				{
					sockaddr_in6* sa6 = (sockaddr_in6*)addr;
					char ip6_str[INET6_ADDRSTRLEN];
					inet_ntop(AF_INET6, &sa6->sin6_addr, ip6_str, INET6_ADDRSTRLEN);

					m_ip = ToWString(string(ip6_str));
					m_port = ntohs(sa6->sin6_port);
				}
			}

			const wstring& getIP() const {
				return m_ip;
			}

			USHORT getPort() const {
				return m_port;
			}

			PacketBuffer& packetBuffer() {
				return m_pkt_buf;
			}
		};

		TreeMap<SOCKET, WinSockInfo> g_ws2_32;

		// post call handlers

		void on_socket(SOCKET s, int af, int type, int prot)
		{
			if (af == AF_INET || af == AF_INET6)
			{
				if (type == SOCK_STREAM)
				{
					if (prot == IPPROTO_TCP || prot == 0)
					{
						WinSockInfo wsi;
						g_ws2_32.insert(s, wsi);
					}
				}
			}
		}

		void on_closesocket(SOCKET s)
		{
			g_ws2_32.remove(s);
		}

		void on_connect(SOCKET s, const sockaddr* name, int addr_len)
		{
			WinSockInfo* wsi = g_ws2_32.find(s);
			if (wsi != nullptr)
				wsi->setSockAddr(name, addr_len);
		}

		void on_accept(SOCKET sock_parent, SOCKET sock, const sockaddr* sock_addr, int addr_len)
		{
			if (g_ws2_32.find(sock_parent) != nullptr)
			{
				WinSockInfo wsi;
				wsi.setSockAddr(sock_addr, addr_len);
				g_ws2_32.insert(sock, wsi);
			}
		}

		bool on_recv(SOCKET s, char *buf, int bytes_read)
		{
			bool status = true;
			WinSockInfo* wsi = g_ws2_32.find(s);
			if (wsi == nullptr)
				return status;

			// check it's plain text data that has apropiate size
			if (bytes_read > 0)
			{

			}

			// check page download has been finished
			auto pbuf = wsi->packetBuffer();
			pbuf.append(buf, bytes_read);
			if (pbuf.isFull()) {
				status = false; // process data
				pbuf.clear();
			}

			return status;
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


