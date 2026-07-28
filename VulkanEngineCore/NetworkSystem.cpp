#include "NetworkSystem.h"
#include <cstdint>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <errno.h>
#endif

NetworkSystem& networkSystem = NetworkSystem::Get();

bool NetworkSystem::StartAsServer(uint16 port)
{
	if (!m_peer.StartUp(true, port)) return false;
	m_mode = NetworkMode::Server;
	return true;
}

bool NetworkSystem::StartAsClient()
{
	if (!m_peer.StartUp(false)) return false;
	m_mode = NetworkMode::Client;
	return true;
}

bool NetworkSystem::Connect(const IpAddress& ip, uint16 port)
{
	if (m_mode != NetworkMode::Client) return false;
	m_serverIP = ip;
	m_serverPort = port;
	return true;
}

void NetworkSystem::Disconnect()
{
}

void NetworkSystem::ProcessIncoming()
{
	PacketHeader header;
	uint8		 payload[1200];
	IpAddress	 ipAddress;
	uint16		 port;

	while (m_peer.Receive(header, payload, sizeof(payload), ipAddress, &port))
	{
		// Optional: update RTT if this is an ack-heavy packet
		if (m_packetCallback) m_packetCallback(header, payload, header.payloadSize, ip, port);
	}
}

void NetworkSystem::ProcessOutgoing(float deltaTime)
{
}

void NetworkSystem::Update(float deltaTime) 
{
	if (m_mode == NetworkMode::None) return;

	ProcessIncoming();
	ProcessOutgoing(deltaTime);
}

bool NetworkSystem::SendReliable(uint8 type, const void* data, uint16 size)
{
	if (m_mode == NetworkMode::Client) return m_peer.Send(0, type, data, size, m_serverIP, m_serverPort);
	// Server would need to send to specific clients (we’ll add client list later)
	return false;
}

bool NetworkSystem::SendUnreliable(uint8 type, const void* data, uint16 size)
{
	// For now just use channel 1 (we still need to implement actual reliability)
	if (m_mode == NetworkMode::Client) return m_peer.Send(1, type, data, size, m_serverIP, m_serverPort);
	return false;
}


void NetworkSystem::Shutdown()
{
	m_peer.Destroy();
	m_mode = NetworkMode::None;
}