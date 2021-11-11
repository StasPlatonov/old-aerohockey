#ifndef NETSERVER_H
#define NETSERVER_H

#include "INetwork.h"
#include <map>

#if defined(WIN32) || defined(WINNT)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "Common/Thread.h"

namespace Network
{
  class Server : public INetServer, public Common::ThreadObject
  {
  public:
    Server(INetPeerFactory& factory, INetServerHandler& handler);
    ~Server();
    virtual void Start(unsigned short port, const NetServerInfo& info, const std::string& password);
    virtual void Shutdown();
    virtual unsigned ConnectionCount() const;
    virtual void SendStringMessage(const std::string& name, const std::string& str);
    virtual void Kick(const std::string& name);
    virtual void Ban(const std::string& ip, unsigned int ms);
    virtual void PrintStat();

    virtual void OnAccepted(NetId id) {};
    virtual void OnConnectionAttemptFailed(NetId id) {};
    virtual void OnNoFreeConnections(NetId id) {};
    virtual void OnDisconnected(NetId id);
    virtual void OnConnectionLost(NetId id);
    virtual void OnNewIncomingConnection(NetId id);
    virtual void OnData(NetId id, unsigned char* data, int size, Network::NetTime time);

    virtual void Send(NetId id, std::vector<unsigned char>& bytes, bool timed = false);
    virtual NetId FindClient(const std::string& name) const;
    
    virtual unsigned int GetClientsCount() const { return m_Clients.size(); }
    virtual const ClientInfos& GetClients() const { return m_Clients; }
    virtual int GetAvgPing(NetId client) const;

    virtual void OnRunThread();

  private:
    bool IsExitRequired() const;
    void EventLoop();
    void Update();

  private:
    unsigned int nextSendTime;
    INetPeer::Ptr peer;
    INetPeerFactory& m_Factory;
    INetServerHandler& m_Handler;
    unsigned int LastSendTime;
    NetServerInfo m_Info;
    typedef std::map<std::string, NetId> ClientNames;

  protected:
    ClientInfos m_Clients;
    ClientNames m_ClientNames;
  };
}

#endif
