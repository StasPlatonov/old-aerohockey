#include "AeroServer.h"
#include "AeroTypes.h"

#include "Common/Serialization.h"
#include "Common/Timer.h"
#include "Common/Utils.h"

#include <GL/freeglut.h>
#include <Gl/glui.h>
#include <Kbhit.h>

namespace Aero
{
  using namespace Network;
  using namespace Common;

  const unsigned int SV_SYNC_RATE = 50; // 50 ticks per second are simulated
  const float SV_TIME_STEP = 0.01f; // 100 ticks per second
  const int SV_RENDER_TICK = 16; // ~60 fps
  const int MAX_CLIENT_MESSAGES = 256;

  const int NumPlayersToStart = 2;

  const unsigned short AERO_PORT = 2345;
  AeroServer* g_Server = 0;

	int32 width = 160;
	int32 height = 160;
	int32 mainWindow;
	float settingsHz = 70.0;
  float32 viewZoom = 1.0f;
  Settings settings;
#ifdef USE_GL
    static void Resize_S2(int32 w, int32 h)
    {
      g_Server->Resize(w, h);
    }

    void RenderTimer_S(int i)
    {
      g_Server->OnRenderTimer(i);
    }

    static void RenderFrame_S()
    {
      g_Server->RenderFrame();
    }
#endif
  //-----------------------------------------------------------------------------------

  AeroServer::AeroServer(unsigned short port, const std::string& password, INetPeerFactory& factory)
    : Common::ThreadObject(1000)
    , m_Server(CreateNetServer(factory, *this))
  {
    g_Server = this;
    m_Server->Start(port, Network::NetServerInfo("Aero game server v.1.0.", 32, !password.empty()), password);
    
    m_Render = IRender::Ptr(CreateGLRender().release());
    //m_Render = IRender::Ptr(CreateNullRender().release());
    m_Phys = AeroPhysWorld::Ptr(AeroPhysWorld::Create(m_Render).release());
  #ifdef USE_GL
      int argc = 1;
      char* argv = "ServerSide";
      glutInit(&argc, &argv);
      glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
      glutInitWindowSize(width, height);
      char title[32];
      sprintf(title, "Box2D4 Version %d.%d.%d", b2_version.major, b2_version.minor, b2_version.revision);
      mainWindow = glutCreateWindow(title);
      //glutSetOption (GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

      glutDisplayFunc(RenderFrame_S);
      GLUI_Master.set_glutReshapeFunc(Resize_S2);
      glutTimerFunc(SV_RENDER_TICK, RenderTimer_S, 0);
  #endif
      //settings.pause = true;
    StartThread();
  }

  AeroServer::~AeroServer()
  {
    StopThread(1000);
    m_Server->Shutdown();
  }
  //-----------------------------------------------------------------------------------

  void AeroServer::Synchronize()
  {
    /*if (m_Players.empty())
    {
      return;
    }
    Bytes bytes;
    BinarySerializer sr(bytes);
    sr.Serialize((char)MSG_SYNCHRONIZE);

    static DynamicClientDataMap oldData;

    DynamicClientDatas dcds;
    for (Players::const_iterator it = m_Players.begin(); it != m_Players.end(); ++it)
    {
      const Player::Ptr& p = *it;
      //if (!p->IsAwake())//skip sleeping players
      //  continue;
      const b2Vec2 pos = p->m_Body->GetPosition();
      const b2Vec2 vel = p->m_Body->GetLinearVelocity();

      DynamicClientDataMap::iterator oldIt = oldData.find(p->m_NetInfo.m_Id);

      if (oldIt == oldData.end()) //we have no old data for this player
      {
        DynamicClientData dcd;
        dcd.m_Id = p->m_NetInfo.m_Id;
        dcd.m_X = pos.x;
        dcd.m_Y = pos.y;
        dcd.m_VX = vel.x;
        dcd.m_VY = vel.y;
        oldData.insert(std::make_pair(dcd.m_Id, dcd));
        dcds.push_back(dcd);
      }
      else
      {
        DynamicClientData& old = oldIt->second;
        if (pos.x == old.m_X && pos.y == old.m_Y && vel.x == old.m_VX && vel.y == old.m_VY)
        {
          continue;
        }
        else
        {
          old.m_Id = p->m_NetInfo.m_Id;
          old.m_X = pos.x;
          old.m_Y = pos.y;
          old.m_VX = vel.x;
          old.m_VY = vel.y;
          dcds.push_back(old);
        }
      }
    }

    if (dcds.empty())
    {
      return;
    }

    sr.Serialize((int)dcds.size());
    sr.Serialize(&dcds.front(), dcds.size() * sizeof(DynamicClientData));

    m_Server->Send(EmptyNetId, bytes, true);*/
  }
  //-----------------------------------------------------------------------------------

