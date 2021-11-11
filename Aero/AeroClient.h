#ifndef AERO_CLIENT_H
#define AERO_CLIENT_H

#include "Network/Client.h"
#include "AeroTypes.h"
#include "AeroPlayer.h"
#include "AeroPhysWorld.h"

#include "Common/Mutex.h"

#include <boost/shared_ptr.hpp>

namespace Aero
{
  class IAeroClient
  {
  public:
    typedef boost::shared_ptr<IAeroClient> Ptr;

    virtual ~IAeroClient() {}
    virtual void Init(const std::string& name, IRender::Ptr render) = 0;
    virtual void Done() = 0;
    virtual void Connect(const char* host, unsigned short port, const std::string& password = std::string()) = 0;

    virtual const Network::NetClientInfo& GetInfo() const = 0;

    virtual void Pause() = 0;
    virtual void Click(int x, int y) = 0;

    virtual void Run() = 0;

    virtual void UpdatePlayers() = 0;
    virtual void UpdatePhysics() = 0;

    virtual void DebugDraw(IRender::Ptr render) = 0;

    virtual void Keyboard(int key, bool pressed) = 0;

    virtual void Shoot(int x, int y) = 0;
  };
  //-----------------------------------------------------------------------------------

  class IAeroClientHandler
  {
  public:
    virtual ~IAeroClientHandler() {}
    virtual void OnMessage(const std::string& text) = 0;
    virtual bool IsActive() const = 0;
    virtual void OnUpdate() = 0;
  };
  //-----------------------------------------------------------------------------------

  class AeroClient : public virtual IAeroClient, public virtual Network::INetClientHandler
  {
  public:
    AeroClient(Network::INetPeerFactory& factory, IAeroClientHandler& h);
    virtual ~AeroClient();

    //IAeroClient
    virtual void Init(const std::string& name, IRender::Ptr render);
    virtual void Done();
    virtual void Connect(const char* host, unsigned short port, const std::string& password = std::string());
    virtual const Network::NetClientInfo& GetInfo() const;
    
    virtual void Pause();
    virtual void Click(int x, int y);

    virtual void Run();

    virtual void UpdatePlayers();
    virtual void UpdatePhysics();
    virtual void DebugDraw(IRender::Ptr render);

    virtual void Keyboard(int key, bool pressed);
    virtual void Shoot(int x, int y);

    //INetClientHandler
    virtual void OnConnected(const Network::NetServerInfo& info);
    virtual void OnDisconnected();
    virtual void OnData(unsigned char* data, int size, Network::NetTime time);

  private:
    void ClientConnected(Network::NetId id, const std::string& name, float x, float y);
    void ClientDisconnected(Network::NetId id);
    void ClientChanged(Network::NetId id, float x, float y);
    void ClientForce(Network::NetId id, float x, float y);
    void Synchronize(const Aero::DynamicClientDatas& data);
    void SendInputEvent();
    void SendChanged(float x, float y);
    void SendShoot(float x, float y);

    Aero::Player::Ptr CreatePlayer(const Network::NetClientInfo& info);
    void DestroyPlayer(Network::NetId id);
    Aero::Player::Ptr FindPlayer(Network::NetId id);

    void AddMoveToHistory(const PlayerMove& move);
    void HistoryCorrection(const SyncEvent& se);

    void Update();

    void ProcessSyncEvent(Network::NetId clientId, const SyncEvent& se);
    void ProcessNetMessage(const NetMessage& msg);

  private:
    IAeroClientHandler& m_Handler;
    std::auto_ptr<Network::INetClient> m_Impl;
    std::string m_Current;

    AeroPhysWorld::Ptr m_Phys;

    Player::Ptr m_LocalPlayer;
    Players m_Players;

    InputState m_Input;

    Moves m_Moves;
    Moves m_ImportantMoves;

    Network::NetTime m_Time;

    Bullets m_Bullets;
    
    
    NetMessages m_ServerMessages;
    Common::Mutex SMGuard;
  };
  //-----------------------------------------------------------------------------------
  
  std::auto_ptr<Aero::IAeroClient> CreateAeroClient(Network::INetPeerFactory& factory, IAeroClientHandler& h);
}

#endif
