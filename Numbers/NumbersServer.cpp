#include "NumbersServer.h"
#include "NumbersTypes.h"

#include "Common/Serialization.h"
#include "Common/Timer.h"
#include "Common/Utils.h"

#include <Kbhit.h>

namespace Numbers
{
  using namespace Network;
  using namespace Common;

  const unsigned int SV_TICK_RATE = 50;//50
  const unsigned int SV_SNAPSHOTS_UPDATE_RATE = 20;

  const int NumPlayersToStart = 2;

  const unsigned short AERO_PORT = 2345;
  NumbersServer* g_Server = 0;
  //-----------------------------------------------------------------------------------

  NumbersServer::NumbersServer(unsigned short port, const std::string& password, INetPeerFactory& factory)
    : m_Server(CreateNetServer(factory, *this))
    , m_SecretNumber(0)
    , m_CurrentClient(-1)
    , m_InProgress(false)
  {
    g_Server = this;
    m_Server->Start(port, Network::NetServerInfo("Numbers game server v.1.0.", 32, !password.empty()), password);
  }

  NumbersServer::~NumbersServer()
  {
    m_Server->Shutdown();
  }
  //-----------------------------------------------------------------------------------

  void NumbersServer::OnClientConnected(const NetClientInfo& info)
  {
    printf("Player %s connected.\n", info.m_Name.c_str());

    //notify all
    {
      Bytes bytes;

      BinarySerializer sr(bytes);
      sr.Serialize((char)MSG_CLIENT_CONNECTED);
      sr.Serialize((unsigned long long)info.m_Id);          //ID
      sr.Serialize(info.m_Name);            //Name
      //printf("Send MSG_CLIENT_CONNECTED to all for client %s at (%d, %d)\n", info.m_Name.c_str(), pos.x, pos.y);

      m_Server->Send(EmptyNetId, bytes);
    }

    //if game waits for reset and it is enough clients to play - reset game
    if (!m_InProgress)
    {
      Reset();
    }
  }

  void NumbersServer::OnClientDisconnected(const NetClientInfo& info)
  {
    //notify all
    {
      Bytes bytes;
      BinarySerializer sr(bytes);
      sr.Serialize((char)MSG_CLIENT_DISCONNECTED);
      sr.Serialize((unsigned long long)info.m_Id);

      m_Server->Send(EmptyNetId, bytes);
    }

    const std::string cur = (m_CurrentClient != -1) ? m_ConnectedNames[m_CurrentClient] : std::string();

    m_ConnectedNames.clear();
    const ClientInfos& clients = m_Server->GetClients();
    for (ClientInfos::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
      m_ConnectedNames.push_back(it->second.m_Name);
    }

    if (m_ConnectedNames.empty()) //the last disconnected
    {
      m_CurrentClient = -1;
      return;
    }

    if (info.m_Name == cur)
    {
      m_CurrentClient = rand() % m_ConnectedNames.size();
    }
    NewMove();
  }
  //-----------------------------------------------------------------------------------

  void NumbersServer::Reset()
  {
    if (m_Server->GetClientsCount() < NumPlayersToStart)
    {
      printf("Not enough clients to play!\n");

      Bytes bytes;
      BinarySerializer sr(bytes);
      sr.Serialize((char)MSG_NOT_ENOUGH_CLIENTS);
      m_Server->Send(EmptyNetId, bytes);
      return;
    }

    printf("Reset\n");

    m_SecretNumber = rand() % 100;
    //printf("Server secret number is: %d\n", m_SecretNumber);
    
    //refill connected names to join players that were connected in-game
    m_ConnectedNames.clear();
    const ClientInfos& clients = m_Server->GetClients();
    for (ClientInfos::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
      m_ConnectedNames.push_back(it->second.m_Name);
    }

    if (m_PrevWinner.empty())
    {
      m_CurrentClient = rand() % m_ConnectedNames.size();
    }
    else //previous winner called Reset() - set him as current
    {
      Names::iterator it = std::find(m_ConnectedNames.begin(), m_ConnectedNames.end(), m_PrevWinner);
      if (it == m_ConnectedNames.end())
      {
        m_CurrentClient = rand() % m_ConnectedNames.size();
      }
      else
      {
        m_CurrentClient = std::distance(m_ConnectedNames.begin(), it);
      }
      m_PrevWinner.clear();
    }

    m_InProgress = true;

    //send notification
    Bytes bytes;

    BinarySerializer sr(bytes);
    sr.Serialize((char)MSG_RESET);

    m_Time = Common::Timer::GetInstance().GetTime();
    sr.Serialize((unsigned long long)m_Time);

    m_Server->Send(EmptyNetId, bytes);

    NewMove();
  }
  //-----------------------------------------------------------------------------------

