#include "AeroClient.h"

#include "Common/Serialization.h"
#include "Common/Timer.h"
#include "Common/Utils.h"

#include <Box2D/Box2D.h>

#include <stdlib.h>
#if defined(WIN32) || defined(WINNT)
#else
#include <stdio.h>
#endif

namespace Aero
{
  using namespace Network;
  using namespace Common;

  namespace
  {
    const float CL_TIME_STEP = 0.01f;
    Settings settings;
    const int MAX_CLIENT_HISTORY = 1000;
  }
  //-----------------------------------------------------------------------------------

  AeroClient::AeroClient(INetPeerFactory& factory, IAeroClientHandler& h)
    : m_Impl(CreateNetClient(factory, *this))
    , m_Handler(h)
    , m_Time(0)
  {
    m_Moves.Resize(MAX_CLIENT_HISTORY + 1);
    m_ImportantMoves.Resize(MAX_CLIENT_HISTORY + 1);
  }

  AeroClient::~AeroClient()
  {
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::OnConnected(const Network::NetServerInfo& info)
  {
  }

  void AeroClient::OnDisconnected()
  {
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::ClientConnected(Network::NetId id, const std::string& name, float x, float y)
  {
    Network::NetClientInfo otherInfo(id, name);

    Aero::Player::Ptr otherPlayer = CreatePlayer(otherInfo);
    otherPlayer->SetPosition(x, y);

    if (id == GetInfo().m_Id)
    {
      m_LocalPlayer = otherPlayer; //store our player
    }

    char str[255] = {0};
    if (id == GetInfo().m_Id) //This is we are =)
    {
      sprintf(str, "You entered the game.\n");
    }
    else
    {
      sprintf(str, "Player '%s' entered the game.\n", name.c_str());
    }

    m_Handler.OnMessage(str);
  }
  //-----------------------------------------------------------------------------------
  
  void AeroClient::ClientDisconnected(Network::NetId id)
  {
    if (Aero::Player::Ptr player = FindPlayer(id))
    {
      DestroyPlayer(id);

      char str[255] = {0};
      sprintf(str, "Player '%s' leave the game.\n", player->m_NetInfo.m_Name.c_str());
      m_Handler.OnMessage(str);
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::ClientChanged(Network::NetId id, float x, float y)
  {
    if (Aero::Player::Ptr player = FindPlayer(id))
    {
      player->Impulse(x, y);
    }
  }

  void AeroClient::ClientForce(Network::NetId id, float x, float y)
  {
    if (Aero::Player::Ptr player = FindPlayer(id))
    {
      player->Force(x, y);
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::AddMoveToHistory(const PlayerMove& move)
  {
    if (!m_Moves.Empty())
    {
      const PlayerMove& prev = m_Moves.Newest();
      if (prev.m_Input != move.m_Input)
      {
        m_ImportantMoves.Add(move);
      }
    }
    
    m_Moves.Add(move);
    if (m_Moves.GetSize() >= MAX_CLIENT_HISTORY)
    {
      printf("WARNING: Maximum client history size reached.\n");
    }
  }
  //------------------------------------------------------------------------------------------

  /*!
    When server sends SyncEvent, we have to correct our stored moves
  */
  void AeroClient::HistoryCorrection(const SyncEvent& se)
  {
    if (!m_LocalPlayer.get())
      return;

    while (m_ImportantMoves.Oldest().m_Time < se.m_Time && !m_ImportantMoves.Empty())
    {
      m_ImportantMoves.Remove();
    }

    // discard out of date moves
    while (m_Moves.Oldest().m_Time < se.m_Time && !m_Moves.Empty())
    {
      m_Moves.Remove();
    }

    if (m_Moves.Empty())
      return;

    // compare correction state with move history state
    if (se.m_State != m_Moves.Oldest().m_State)
    {
      // discard corrected move
      m_Moves.Remove();

      // save current scene data
      unsigned int savedTime = m_Time;
      InputState savedInput = m_Input;

      // rewind to correction and replay moves
      //set all parameters to server's values
      m_Time = se.m_Time;
      m_Input = se.m_Input;
      m_LocalPlayer->SetState(se.m_State);//or SetDesiredState()?
      //now physics will update player with received state and input

      int i = m_Moves.m_Tail;
      while (i != m_Moves.m_Head)
      {
        //Update() is called from main client thread, so just wait for i-th move's time
        while (m_Time < m_Moves[i].m_Time)
        {   //Update();
        }
        //save current player's state that was modified by local physics simulation
        m_LocalPlayer->GetState(m_Moves[i].m_State);

        //set input (that will be active now) to i-th move's input
        m_Input = m_Moves[i].m_Input;

        m_Moves.Next(i);
      }

      //Update();

      // restore saved input
      m_Input = savedInput;
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::Synchronize(const Aero::DynamicClientDatas& data)
  {
    for (Aero::DynamicClientDatas::const_iterator it = data.begin(); it != data.end(); ++it)
    {
      const Aero::DynamicClientData& dcd = *it;
      
      if (Aero::Player::Ptr player = FindPlayer(dcd.m_Id))
      {
        player->SetDesiredState(dcd.m_X, dcd.m_Y, dcd.m_VX, dcd.m_VY);
      }
    }
    //m_Handler
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::OnData(unsigned char* data, int size, Network::NetTime time)
  {
    //! We enqueue network message for futher processing for network performance.
    //! We can also just call ProcessNetMessage() from here.
    const unsigned char pid = data[0];
    //Common::CSLocker locker(SMGuard);
    //m_ServerMessages.push_back(NetMessage(pid, time, data, size));
    ProcessNetMessage(NetMessage(pid, time, data, size));
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::ProcessNetMessage(const NetMessage& msg)
  {
    const unsigned char pid = msg.Id;
    Network::NetTime time = msg.Time;
    const unsigned char* data = &msg.Data.front();
    int size = msg.Data.size();

    switch (pid)
    {
    case MSG_CLIENT_CONNECTED:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        NetId id;
        dsr.Deserialize((unsigned long long&)id);       //ID
        std::string name;
        dsr.Deserialize(name);                          //Name
        float x, y;
        dsr.Deserialize((float&)x);                       //Position
        dsr.Deserialize((float&)y);
        
        ClientConnected(id, name, x, y);
        break;
      }

    case MSG_CLIENT_DISCONNECTED:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        NetId id;
        dsr.Deserialize((unsigned long long&)id);
        
        ClientDisconnected(id);
        break;
      }

    case MSG_CURRENT_WORLD_STATE:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        int pCount = 0;
        dsr.Deserialize((int&)pCount);

        //create player for each client that exists on server now
        for (int i = 0; i < pCount; ++i)
        {
           NetId id;
           dsr.Deserialize((unsigned long long&)id);       //ID
           std::string name;
           dsr.Deserialize(name);                          //Name
           float x, y;
           dsr.Deserialize((float&)x);                       //Position
           dsr.Deserialize((float&)y);

           ClientConnected(id, name, x, y);
        }
        break;
      }

    case MSG_SYNCHRONIZE:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        int pCount = 0;
        dsr.Deserialize(pCount);

        DynamicClientDatas dcds(pCount, DynamicClientData());
        dsr.Deserialize(&dcds.front(), pCount * sizeof(DynamicClientData));

        Synchronize(dcds);
        break;
      }

    case MSG_CLIENT_CHANGED:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        NetId id;
        dsr.Deserialize((unsigned long long&)id);
        float x,y;
        dsr.Deserialize(x);
        dsr.Deserialize(y);

        if (id == m_Impl->GetInfo().m_Id)
        {
          return;
        }
   
        ClientChanged(id, x, y);
        break;
      }

    /*case MSG_CLIENT_FORCE:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        NetId id;
        dsr.Deserialize((unsigned long long&)id);
        float x,y;
        dsr.Deserialize(x);
        dsr.Deserialize(y);

        if (id == m_Impl->GetInfo().m_Id)
        {
          return;
        }
   
        ClientForce(id, x, y);
        break;
      }*/

      case MSG_CLIENT_INPUT_CONFIRM:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        NetId id;
        dsr.Deserialize((unsigned long long&)id);
        
        SyncEvent se;
        dsr.Deserialize(&se, sizeof(SyncEvent));

        ProcessSyncEvent(id, se);
        break;
      }

      case MSG_CLIENT_SHOOT_RESP:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        NetId id;
        dsr.Deserialize((unsigned long long&)id);

        float x, y;
        dsr.Deserialize(x);
        dsr.Deserialize(y);

        float vx, vy;
        dsr.Deserialize(vx);
        dsr.Deserialize(vy);

        m_Bullets.push_back(Bullet::Ptr(new Bullet(x, y, vx, vy, m_Phys))); 
        
        break;
      }
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::ProcessSyncEvent(Network::NetId clientId, const SyncEvent& se)
  {
    if (Aero::Player::Ptr player = FindPlayer(clientId))
    {
      //correct player's physics state
      player->SetDesiredState(se.m_State.m_X, se.m_State.m_Y, se.m_State.m_VX, se.m_State.m_VY);

      if (clientId == m_Impl->GetInfo().m_Id)
      {
        HistoryCorrection(se);
      }
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::Init(const std::string& name, IRender::Ptr render)
  {
    m_Impl->Startup(Network::NetClientInfo(name));
    m_Phys = AeroPhysWorld::Ptr(AeroPhysWorld::Create(render).release());
  }

  void AeroClient::Done()
  {
    m_Impl->Disconnect();
    m_Impl->Shutdown();
  }

  void AeroClient::Connect(const char* host, unsigned short port, const std::string& password)
  {
    m_Impl->Connect(host, port, password);
  }
  //-----------------------------------------------------------------------------------

  const Network::NetClientInfo& AeroClient::GetInfo() const
  {
    return m_Impl->GetInfo();
  }
  //-----------------------------------------------------------------------------------
  
  void AeroClient::SendChanged(float x, float y)
  {
    Bytes bytes;

    BinarySerializer sr(bytes);
    sr.Serialize((char)MSG_CLIENT_CHANGED);
    sr.Serialize((float)x);
    sr.Serialize((float)y);

    m_Impl->Send(bytes, true);
  }

  /*void AeroClient::SendForce(float x, float y)
  {
    Bytes bytes;

    BinarySerializer sr(bytes);
    sr.Serialize((char)MSG_CLIENT_FORCE);
    sr.Serialize((float)x);
    sr.Serialize((float)y);

    m_Impl->Send(bytes, true);
  }*/
  //-----------------------------------------------------------------------------------

  Aero::Player::Ptr AeroClient::CreatePlayer(const Network::NetClientInfo& info)
  {
    Aero::Player::Ptr player(new Aero::Player(info, m_Phys));
    m_Players.push_back(player);
    return player;
  }
  //-----------------------------------------------------------------------------------

  Aero::Player::Ptr AeroClient::FindPlayer(Network::NetId id)
  {
    const Aero::PlayerFinder finder(id);
    Aero::Players::iterator it = std::find_if(m_Players.begin(), m_Players.end(), finder);
    
    return it == m_Players.end() ? Aero::Player::Ptr() : *it;
  }

  /*Aero::Player::Ptr AeroClient::FindPlayer(b2Body* body)
  {
    for (Aero::Players::iterator it = m_Players.begin(); it != m_Players.end(); ++it)
    {
      if ((*it)->m_Body == body)
      {
        return *it;
      }
    }

    return Aero::Player::Ptr();
  }*/
  //-----------------------------------------------------------------------------------

  void AeroClient::DestroyPlayer(Network::NetId id)
  {
    const Aero::PlayerFinder finder(id);
    Aero::Players::iterator it = std::find_if(m_Players.begin(), m_Players.end(), finder);
    
    if (it != m_Players.end())
    {
      m_Players.erase(it);
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::UpdatePlayers()
  {
    for (Aero::Players::iterator it = m_Players.begin(); it != m_Players.end(); ++it)
    {
      (*it)->Update(0);
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::UpdatePhysics()
  {
    if (m_LocalPlayer.get())
    {
      //always simulate client-side physics according to current input (but not confirmed!) 
      m_LocalPlayer->ProcessInput(m_Input);
    }

	  settings.hz = 70;
	  m_Phys->Step(settings);
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::DebugDraw(IRender::Ptr render)
  {
    m_Phys->SetTextLine(30);
    m_Phys->DebugDraw(settings);

   /* render->EnableBlend(true);
    int i = m_ImportantMoves.m_Head;
    while (i != m_ImportantMoves.m_Tail)
    {
      const PlayerMove& move = m_ImportantMoves[i];
      
      b2Vec2 v1(move.m_State.m_X - 20, move.m_State.m_Y - 20);
      b2Vec2 v2(move.m_State.m_X + 20, move.m_State.m_Y + 20);
      b2AABB aabb;
      aabb.lowerBound = v1;
      aabb.upperBound = v2;
      render->DrawAABB(&aabb, b2Color(0.2, 0.8, 0.8));

      m_ImportantMoves.Next(i);
    }
    render->EnableBlend(false);
*/
    //if (m_LocalPlayer.get())
    
   /* render->EnableBlend(true);
    for (int i = 0; i < 10; ++i)
    {
      b2Vec2 pos(0 + i * 20, 0 + i * 20);//m_LocalPlayer->GetPosition();
      b2Vec2 v1(pos.x - 20, pos.y - 20);
      b2Vec2 v2(pos.x + 20, pos.y + 20);
      b2AABB aabb;
      aabb.lowerBound = v1;
      aabb.upperBound = v2;
    
      render->DrawAABB(&aabb, b2Color(0.2, 0.2, 0.2));
    }
    render->EnableBlend(false);
    */
    render->DrawString(0, 10, GetInfo().m_Name.c_str());

    char buf[255] = {0};
    sprintf(buf, "im / m: %d / %d", m_ImportantMoves.GetSize(), m_Moves.GetSize());
    render->DrawString(0, 20, buf);
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::SendInputEvent()
  {
    InputEvent ev;
    ev.m_Time = m_Time;
    ev.m_Input = m_Input;
    ev.m_ImportantMoves = m_ImportantMoves;

    Bytes bytes;
    BinarySerializer sr(bytes);

    sr.Serialize((char)MSG_CLIENT_INPUT);
    sr.Serialize((unsigned long long)ev.m_Time);
    sr.Serialize(&ev.m_Input, sizeof(InputState));
    
    sr.Serialize((unsigned long long)ev.m_ImportantMoves.GetSize());
    
    int i = ev.m_ImportantMoves.m_Head;
    while (i != ev.m_ImportantMoves.m_Tail)
    {
      const PlayerMove& move = ev.m_ImportantMoves[i];
      sr.Serialize(&move, sizeof(PlayerMove));
      ev.m_ImportantMoves.Next(i);
    }

    m_Impl->Send(bytes);//, true);
  }
  //------------------------------------------------------------------------------------------

  void AeroClient::Click(int x, int y)
  {
    m_LocalPlayer->Impulse(x, y);
    SendChanged(x, y);
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::SendShoot(float x, float y)
  {
    Bytes bytes;

    BinarySerializer sr(bytes);
    sr.Serialize((char)MSG_CLIENT_SHOOT_REQ);
    sr.Serialize((float)x);
    sr.Serialize((float)y);

    m_Impl->Send(bytes, true);
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::Shoot(int x, int y)
  {
    SendShoot(x, y);
  }
  //-----------------------------------------------------------------------------------

  #define  GLUT_KEY_LEFT                      0x0064
  #define  GLUT_KEY_UP                        0x0065
  #define  GLUT_KEY_RIGHT                     0x0066
  #define  GLUT_KEY_DOWN                      0x0067

  void AeroClient::Keyboard(int key, bool pressed)
  {
    switch (key)
    {
    case GLUT_KEY_LEFT:
      m_Input.m_Left = pressed;
      break;

    case GLUT_KEY_RIGHT:
      m_Input.m_Right = pressed;
      break;

    case GLUT_KEY_DOWN:
      m_Input.m_Down = pressed;
      break;

    case GLUT_KEY_UP:
      m_Input.m_Up = pressed;
      break;
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::Pause()
  {
    settings.pause = !settings.pause;
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::Update()
  {
    // Process all messages in queue
    /*{
      Common::CSLocker locker(SMGuard);
      for (NetMessages::iterator it = m_ServerMessages.begin(); it != m_ServerMessages.end(); ++it)
      {
        ProcessNetMessage(*it);
      }
      m_ServerMessages.clear();
    }*/

    UpdatePhysics();

    for (Bullets::iterator it = m_Bullets.begin(); it != m_Bullets.end();)
    {
      Bullet::Ptr bullet = *it;
      if (!--bullet->m_LifeTime)
      {
        it = m_Bullets.erase(it);
      }
      else
      {
        ++it;
      }
    }

    ++m_Time;
  }
  //-----------------------------------------------------------------------------------

  void AeroClient::Run()
  {
    Network::NetTime lastTime;
    Network::NetTime currentTime;
    lastTime = currentTime = Common::Timer::GetInstance().GetTime();

    float accumulator = 0.0f;
    int totalSteps = 0;

    while (m_Handler.IsActive())
    {
      currentTime = Common::Timer::GetInstance().GetTime();
      float timeDelta = (float)(currentTime - lastTime) * 0.001f;

      if (timeDelta <= 0.0f)
      {
        continue;
      }
      lastTime = currentTime;
      accumulator += timeDelta;

      while (accumulator >= CL_TIME_STEP)
      {
        accumulator -= CL_TIME_STEP;

        //get  state and store it into as history
        //may be move this to Keyboard/mouse callback?
        if (m_LocalPlayer.get())
        {
          PlayerMove move;
          move.m_Time = totalSteps;
          move.m_Input = m_Input;
          m_LocalPlayer->GetState(move.m_State);
          AddMoveToHistory(move);

          SendInputEvent();
        }

        Update();

        ++totalSteps;
      }

      m_Handler.OnUpdate();

#if defined(WIN32) || defined(WINNT)
      Sleep(10);
#endif
    }
  }
  //-----------------------------------------------------------------------------------

  std::auto_ptr<Aero::IAeroClient> CreateAeroClient(Network::INetPeerFactory& factory, IAeroClientHandler& h)
  {
    return std::auto_ptr<Aero::IAeroClient>(new Aero::AeroClient(factory, h));
  }
}