  Player::Ptr AeroServer::CreatePlayer(const Network::NetClientInfo& info)
  {
    Player::Ptr player(new Player(info, m_Phys));
    m_Players.push_back(player);

    const int initialX = -20 + rand() % 40;
    const int initialY =  0 + rand() % 40;

    player->SetPosition(initialX, initialY);

    return player;
  }

  Aero::Player::Ptr AeroServer::FindPlayer(NetId id)
  {
    PlayerFinder finder(id);
    Aero::Players::iterator it = std::find_if(m_Players.begin(), m_Players.end(), finder);
    
    return it == m_Players.end() ? Aero::Player::Ptr() : *it;
  }

  void AeroServer::DestroyPlayer(Network::NetId id)
  {
    PlayerFinder finder(id);
    Aero::Players::iterator it = std::find_if(m_Players.begin(), m_Players.end(), finder);
    
    if (it != m_Players.end())
    {
      m_Players.erase(it);
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroServer::OnClientConnected(const NetClientInfo& info)
  {
    printf("Player %s connected.\n", info.m_Name.c_str());

    Player::Ptr newPlayer = CreatePlayer(info);
    //notify all
    {
      Bytes bytes;

      BinarySerializer sr(bytes);
      sr.Serialize((char)MSG_CLIENT_CONNECTED);
      sr.Serialize((unsigned long long)info.m_Id);          //ID
      sr.Serialize(info.m_Name);            //Name
      const b2Vec2 pos = newPlayer->m_Body->GetPosition();  
      sr.Serialize((float)pos.x);                             //Position
      sr.Serialize((float)pos.y);
      //printf("Send MSG_CLIENT_CONNECTED to all for client %s at (%d, %d)\n", info.m_Name.c_str(), pos.x, pos.y);
      m_Server->Send(EmptyNetId, bytes);
    }

    //send current state to connected client
    const int numOthers = m_Players.size() - 1;
    if (numOthers)
    {
      Bytes bytes;
      BinarySerializer sr(bytes);

      sr.Serialize((char)MSG_CURRENT_WORLD_STATE);

      sr.Serialize((int)numOthers);
      //printf("Send MSG_CURRENT_WORLD_STATE with %d players\n", numOthers);
      for (Players::const_iterator it = m_Players.begin(); it != m_Players.end(); ++it)
      {
        const Player::Ptr& otherPlayer = *it;
        if (otherPlayer->m_NetInfo.m_Id == info.m_Id) //skip already added player
        {
          continue;
        }
        sr.Serialize((unsigned long long)otherPlayer->m_NetInfo.m_Id);          //ID
        sr.Serialize(otherPlayer->m_NetInfo.m_Name);                            //Name
        const b2Vec2 pos = otherPlayer->m_Body->GetPosition();  
        sr.Serialize((float)pos.x);                             //Position
        sr.Serialize((float)pos.y);
      }

      m_Server->Send(info.m_Id, bytes);
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroServer::OnClientDisconnected(const NetClientInfo& info)
  {
    DestroyPlayer(info.m_Id);

    // remove from queues
    {
      Common::MutexLocker locker(CMGuard);
      m_ClientsMessages.erase(info.m_Id);
    }

    // notify all
    {
      Bytes bytes;
      BinarySerializer sr(bytes);
      sr.Serialize((char)MSG_CLIENT_DISCONNECTED);
      sr.Serialize((unsigned long long)info.m_Id);

      m_Server->Send(EmptyNetId, bytes);
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroServer::OnData(NetId id, unsigned char* data, int size, Network::NetTime time)
  {
    //! We enqueue network message for futher processing for network performance.
    //! We can just also call ProcessNetMessage() from here.
    const unsigned char pid = data[0];
  
    /*
    Common::CSLocker locker(CMGuard);
    ClientsMessages::iterator clIt = m_ClientsMessages.find(id);
    if (clIt == m_ClientsMessages.end())
    {
      if (m_ClientsMessages.size() > MAX_CLIENT_MESSAGES)
      {
        printf("WARNING: Maximum client messages reached for client %llu.", id);
        return;
      }
      clIt = m_ClientsMessages.insert(std::make_pair(id, NetMessages())).first;
    }

    clIt->second.push_back(NetMessage(pid, time, data, size));
    */
    ProcessNetMessage(id, NetMessage(pid, time, data, size));
  }
  //-----------------------------------------------------------------------------------

  void AeroServer::ProcessNetMessage(Network::NetId clientId, const NetMessage& msg)
  {
    const unsigned char pid = msg.Id;
    const unsigned char* data = &msg.Data.front();
    int size = msg.Data.size();
    Network::NetTime time = msg.Time;
    Network::NetId id = clientId;
    
    switch (pid)
    {
    case MSG_CLIENT_CHANGED:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        float x,y;
        dsr.Deserialize((float&)x);
        dsr.Deserialize((float&)y);

        //ignore old inputs
        if (time < Common::Timer::GetInstance().GetTime())
        {
          //break;
        }

        OnClientChanged(id, x, y);

        //broadcast to all that player changed
        Bytes bytes;
        BinarySerializer sr(bytes);
        sr.Serialize((char)MSG_CLIENT_CHANGED);
        sr.Serialize((unsigned long long)id);
        sr.Serialize((float)x);
        sr.Serialize((float)y);
        m_Server->Send(EmptyNetId, bytes);
        break;
      }
    
    case MSG_CLIENT_SHOOT_REQ:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        float x,y;
        dsr.Deserialize((float&)x);
        dsr.Deserialize((float&)y);

        if (Player::Ptr player = FindPlayer(id))
        {
          const b2Vec2 pos = player->GetPosition();

          b2Vec2 vel = b2Vec2(x - pos.x, y - pos.y);
          vel.Normalize();
          vel *= 200.0f;
          m_Bullets.push_back(Bullet::Ptr(new Bullet(pos.x, pos.y, vel.x, vel.y, m_Phys)));

          //broadcast to all that player changed
          Bytes bytes;
          BinarySerializer sr(bytes);
          sr.Serialize((char)MSG_CLIENT_SHOOT_RESP);
          sr.Serialize((unsigned long long)id);
          sr.Serialize((float)pos.x);
          sr.Serialize((float)pos.y);
          sr.Serialize((float)vel.x);
          sr.Serialize((float)vel.y);
          m_Server->Send(EmptyNetId, bytes);
        }
        break;
      }

    case MSG_CLIENT_INPUT:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        InputEvent ev;
        dsr.Deserialize((unsigned long long&)ev.m_Time);
        dsr.Deserialize(&ev.m_Input, sizeof(InputState));

        unsigned long long movesCount = 0;
        dsr.Deserialize((unsigned long long&)movesCount);
        ev.m_ImportantMoves.Resize(1000);
        for (int i = 0; i < movesCount; ++i)
        {
          PlayerMove move;
          dsr.Deserialize(&move, sizeof(PlayerMove));
          ev.m_ImportantMoves.Add(move);
        }
        
        ProcessInputEvent(id, ev);
        break;
      }
    }
  }
  //-----------------------------------------------------------------------------------

  /*!
    Process input event, received from client. Affect phycics, read actual client state and send it back to client
  */
  void AeroServer::ProcessInputEvent(Network::NetId id, const InputEvent& ie)
  {
    while (m_Time < ie.m_Time)
    {
      //Update() is called from main server thread, so just wait;
    }

    if (Player::Ptr player = FindPlayer(id))
    {
      player->ProcessInput(ie.m_Input);

      //send confirmation event
      Bytes bytes;
      BinarySerializer sr(bytes);
      sr.Serialize((char)MSG_CLIENT_INPUT_CONFIRM);
      sr.Serialize((unsigned long long)id);
      
      SyncEvent se;
      se.m_Time = m_Time;
      se.m_Input = ie.m_Input;
      player->GetState(se.m_State);
      sr.Serialize(&se, sizeof(SyncEvent));

      m_Server->Send(EmptyNetId, bytes);
    }
  }

  void AeroServer::OnClientChanged(Network::NetId id, float x, float y)
  {
    if (Player::Ptr player = FindPlayer(id))
    {
      player->Impulse(x, y);
    }
  }

  void AeroServer::OnClientForce(Network::NetId id, float x, float y)
  {
    if (Player::Ptr player = FindPlayer(id))
    {
      player->Force(x, y);
    }
  }
  //------------------------------------------------------------------------------------------

  void AeroServer::OnRunThread()
  {
    SyncLoop();
  }

  void AeroServer::SyncLoop()
  {
    while (true)
    {
      if  (WaitForStopEvent(1000 / SV_SYNC_RATE))
      {
        break;
      }

      //Synchronize();
    }
  }
  //------------------------------------------------------------------------------------------

  void AeroServer::UpdatePhysics()
  {
    settings.hz = settingsHz;
	  m_Phys->Step(settings);
  }
  //------------------------------------------------------------------------------------------

  void AeroServer::Update()
  {
    // Process all messages in queue
    /*{
      Common::CSLocker locker(CMGuard);
      for (ClientsMessages::iterator it = m_ClientsMessages.begin(); it != m_ClientsMessages.end(); ++it)
      {
        const NetMessages& cmsgs = it->second;
        for (NetMessages::const_iterator it2 = cmsgs.begin(); it2 != cmsgs.end(); ++it2)
        {
          ProcessNetMessage(it->first, *it2);
        }
        it->second.clear();
      }
    }
    */

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
  //------------------------------------------------------------------------------------------

  void AeroServer::Run()
  {
    Network::NetTime lastTime;
    Network::NetTime currentTime;
    lastTime = currentTime = Common::Timer::GetInstance().GetTime();

    float accumulator = 0.0f;
    int totalSteps = 0;

    while (true)
    {
      currentTime = Common::Timer::GetInstance().GetTime();
      float timeDelta = (float)(currentTime - lastTime) * 0.001f;

      if (timeDelta <= 0.0f)
      {
        continue;
      }
      lastTime = currentTime;
      accumulator += timeDelta;

      while (accumulator >= SV_TIME_STEP)
      {
        accumulator -= SV_TIME_STEP;

        Update();
        ++totalSteps;
      }
#if defined(WIN32) || defined(WINNT)
      Sleep(10);
#endif
#ifdef USE_GL
      glutMainLoopEvent();
#endif

#if defined(WIN32) || defined(WINNT)
      if (_kbhit())
#else
      if (kbhit())
#endif
      {
        const std::string command = Utils::GetString();

        if (command == "quit" || command == "exit")
        {
          break;
        }

        if (command == "cls")
        {
          system("cls");
        }

        if (command == "kick")
        {
          printf("\nEnter client name to kick: ");
          const std::string name = Utils::GetString();
          //m_Server->Kick(name);
          continue;
        }

        if (command == "ban")
        {
          printf("\nEnter ip address to ban: ");
          const std::string ip = Utils::GetString();
          printf("\nEnter time(ms) to ban: ");
          const std::string tm = Utils::GetString();
          const unsigned int time = atoi(tm.c_str());
          //m_Server->Ban(ip, time);
          continue;
        }
        if (command == "stat")
        {
          m_Server->PrintStat();
        }
      }
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroServer::Resize(int32 w, int32 h)
  {
	  width = w;
	  height = h;

    int tx, ty, tw, th;
    GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
	  glViewport(tx, ty, tw, th);

	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
	  float32 ratio = float32(tw) / float32(th);

	  b2Vec2 extents(ratio * 25.0f, 25.0f);
	  extents *= viewZoom;

	  b2Vec2 lower = settings.viewCenter - extents;
	  b2Vec2 upper = settings.viewCenter + extents;

	  // L/R/B/T
	  gluOrtho2D(lower.x, upper.x, lower.y, upper.y);
  }
  //-----------------------------------------------------------------------------------

  void AeroServer::OnRenderTimer(int i)
  {
	  glutSetWindow(mainWindow);
	  glutPostRedisplay();
#ifdef USE_GL
    glutTimerFunc(SV_RENDER_TICK, RenderTimer_S, 0);
#endif
  }
  //-----------------------------------------------------------------------------------

  void AeroServer::RenderFrame()
  {
    glClearColor(255, 255, 255, 255);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();

	  m_Phys->SetTextLine(30);
	  
    b2Vec2 oldCenter = settings.viewCenter;
    m_Phys->DebugDraw(settings);

	  if (oldCenter.x != settings.viewCenter.x || oldCenter.y != settings.viewCenter.y)
	  {
		  Resize(width, height);
	  }

    m_Render->DrawString(0, 10, "server");
    
    char buf[128] = {0};
    int pos = 20;
    for (ClientsMessages::const_iterator it = m_ClientsMessages.begin(); it != m_ClientsMessages.end(); ++it)
    {
      sprintf(buf, "cqs[%llu] : %d", it->first, it->second.size());
      m_Render->DrawString(0, pos, buf);
      pos += 10;
    }

    glutSwapBuffers();
  }
  //-----------------------------------------------------------------------------------

  std::auto_ptr<Aero::AeroServer> CreateAeroServer(unsigned short port, const std::string& password, Network::INetPeerFactory& factory)
  {
    return std::auto_ptr<Aero::AeroServer>(new Aero::AeroServer(port, password, factory));
  }
}
