#pragma once
#include "platform.h"
#include "NetConnection.h"
class NetworkSystem
{
public:
    static NetworkSystem& Get();
    enum class NetworkMode 
    {
        None,
        Server,
        Client
    };
    using PacketCallback = std::function<void(const PacketHeader&, const uint8* payload, uint16 size, const char* ip, uint16 port)>;

private:
    NetworkSystem() = default;
    ~NetworkSystem() = default;
    NetworkSystem(const NetworkSystem&) = delete;
    NetworkSystem& operator=(const NetworkSystem&) = delete;
    NetworkSystem(NetworkSystem&&) = delete;
    NetworkSystem& operator=(NetworkSystem&&) = delete;

    NetworkMode    m_mode = NetworkMode::None;
    NetConnection  m_peer;
    String         m_serverIP;
    uint16         m_serverPort = 0;
    PacketCallback m_packetCallback;

    void ProcessIncoming();
    void ProcessOutgoing(float deltaTime);

public:
    bool StartAsServer(uint16 port);
    bool StartAsClient();
    bool Connect(const char* ip, uint16 port);
    void Update(float deltaTime);
    void Shutdown();

    bool SendUnreliable(uint8 type, const void* data, uint16 size);
    bool SendReliable(uint8 type, const void* data, uint16 size);

    void SetPacketCallback(PacketCallback cb);

    bool IsServer()    const { return m_mode == NetworkMode::Server; }
    bool IsClient()    const { return m_mode == NetworkMode::Client; }
    bool IsConnected() const { return m_peer.IsConnected(); }
};
extern DLL_EXPORT NetworkSystem& networkSystem;
inline NetworkSystem& NetworkSystem::Get()
{
    static NetworkSystem instance;
    return instance;
}
