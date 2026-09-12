#pragma once
#include "DLL.h"
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

class NetworkSystem
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
    CORE_DLL_EXPORT  bool                                    StartAsServer(uint16 port);
    CORE_DLL_EXPORT  bool                                    StartAsClient();
    CORE_DLL_EXPORT bool                                    ConnectToServer(const IpAddress& ip, uint16 port);
    CORE_DLL_EXPORT void                                    Update(float deltaTime);
    CORE_DLL_EXPORT  void                                    Stop();
    CORE_DLL_EXPORT  void                                    Shutdown();

    CORE_DLL_EXPORT  void                                    SendChatMessage(const String& text);
    CORE_DLL_EXPORT  void                                    OnPacketReceived(const PacketHeader& header, const byte* data, uint16 size, IpAddress ip, uint16 port);

    CORE_DLL_EXPORT void                                    SetNetworkMode(NetworkMode networkMode);
    CORE_DLL_EXPORT bool                                    SendUnreliable(uint8 type, const void* data, uint16 size);
    CORE_DLL_EXPORT bool                                    SendReliable(uint8 type, const void* data, uint16 size);
    CORE_DLL_EXPORT  bool                                    BroadcastUnreliable(uint8 type, const void* data, uint16 size, const IpAddress* excludeIp = nullptr, uint16 excludePort = 0);
    CORE_DLL_EXPORT  bool                                    BroadcastReliable(uint8 type, const void* data, uint16 size);

    CORE_DLL_EXPORT [[nodiscard]] bool                      IsServer()       const;
    CORE_DLL_EXPORT [[nodiscard]] bool                      IsClient()       const;
    CORE_DLL_EXPORT [[nodiscard]] bool                      IsConnected()    const;
    CORE_DLL_EXPORT [[nodiscard]] NetworkMode               GetNetworkMode() const;
    CORE_DLL_EXPORT [[nodiscard]] const Vector<ClientInfo>& GetClients()     const;
};
CORE_DLL_EXPORT extern NetworkSystem& networkSystem;
inline NetworkSystem& NetworkSystem::Get()
{
    static NetworkSystem instance;
    return instance;
}