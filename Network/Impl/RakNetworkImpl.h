#ifndef RAK_NETWORK_H
#define RAK_NETWORK_H

#include "../INetwork.h"

#include "RakPeerInterface.h"

namespace Network
{
  using namespace RakNet;

  class RakNetPeer : public INetPeer
  {
  public:
    explicit RakNetPeer(INetPeerHandler& handler);
    ~RakNetPeer();

    virtual unsigned int GetConnectionsCount() const;
    virtual void SetMaximumIncomingConnections(unsigned int count);
    virtual void SetIncomingPassword(const std::string& password);
    virtual void Startup(const PeerParameters& params);
    virtual void Shutdown();
    virtual bool Connect(const std::string& host, unsigned short port, const std::string& password);
    virtual void Disconnect();
    virtual void Send(const unsigned char* data, long size, NetId id, bool broadcast, bool timed = false);
    virtual void Kick(NetId id);
    virtual void Ban(const std::string& ip, unsigned int ms);
    virtual void Update();
    virtual void Ping(NetId id);
    virtual std::string GetStatistics() const;
    virtual int GetPing(NetId id) const;

    virtual NetId GetId() const;

  private:
    void ProcessPacketInternal(Packet* p);

  private:
    RakNet::RakPeerInterface* m_Peer;
    INetPeerHandler& m_Handler;
  };
  //-----------------------------------------------------------------------------------

  class RakNetPeerFactory : public INetPeerFactory
  {
  public:
    virtual INetPeer::Ptr CreatePeer(INetPeerHandler& handler);
    virtual void DestroyPeer(INetPeer::Ptr peer);
  };

  INetPeerFactory& GetRakNetPeerFactory();
  //-----------------------------------------------------------------------------------
}

#endif
