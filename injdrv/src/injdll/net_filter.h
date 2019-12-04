#pragma once

#include "str_util.h"
#include "string.h"
#include "map.h"
#include "list.h"

#include <ntdll.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace ownstl;

using _inet_ntop_t = decltype(inet_ntop)*;
using _ntohs_t = decltype(ntohs)*;


namespace netfilter
{
	using len_t = size_t;
	using data_t = char;

	class PacketBuffer
	{

	private:

		data_t* m_buf = nullptr;
		len_t m_len_total = 0;
		len_t m_offset = 0;


	public:

		PacketBuffer() = default;

		PacketBuffer(const PacketBuffer& other) {
			*this = other;
		}

		~PacketBuffer() {
			this->clear();
		}

		PacketBuffer& operator=(const PacketBuffer& other);

		data_t* allocate(len_t len_total);

		len_t append(const data_t* src, len_t length);

		void clear();

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

	class ChunkedBuffer
	{

	private:

		list<PacketBuffer> m_chunks;
		len_t m_total_size = 0;
		len_t m_max_size = 1024 * 1024;

	public:

		ChunkedBuffer() = default;
		~ChunkedBuffer() = default;

		ChunkedBuffer(const ChunkedBuffer& other) {
			*this = other;
		}

		void setMaxSize(len_t max_size) {
			m_max_size = max_size;
		}

		void clear() {
			m_chunks.clear();
			m_total_size = 0;
		}

		const PacketBuffer& getFirstChunk() const {
			return m_chunks.get_first();
		}

		ChunkedBuffer& operator=(const ChunkedBuffer& other);

		bool addChunk(const data_t* src, len_t length);

		PacketBuffer toPacketBuffer() const;

		bool isEmpty() const {
			return m_total_size == 0;
		}
	};

	namespace ws2_32
	{
		class WinSockInfo
		{

		private:

			USHORT m_port = 0;
			wstring m_ip;
			ChunkedBuffer m_pkt_buf;

		public:

			WinSockInfo() = default;
			~WinSockInfo() = default;

			WinSockInfo(const WinSockInfo& other) {
				*this = other;
			}

			WinSockInfo& operator=(const WinSockInfo& other);
			
			void setSockAddr(const sockaddr* addr, int addr_len);

			const wstring& getIP() const {
				return m_ip;
			}

			USHORT getPort() const {
				return m_port;
			}

			ChunkedBuffer* chunkedBuffer() {
				return &m_pkt_buf;
			}
		};

		// post call handlers
		void on_socket(SOCKET s, int af, int type, int prot);

		void on_closesocket(SOCKET s);

		void on_connect(SOCKET s, const sockaddr* name, int addr_len);

		WinSockInfo* on_recv(SOCKET s, char* buf, int bytes_read);

		wstring get_bad_sock_info(SOCKET s);
	}
}


