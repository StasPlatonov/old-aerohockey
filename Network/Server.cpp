#include "Server.h"
#include "Types.h"

#include "Common/Serialization.h"
#include "Common/Utils.h"
#include "Common/Timer.h"

#if defined(WIN32) || defined(WINNT)
#else
  #include <stdio.h>
#endif

namespace Network
{
  using namespace Common;
  namespace
  {
    #define RANDOM_DATA_SIZE_1 50
    #define RANDOM_DATA_SIZE_2 100

    char randomData1[RANDOM_DATA_SIZE_1] = {0};
    char randomData2[RANDOM_DATA_SIZE_2] = {0};
  }
  //-----------------------------------------------------------------------------------

  Server::Server(INetPeerFactory& factory, INetServerHandler& h)
    : Common::ThreadObject(1000)
    , m_Factory(factory)
    , m_Handler(h)
    , peer(factory.CreatePeer(*this))
  {
    nextSendTime=0;
    unsigned i;
    randomData1[0]=ID_RANDOM_DATA;
    for (i=0; i < RANDOM_DATA_SIZE_1-1; i++)
      randomData1[i+1]=i;
    randomData2[0]=ID_RANDOM_DATA;
    for (i=0; i < RANDOM_DATA_SIZE_2-1; i++)
      randomData2[i+1]=i;
  }
  Server::~Server()
  {
    Shutdown();
    m_Factory.DestroyPeer(peer);
    peer = 0;
  }
  //-----------------------------------------------------------------------------------

  void Server::Start(unsigned short port, const NetServerInfo& info, const std::string& password)
  {
    PeerParameters params;
    params.m_Port = (unsigned short) port;
    params.m_MaxConnections = info.m_MaxConnections;
    peer->Startup(params);
    peer->SetMaximumIncomingConnections(info.m_MaxConnections);
    //printf("GUID: %s\n", peer->GetMyGUID().ToString());
    peer->SetIncomingPassword(password);
    m_Info = info;
    StartThread();
  }

  void Server::Shutdown()
  {
    StopThread();
    peer->Shutdown();
  }
  //-----------------------------------------------------------------------------------
  
  bool Server::IsExitRequired() const
  {
    return IsStopping();
  }
  //-----------------------------------------------------------------------------------

  void Server::EventLoop()
  {
    while (true)
    {
      if (IsExitRequired())
      {
        break;
      }
      Update();
#if defined(WIN32) || defined(WINNT)
      Sleep(10);
#else
      WaitForStopEvent(10);
#endif
    }
  }
  //-----------------------------------------------------------------------------------

  void Server::OnRunThread()
  {
    EventLoop();
  }

  unsigned Server::ConnectionCount() const
  {
    return peer->GetConnectionsCount();
  }
  //-----------------------------------------------------------------------------------

  void Server::Send(NetId id, std::vector<unsigned char>& bytes, bool timed)
  {
    peer->Send(&bytes.front(), bytes.size(), id, (id == EmptyNetId), timed);
  }
  //-----------------------------------------------------------------------------------

  NetId Server::FindClient(const std::string& name) const
  {
    if (name.empty())
      return EmptyNetId;
 
    ClientNames::const_iterator nameIt = m_ClientNames.find(name);
    return (nameIt != m_ClientNames.end()) ? nameIt->second : EmptyNetId;
  }
  //-----------------------------------------------------------------------------------

  void Server::SendStringMessage(const std::string& name, const std::string& str)
  {
    NetId to = FindClient(name);

    Bytes bytes;

    BinarySerializer sr(bytes);
    sr.Serialize((char)ID_MESSAGE_STRING);
    sr.Serialize(str);

    Send(to, bytes, true);
  }
  //-----------------------------------------------------------------------------------

  int Server::GetAvgPing(NetId client) const
  {
    return peer->GetPing(client);
  }

  void Server::OnDisconnected(NetId id)
  {
    printf("Connections = %i\n", ConnectionCount());
    ClientInfos::iterator it = m_Clients.find(id);
    if (it != m_Clients.end())
    {
      const NetClientInfo info = it->second;
      printf("%s disconnected\n", info.m_Name.c_str());
      m_ClientNames.erase(info.m_Name);
      m_Clients.erase(it);
      m_Handler.OnClientDisconnected(info);
    }
  }

