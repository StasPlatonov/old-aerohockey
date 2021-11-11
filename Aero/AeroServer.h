#ifndef GSERVER_H
#define GSERVER_H

#include "Network/Server.h"
#include "AeroTypes.h"
#include "AeroPlayer.h"
#include "AeroPhysWorld.h"
#include "Common/Mutex.h"
#include "Common/Thread.h"

namespace Aero
{
  class AeroServer : public Network::INetServerHandler, public Common::ThreadObject
  {
  public:
    AeroServer(unsigned short port, const std::string& password, Network::INetPeerFactory& factory);
    ~AeroServer();

    //INetServerHandler
    virtual void OnClientConnected(const Network::NetClientInfo& info);
    virtual void OnClientDisconnected(const Network::NetClientInfo& info);
    virtual void OnData(Network::NetId id, unsigned char* data, int size, Network::NetTime time);

    void Resize(int w, int h);
    void OnRenderTimer(int);
    void RenderFrame();

    void Run();

    virtual void OnRunThread();

  private:
    Player::Ptr CreatePlayer(const Network::NetClientInfo& info);
    void DestroyPlayer(Network::NetId id);
    Aero::Player::Ptr FindPlayer(Network::NetId id);

    void Synchronize();

    void OnClientChanged(Network::NetId id, float x, float y);
    void OnClientForce(Network::NetId id, float x, float y);

    void SyncLoop();

    void UpdatePhysics();
    void Update();
    void ProcessInputEvent(Network::NetId id, const InputEvent& ie);
    
    struct ClientMessage
    {
      ClientMessage() : Id(0), Time(0) {}
      ClientMessage(unsigned char id, Network::NetTime time, const unsigned char* data, int dataSize);
      unsigned char Id;
      Network::NetTime Time;
      Bytes Data;
    };

    void ProcessNetMessage(Network::NetId clientId, const NetMessage& msg);

  private:
    std::auto_ptr<Network::INetServer> m_Server;

    Players m_Players;

    IRender::Ptr m_Render;
    AeroPhysWorld::Ptr m_Phys;

    unsigned int m_Time;
    
    Bullets m_Bullets;

    typedef std::map<Network::NetId, NetMessages> ClientsMessages;
    ClientsMessages m_ClientsMessages;
    Common::Mutex CMGuard;
  };

  std::auto_ptr<Aero::AeroServer> CreateAeroServer(unsigned short port, const std::string& password, Network::INetPeerFactory& factory);
}

#endif
