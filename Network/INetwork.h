#ifndef I_NETWORK_H
#define I_NETWORK_H

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

namespace Network
{
  typedef unsigned long long int uint64_t;

  typedef uint64_t NetId;
  const NetId EmptyNetId((uint64_t)-1);

  typedef uint64_t NetTime;
  //-----------------------------------------------------------------------------------

  struct NetClientInfo
  {
    NetClientInfo() {}

    explicit NetClientInfo(const std::string& name)
      : m_Name(name)
      , m_Id(EmptyNetId)
    {}

    explicit NetClientInfo(const NetId& id)
      : m_Id(id)
    {}

    NetClientInfo(const NetId& id, const std::string& name)
      : m_Id(id), m_Name(name) 
    {}

    NetId m_Id;
    std::string m_Name;
  };
  typedef std::map<NetId, NetClientInfo> ClientInfos;
  //-----------------------------------------------------------------------------------

  struct NetServerInfo
  {
    NetServerInfo() : m_MaxConnections(0), m_Protected(false) {}
    NetServerInfo(const std::string& name, unsigned short maxConn, bool prot)
      : m_Name(name)
      , m_MaxConnections(maxConn)
      , m_Protected(prot)
    {}

    std::string m_Name;
    unsigned short m_MaxConnections;
    bool m_Protected;
  };
  //-----------------------------------------------------------------------------------

  struct PeerParameters
  {
    PeerParameters()
      : m_Port(0)
      , m_MaxConnections(1)
    {
    }
    unsigned short m_Port;
  	char m_HostAddress[32];
    unsigned int m_MaxConnections;
  };
  //-----------------------------------------------------------------------------------
  
  class INetPeer
  {
  public:
    typedef INetPeer* Ptr;
    virtual ~INetPeer() {}

    virtual void Startup(const PeerParameters& params) = 0;
    virtual void Shutdown() = 0;

    virtual NetId GetId() const = 0;

    virtual bool Connect(const std::string& host, unsigned short port, const std::string& password) = 0;
    virtual void Disconnect() = 0;

    virtual void Send(const unsigned char* data, long size, NetId id, bool broadcast, bool timed = false) = 0;
    
    virtual void Update() = 0;

    virtual unsigned int GetConnectionsCount() const = 0;

    virtual void SetMaximumIncomingConnections(unsigned int count) = 0;
    virtual void SetIncomingPassword(const std::string& password) = 0;

    virtual std::string GetStatistics() const = 0;

    virtual void Kick(NetId id) = 0;
    virtual void Ban(const std::string& ip, unsigned int ms) = 0;
    virtual void Ping(NetId id) = 0;

    virtual int GetPing(NetId id) const = 0;
  };
  //-----------------------------------------------------------------------------------

  class INetPeerHandler
  {
  public:
    virtual ~INetPeerHandler() {}

    virtual void OnAccepted(NetId id) = 0;
    virtual void OnConnectionAttemptFailed(NetId id) = 0;
    virtual void OnNoFreeConnections(NetId id) = 0;
    virtual void OnDisconnected(NetId id) = 0;
    virtual void OnConnectionLost(NetId id) = 0;
    virtual void OnNewIncomingConnection(NetId id) = 0;
    virtual void OnData(NetId id, unsigned char* data, int size, Network::NetTime time = 0) = 0;
  };
  //-----------------------------------------------------------------------------------

  class INetPeerFactory
  {
  public:
    virtual ~INetPeerFactory() {}
    virtual INetPeer::Ptr CreatePeer(INetPeerHandler& handler) = 0;
    virtual void DestroyPeer(INetPeer::Ptr peer) = 0;
  };
  //-----------------------------------------------------------------------------------

  class INetClientHandler
  {
  public:
    virtual ~INetClientHandler() {}
    virtual void OnConnected(const NetServerInfo& info) = 0;
    virtual void OnDisconnected() = 0;
    virtual void OnData(unsigned char* data, int size, Network::NetTime time) = 0;
  };

  class INetClient
  {
  public:
    virtual ~INetClient() {}
    virtual void Startup(const NetClientInfo& info) = 0;
    virtual void Shutdown() = 0;
    virtual void Connect(const char* host, unsigned short port, const std::string& password = std::string()) = 0;
    virtual void Disconnect() = 0;
    virtual void SendStringMessage(const std::string& str) = 0;
    virtual void PrintStat() = 0;
    virtual void Send(std::vector<unsigned char>& bytes, bool timed = false) = 0;
    virtual const NetClientInfo& GetInfo() const = 0;
    virtual int GetAvgPing() const = 0;
  };

  std::auto_ptr<INetClient> CreateNetClient(INetPeerFactory& factory, INetClientHandler& h);
  //-----------------------------------------------------------------------------------

  class INetServerHandler
  {
  public:
    virtual ~INetServerHandler() {}
    virtual void OnClientConnected(const Network::NetClientInfo& info) = 0;
    virtual void OnClientDisconnected(const Network::NetClientInfo& info) = 0;
    virtual void OnData(Network::NetId id, unsigned char* data, int size, Network::NetTime time) = 0;
  };

  class INetServer : public INetPeerHandler
  {
  public:
    virtual ~INetServer() {}
    virtual void Start(unsigned short port, const NetServerInfo& info, const std::string& password = std::string()) = 0;
    virtual void Shutdown() = 0;
    virtual unsigned ConnectionCount() const = 0;
    virtual void SendStringMessage(const std::string& name, const std::string& str) = 0;
    virtual void Kick(const std::string& name) = 0;
    virtual void Ban(const std::string& ip, unsigned int ms) = 0;
    virtual void PrintStat() = 0;

    virtual void Send(NetId id, std::vector<unsigned char>& bytes, bool timed = false) = 0;
    virtual NetId FindClient(const std::string& name) const = 0;
    
    virtual unsigned int GetClientsCount() const = 0;
    virtual const ClientInfos& GetClients() const = 0;
    virtual int GetAvgPing(NetId client) const = 0;
  };

  std::auto_ptr<INetServer> CreateNetServer(INetPeerFactory& factory, INetServerHandler& h);
  //-----------------------------------------------------------------------------------
}

#endif