  void NumbersServer::NewMove()
  {
    Bytes bytes;
    bytes.clear();

    BinarySerializer sr(bytes);
    sr.Serialize((char)MSG_NEW_MOVE);
    sr.Serialize(m_ConnectedNames[m_CurrentClient]);

    m_Server->Send(EmptyNetId, bytes);
  }
  //-----------------------------------------------------------------------------------

  void NumbersServer::OnData(NetId id, unsigned char* data, int size, Network::NetTime time)
  {
    const unsigned char pid = data[0];
    switch (pid)
    {
    case MSG_CLIENT_MOVE:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        std::string player;
        dsr.Deserialize(player);

        int number;
        dsr.Deserialize(number);

        Move(player, number);
        break;
      }
    case MSG_CHAT_MESSAGE:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        std::string sender;
        dsr.Deserialize(sender);

        std::string msg;
        dsr.Deserialize(msg);

        //broadcast to all
        Bytes bytes;
        BinarySerializer sr(bytes);
        sr.Serialize((char)MSG_CHAT_MESSAGE);
        sr.Serialize(sender);
        sr.Serialize(msg);
        //just send latency
        const NetTime lat = Common::Timer::GetInstance().GetTime() - time;
        //printf("Msg came with latency %llu\n", lat);
        sr.Serialize((unsigned long long)lat);

        m_Server->Send(EmptyNetId, bytes, true);
      }
    }
  }
  //-----------------------------------------------------------------------------------

  void NumbersServer::Move(const std::string& player, int number)
  {
    printf("Client move: %s %d\n", player.c_str(), number);
    if (player != m_ConnectedNames[m_CurrentClient])
    {
      printf("Wrong player tried to make a move.\n");
      return;
    }

    Bytes bytes;

    //send move result
    BinarySerializer sr(bytes);
    sr.Serialize((char)MSG_MOVE_RESULT);
    sr.Serialize(m_ConnectedNames[m_CurrentClient]);
    sr.Serialize(number);
    int moveResult;
    if (number > m_SecretNumber)
      moveResult = MR_GREATER;
    else
    if (number < m_SecretNumber)
      moveResult = MR_LESS;
    else
      moveResult = MR_WIN;

    sr.Serialize(moveResult);

    m_Server->Send(EmptyNetId, bytes);


    //send wait for reset message
    if (moveResult == MR_WIN)
    {
      m_InProgress = false;
      m_PrevWinner = m_ConnectedNames[m_CurrentClient];
      printf("MR_WIN: %s\n", m_PrevWinner.c_str());
      Reset();
      return;
    }

    //change client
    ++m_CurrentClient;
    if (m_CurrentClient >= m_ConnectedNames.size())
    {
      m_CurrentClient = 0;
    }

    //send new move
    NewMove();
  }
  //------------------------------------------------------------------------------------------

  void NumbersServer::Run()
  {
    while (true)
    {
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
#if defined(WIN32) || defined(WINNT)
      Sleep(30);
#endif
    }
  }

  std::auto_ptr<Numbers::NumbersServer> CreateNumbersServer(unsigned short port, const std::string& password, Network::INetPeerFactory& factory)
  {
    return std::auto_ptr<Numbers::NumbersServer>(new Numbers::NumbersServer(port, password, factory));
  }
}