  void Server::OnConnectionLost(NetId id)
  {
    printf("Connections = %i\n", ConnectionCount());
    ClientInfos::iterator it = m_Clients.find(id);
    if (it != m_Clients.end())
    {
      const NetClientInfo info = it->second;
      printf("Connection with %s has been lost\n", info.m_Name.c_str());
      m_ClientNames.erase(info.m_Name);
      m_Clients.erase(it);
      m_Handler.OnClientDisconnected(info);
    }
  }
  //-----------------------------------------------------------------------------------

  void Server::OnNewIncomingConnection(NetId id)
  {
    printf("Connections = %i\n", ConnectionCount());
    m_Clients.insert(std::make_pair(id, NetClientInfo(id)));

    Bytes bytes;
    BinarySerializer sr(bytes);
    sr.Serialize((char)ID_SERVER_INFO);
    sr.Serialize(m_Info.m_Name);
    sr.Serialize((int)m_Info.m_MaxConnections);
    sr.Serialize((char)m_Info.m_Protected);

    peer->Send(&bytes.front(), bytes.size(), id, false, false);
  }
  //-----------------------------------------------------------------------------------

  void Server::OnData(NetId id, unsigned char* data, int size, Network::NetTime time)
  {
    const unsigned char pid = data[0];
    switch (pid)
    {
    case ID_CLIENT_INFO:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum
        
        NetClientInfo info(id);
        dsr.Deserialize(info.m_Name);

        printf("Client %s connected\n", info.m_Name.c_str());
        ClientInfos::iterator it = m_Clients.find(id);
        if (it == m_Clients.end())
        {
          printf("Error: client not found in server's list!");
        }
        m_Clients[id] = info;
        m_ClientNames.insert(std::make_pair(info.m_Name, id));
        m_Handler.OnClientConnected(info);
        break;
      }

    case ID_MESSAGE_STRING:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        std::string res;
        dsr.Deserialize(res);
        ClientInfos::const_iterator it = m_Clients.find(id);
        //assert(time != 0);
        printf(" %s: %s (%d ms)\n", it == m_Clients.end() ? "UNKNOWN USER" : it->second.m_Name.c_str(), res.c_str(),  Common::Timer::GetInstance().GetTime() - time);
        break;
      }
    default:
        m_Handler.OnData(id, data, size, time);
    }
  }
  //-----------------------------------------------------------------------------------

  void Server::Update()
  {
    unsigned int curTime = Common::Timer::GetInstance().GetTime();

    peer->Update();
    /*
    if (curTime > nextSendTime)
    {
      if (rand() % 10 == 0)
      {
        peer->Send((const char*)&randomData2, RANDOM_DATA_SIZE_2, EmptyNetId, true);
      }
      else
      {
        peer->Send((const char*)&randomData1, RANDOM_DATA_SIZE_1, EmptyNetId, true);
      }

      nextSendTime = curTime + 100;
    }*/
  }
  //-----------------------------------------------------------------------------------

  void Server::PrintStat()
  {
    //printf("Logging server statistics to ServerStats.txt\n");
    //if (FILE* fp = fopen("ServerStats.txt","wt"))
    {
      std::string ss = peer->GetStatistics();
      printf("%s\n\n", ss.c_str());

      printf("Pings:\n");
      for (ClientInfos::const_iterator it = m_Clients.begin(); it != m_Clients.end(); ++it)
      {
        printf("'%s': %d ms\n", it->second.m_Name.c_str(), GetAvgPing(it->second.m_Id));
      }

      //fclose(fp);
    }
  }
  //-----------------------------------------------------------------------------------

  void Server::Kick(const std::string& name)
  {
    if (name.empty())
    {
      return;
    }

    ClientNames::const_iterator nameIt = m_ClientNames.find(name);
    if (nameIt == m_ClientNames.end())
    {
      return;
    }
      
    peer->Kick(nameIt->second);
  }

  void Server::Ban(const std::string& ip, unsigned int ms)
  {
    peer->Ban(ip, ms);
  }

  std::auto_ptr<INetServer> CreateNetServer(INetPeerFactory& factory, INetServerHandler& h)
  {
    return std::auto_ptr<INetServer>(new Server(factory, h));
  }
}
