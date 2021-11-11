#ifndef CLIENT_H
#define CLIENT_H

#include "INetwork.h"

#if defined(WIN32) || defined(WINNT)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "Common/Thread.h"

namespace Network
{
  class Client : public INetClient, public INetPeerHandler, public Common::ThreadObject
  {
  public:
    Client(INetPeerFactory& factory, INetClientHandler& h);
    ~Client();

    //INetPeerHandler
    virtual void OnAccepted(NetId id);
    virtual void OnConnectionAttemptFailed(NetId id);
    virtual void OnNoFreeConnections(NetId id);
    virtual void OnDisconnected(NetId id);
    virtual void OnConnectionLost(NetId id);
    virtual void OnNewIncomingConnection(NetId id) {};
    virtual void OnData(NetId id, unsigned char* data, int size, Network::NetTime time);

    //INetClient
    virtual void Startup(const NetClientInfo& info);
    virtual void Shutdown();
    virtual void Connect(const char* host, unsigned short port, const std::string& password);
    virtual void Disconnect();
    virtual void SendStringMessage(const std::string& str);
    virtual void PrintStat();
    virtual void Send(std::vector<unsigned char>& bytes, bool timed = false);
    virtual const NetClientInfo& GetInfo() const { return m_Info; }
    virtual int GetAvgPing() const;

    virtual void OnRunThread();

  private:
    bool IsExitRequired() const;
    void EventLoop();
    void Update();

  private:
    INetPeerFactory& m_Factory;
    INetClientHandler& m_Handler;
    bool isConnected;
    INetPeer::Ptr peer;
    unsigned int nextSendTime;
    NetId m_ServerId;
    NetClientInfo m_Info;
  };
}

#endif
