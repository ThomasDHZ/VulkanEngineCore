#include "NetworkSystem.h"
#include "ChatSystem.h"
#include <cstdint>
#include <cstring>
#include <chrono>
#include <ctime>

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
		bool found = false;
		if (m_mode == NetworkMode::Server)
		{
			for (auto& client : m_clients)
			{
				if (strcmp(client.ip.data(), ipAddress.data()) == 0 &&
					client.port == port)
				{
					client.lastSeenTime = 0;
					found = true;
					break;
				}
			}
			if (!found)
			{
				m_clients.emplace_back(ClientInfo
					{
						.ip = ipAddress,
						.port = port
					});
				printf("New client connected: %s:%u\n", ipAddress.data(), port);
			}
		}
		OnPacketReceived(header, payload, sizeof(payload), ipAddress, port);
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

bool NetworkSystem::SendUnreliable(uint8 type, const void* data, uint16 size)
{
	// For now just use channel 1 (we still need to implement actual reliability)
	if (m_mode == NetworkMode::Client) return m_connection.Send(1, type, data, size, m_serverIP, m_serverPort);
	return false;
}

bool NetworkSystem::SendReliable(uint8 type, const void* data, uint16 size)
{
	if (m_mode == NetworkMode::Client) return m_connection.Send(0, type, data, size, m_serverIP, m_serverPort);
	// Server would need to send to specific clients (we’ll add client list later)
	return false;
}

bool NetworkSystem::BroadcastUnreliable(uint8 type, const void* data, uint16 size, const IpAddress* excludeIp, uint16 excludePort)
{
	if (m_mode != NetworkMode::Server) return false;

	bool success = true;
	for (auto& client : m_clients)
	{
		if (excludeIp != nullptr &&
			strcmp(client.ip.data(), excludeIp->data()) == 0 &&
			client.port == excludePort)
		{
			continue;
		}

		if (!m_connection.Send(0, type, data, size, client.ip, client.port))
		{
			success = false;
		}
	}

	return success;
}

bool NetworkSystem::BroadcastReliable(uint8 type, const void* data, uint16 size)
{
	return false;
}

void NetworkSystem::SetNetworkMode(NetworkMode networkMode)
{
}

void NetworkSystem::Shutdown()
{
	m_connection.Destroy();
	m_mode = NetworkMode::None;
}

void NetworkSystem::SendChatMessage(const String& text)
{
	ChatMessagePacket packet{};
	strncpy(packet.text, text.c_str(), sizeof(packet.text) - 1);
	packet.text[sizeof(packet.text) - 1] = '\0';

	if (IsClient()) SendUnreliable(static_cast<uint8>(PacketType::ChatMessage), &packet, sizeof(packet));
	else if (IsServer()) BroadcastUnreliable(static_cast<uint8>(PacketType::ChatMessage), &packet, sizeof(packet));
}

void NetworkSystem::OnPacketReceived(const PacketHeader& header, const byte* data, uint16 size, IpAddress ip, uint16 port)
{
	switch (static_cast<PacketType>(header.type))
	{
	case PacketType::ChatMessage:
	{
		if (size < sizeof(ChatMessagePacket)) return;
		const ChatMessagePacket* chat = reinterpret_cast<const ChatMessagePacket*>(data);

		char buffer[128]{};
		memcpy(buffer, chat->text, sizeof(buffer) - 1);
		buffer[127] = '\0';

		if(m_serverIP != ip) chatSystem.AddChatMessage(ip.data(), buffer);
		if (IsServer()) BroadcastUnreliable(static_cast<uint8>(PacketType::ChatMessage), data, size, &ip, port);
		break;
	}
	case PacketType::Ping:
		// Optional: reply with Pong later
		break;
	default: break;
	}
}

bool					  NetworkSystem::IsServer()		  const		{ return m_mode == NetworkMode::Server; }
bool					  NetworkSystem::IsClient()		  const		{ return m_mode == NetworkMode::Client; }
bool					  NetworkSystem::IsConnected()	  const		{ return m_connection.IsConnected(); }
NetworkMode				  NetworkSystem::GetNetworkMode() const     { return m_mode; }
const Vector<ClientInfo>& NetworkSystem::GetClients()	  const		{ return m_clients; }

