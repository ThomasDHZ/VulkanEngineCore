#include "NetConnection.h"
#include "NetworkSystem.h"

NetConnection::NetConnection()
{
}

NetConnection::~NetConnection()
{
}

bool NetConnection::StartUp(bool isServer, uint16 port)
{
#ifdef _WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
#endif
	if (!CreateUDP())	   return false;
	if (!SetNonBlocking()) return false;

	m_isServer = isServer;
	if (m_isServer)
	{
		if (!Bind(port)) return false;
	}
	return true;
}

bool NetConnection::CreateUDP()
{
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	return m_socket != INVALID_SOCKET_HANDLE;
}

bool NetConnection::SetNonBlocking()
{
#ifdef _WIN32
	u_long mode = 1;
	return ioctlsocket(m_socket, FIONBIO, &mode) == 0;
#else
	int flags = fcntl(m_socket, F_GETFL, 0);
	if (flags == -1) return false;
	return fcntl(m_socket, F_SETFL, flags | O_NONBLOCK) != -1;
#endif
}

bool NetConnection::Bind(uint16 port)
{
	sockaddr_in address = sockaddr_in
	{
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr = IN_ADDR
		{
			.s_addr = INADDR_ANY,
		},
	};

	byte reuseAddress = 0x1;
#ifdef _WIN32
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseAddress, sizeof(reuseAddress));
#else
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif
	return bind(m_socket, (sockaddr*)&address, sizeof(address)) != -1;
}

int NetConnection::SendTo(const void* data, int len, IpAddress& ip, uint16 port)
{
	sockaddr_in address = sockaddr_in
	{
		.sin_family = AF_INET,
		.sin_port = htons(port)
	};

	if (inet_pton(AF_INET, ip.data(), &address.sin_addr) <= 0) return -1;
	int result = sendto(m_socket, (const char*)data, len, 0, (sockaddr*)&address, sizeof(address));
#ifdef _WIN32
	if (result == SOCKET_ERROR) return -1;
#else
	if (result < 0) return -1;
#endif

	return result;
}

int NetConnection::ReceiveFrom(void* buffer, int maxLen, IpAddress& outIp, uint16* outPort)
{
	sockaddr_in from{};
	socklen_t fromLen = sizeof(from);
	int result = recvfrom(m_socket, (char*)buffer, maxLen, 0, (sockaddr*)&from, &fromLen);
#ifdef _WIN32
	if (result == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK) return 0;
		return -1;
	}
#else
	if (result < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN) return 0;
		return -1;
	}
#endif
	if (!outIp.empty()) inet_ntop(AF_INET, &from.sin_addr, outIp.data(), outIp.size());
	if (outPort) *outPort = ntohs(from.sin_port);
	return result;
}

bool NetConnection::Send(uint8 channel, uint8 type, const void* payload, uint16 payloadSize, IpAddress& ip, uint16 port)
{
	if (payloadSize > 1200) return false;

	byte buffer[1300];
	PacketHeader header = PacketHeader
	{
		.sequence = m_localSequence++,
		.ack = m_remoteSequence,
		.ackBits = m_remoteAckBits,
		.channel = channel,
		.type = type,
		.payloadSize = payloadSize
	};
	std::memcpy(buffer, &header, sizeof(PacketHeader));
	if (payloadSize > 0 && payload) std::memcpy(buffer + sizeof(PacketHeader), payload, payloadSize);

	int totalSize = sizeof(PacketHeader) + payloadSize;
	int sent = SendTo(buffer, totalSize, ip, port);
	return sent == totalSize;
}

bool NetConnection::Receive(PacketHeader& outHeader, void* outPayload, uint16 maxPayloadSize, IpAddress& outIp, uint16* outPort)
{
	byte buffer[1300];
	int received = ReceiveFrom(buffer, sizeof(buffer), outIp, outPort);

	if (received <= 0) return false;
	if (received < (int)sizeof(PacketHeader)) return false;
	std::memcpy(&outHeader, buffer, sizeof(PacketHeader));

	if (outHeader.magic != PacketMagicCode) return false;
	if (outHeader.payloadSize > maxPayloadSize) return false;
	if (received < (int)(sizeof(PacketHeader) + outHeader.payloadSize)) return false;
	if (outHeader.payloadSize > 0 && outPayload)
	{
		std::memcpy(outPayload, buffer + sizeof(PacketHeader), outHeader.payloadSize);
	}

	UpdateRemoteSequence(outHeader.sequence);

	m_isConnected = true;
	return true;
}

void NetConnection::CloseSocket()
{
	if (m_socket != INVALID_SOCKET_HANDLE)
	{
#ifdef _WIN32
		closesocket(m_socket);
#else
		close(m_socket);
#endif
		m_socket = INVALID_SOCKET_HANDLE;
	}
}

void NetConnection::UpdateRemoteSequence(uint16 sequence)
{
	uint16 diff = static_cast<uint16_t>(sequence - m_remoteSequence);

	if (diff == 0) return; 
	if (diff < 0x8000)
	{
		if (diff < 32)
		{
			m_remoteAckBits <<= diff;
			m_remoteAckBits |= 1;
		}
		else
		{
			m_remoteAckBits = 0;
		}

		m_remoteSequence = sequence;
	}
	else
	{
		uint16 behind = static_cast<uint16>(m_remoteSequence - sequence);
		if (behind >= 1 && behind <= 32) m_remoteAckBits |= (1u << (behind - 1));
	}
}

bool NetConnection::IsSequenceNewer(uint16_t s1, uint16_t s2) const
{
	return static_cast<uint16_t>(s1 - s2) < 0x8000;
}

void NetConnection::Destroy()
{
	CloseSocket();
#ifdef _WIN32
	WSACleanup();
#endif
	m_isConnected = false;
}

