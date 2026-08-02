#pragma once
#include "platform.h"
#include "NetConnection.h"
    enum class NetworkMode 
    {
        None,
        Server,
        Client
    };

    enum class PacketType : byte
    {
        Ping        = 0x0,
        Pong        = 0x1,
        ChatMessage = 0x2,
        // add more later
    };

    struct ClientInfo
    {
        IpAddress ip{};
        uint16    port = 0;
        double    lastSeenTime = 0.0;   // useful later for timeouts
    };

#pragma pack(push, 1)
    struct ChatMessagePacket
    {
        char text[128];          // null-terminated
    };
#pragma pack(pop)

class DLL_EXPORT NetworkSystem
{
public:
    static NetworkSystem& Get();
    using PacketCallback = std::function<void(const PacketHeader&, const uint8* payload, uint16 size, const IpAddress& ip, uint16 port)>;

private:
    NetworkSystem() = default;
    ~NetworkSystem() = default;
    NetworkSystem(const NetworkSystem&) = delete;
    NetworkSystem& operator=(const NetworkSystem&) = delete;
    NetworkSystem(NetworkSystem&&) = delete;
    NetworkSystem& operator=(NetworkSystem&&) = delete;

    NetworkMode                             m_mode = NetworkMode::None;
    NetConnection                           m_connection;
    IpAddress                               m_serverIP;
    uint16                                  m_serverPort = 0;
    PacketCallback                          m_packetHandler;
    Vector<ClientInfo>                      m_clients;

    void                                    ProcessIncoming();
    void                                    ProcessOutgoing(float deltaTime);

public:
    bool                                    StartAsServer(uint16 port);
    bool                                    StartAsClient();
    bool                                    ConnectToServer(const IpAddress& ip, uint16 port);
    void                                    Update(float deltaTime);
    void                                    Stop();
    void                                    Shutdown();

    void                                    SendChatMessage(const String& text);
    void                                    OnPacketReceived(const PacketHeader& header, const byte* data, uint16 size, IpAddress ip, uint16 port);

    void                                    SetNetworkMode(NetworkMode networkMode);
    bool                                    SendUnreliable(uint8 type, const void* data, uint16 size);
    bool                                    SendReliable(uint8 type, const void* data, uint16 size);
    bool                                    BroadcastUnreliable(uint8 type, const void* data, uint16 size, const IpAddress* excludeIp = nullptr, uint16 excludePort = 0);
    bool                                    BroadcastReliable(uint8 type, const void* data, uint16 size);

    [[nodiscard]] bool                      IsServer()       const;
    [[nodiscard]] bool                      IsClient()       const;
    [[nodiscard]] bool                      IsConnected()    const;
    [[nodiscard]] NetworkMode               GetNetworkMode() const;
    [[nodiscard]] const Vector<ClientInfo>& GetClients()     const;
};
extern DLL_EXPORT NetworkSystem& networkSystem;
inline NetworkSystem& NetworkSystem::Get()
{
    static NetworkSystem instance;
    return instance;
}