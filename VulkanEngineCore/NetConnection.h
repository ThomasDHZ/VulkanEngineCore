#pragma once
#include "Platform.h"

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
	using SocketHandle = SOCKET;
	constexpr SocketHandle INVALID_SOCKET_HANDLE = INVALID_SOCKET;
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>
	using SocketHandle = int;
	constexpr SocketHandle INVALID_SOCKET_HANDLE = -1;
#endif

	constexpr uint32 PacketMagicCode = 0x474E4554;

#pragma pack(push, 1)
struct PacketHeader
{
	uint32 magic = PacketMagicCode;
	uint16 sequence = 0;
	uint16 ack = 0;
	uint32 ackBits = 0;
	uint8  channel = 0;
	uint8  type = 0;
	uint16 payloadSize = 0;
};
#pragma pack(pop)

class NetConnection
{
private:

	SocketHandle m_socket = INVALID_SOCKET_HANDLE;
	uint16       m_localSequence = 0;
	uint16       m_remoteSequence = 0;
	uint32       m_remoteAckBits = 0;
	bool         m_isServer = false;
	bool         m_isConnected = false;

	bool CreateUDP();
	bool SetNonBlocking();
	bool Bind(uint16 port);
	int  SendTo(const void* data, int len, IpAddress& ip, uint16 port);
	int  ReceiveFrom(void* buffer, int maxLen, IpAddress& outIp, uint16* outPort);
	void CloseSocket();
	void UpdateRemoteSequence(uint16 sequence);
	bool IsSequenceNewer(uint16_t s1, uint16_t s2) const;
public:

	NetConnection();
	~NetConnection();

	bool StartUp(bool isServer, uint16 port = 0);
	bool Send(uint8 channel, uint8 type, const void* payload, uint16 payloadSize, IpAddress& ip, uint16 port);
	bool Receive(PacketHeader& outHeader, void* outPayload, uint16 maxPayloadSize, IpAddress&  outIp, uint16* outPort);
	void Destroy();

	bool IsConnected() const { return m_isConnected; }
};

