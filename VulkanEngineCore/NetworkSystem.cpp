#include "NetworkSystem.h"
#include <cstdint>
#include <cstring>

NetworkSystem& networkSystem = NetworkSystem::Get();

bool NetworkSystem::StartAsServer(uint16 port)
{
	Stop();
	if (!m_connection.StartUp(true, port)) return false;
	m_mode = NetworkMode::Server;
	return true;
}

bool NetworkSystem::StartAsClient()
{
	Stop();
	if (!m_connection.StartUp(false)) return false;
	m_mode = NetworkMode::Client;
	return true;
}

bool NetworkSystem::ConnectToServer(const IpAddress& ip, uint16 port)
{
	if (m_mode != NetworkMode::Client) return false;
	m_serverIP = ip;
	m_serverPort = port;
	return true;
}

void NetworkSystem::ProcessIncoming()
{
	PacketHeader header;
	byte		 payload[1200];
	IpAddress	 ipAddress;
	uint16		 port;

	while (m_connection.Receive(header, payload, sizeof(payload), ipAddress, &port))
	{
		if (m_packetHandler) m_packetHandler(header, payload, header.payloadSize, ipAddress, port);
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

void NetworkSystem::Stop()
{
	m_connection.Destroy();
	m_mode = NetworkMode::None;
	m_serverIP = IpAddress();
	m_serverPort = 0;
}

bool NetworkSystem::SendReliable(uint8 type, const void* data, uint16 size)
{
	if (m_mode == NetworkMode::Client) return m_connection.Send(0, type, data, size, m_serverIP, m_serverPort);
	// Server would need to send to specific clients (we’ll add client list later)
	return false;
}

void NetworkSystem::SetNetworkMode(NetworkMode networkMode)
{
}

bool NetworkSystem::SendUnreliable(uint8 type, const void* data, uint16 size)
{
	// For now just use channel 1 (we still need to implement actual reliability)
	if (m_mode == NetworkMode::Client) return m_connection.Send(1, type, data, size, m_serverIP, m_serverPort);
	return false;
}

void NetworkSystem::Shutdown()
{
	m_connection.Destroy();
	m_mode = NetworkMode::None;
}

void NetworkSystem::SendChatMessage(const String& text)
{
	if (!IsClient() || !IsConnected()) return;
	SendUnreliable(static_cast<uint8_t>(PacketType::ChatMessage), text.data(), static_cast<uint16>(text.size()));
}

void NetworkSystem::OnPacketReceived(const PacketHeader& header, const byte* data, uint16 size, IpAddress ip, uint16 port)
{
	switch (static_cast<PacketType>(header.type))
	{
		case PacketType::ChatMessage:
		{
			char buffer[256]{};
			uint16_t copySize = std::min(size, (uint16_t)(sizeof(buffer) - 1));
			memcpy(buffer, data, copySize);

			printf("Chat from %s: %s\n", ip, buffer);
			break;
		}
		case PacketType::Ping: break;
	}
}

bool		NetworkSystem::IsServer() const			{ return m_mode == NetworkMode::Server; }
bool		NetworkSystem::IsClient() const			{ return m_mode == NetworkMode::Client; }
bool		NetworkSystem::IsConnected() const		{ return m_connection.IsConnected(); }
NetworkMode NetworkSystem::GetNetworkMode() const   { return m_mode; }
