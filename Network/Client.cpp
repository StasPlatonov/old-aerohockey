#include "Client.h"
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

  Client::Client(INetPeerFactory& factory, INetClientHandler& h)
    : Common::ThreadObject(1000)
    , m_Factory(factory)
    , m_Handler(h)
    , peer(factory.CreatePeer(*this))
  {
    unsigned i;
    randomData1[0]=ID_RANDOM_DATA;
    for (i=0; i < RANDOM_DATA_SIZE_1-1; i++)
      randomData1[i+1]=i;
    randomData2[0]=ID_RANDOM_DATA;
    for (i=0; i < RANDOM_DATA_SIZE_2-1; i++)
      randomData2[i+1]=i;
  }

  Client::~Client()
  {
    Disconnect();
    Shutdown();
    m_Factory.DestroyPeer(peer);
    peer = 0;
  }
  //-----------------------------------------------------------------------------------

  void Client::Startup(const NetClientInfo& info)
  {
    PeerParameters params;
    nextSendTime = 0;
    peer->Startup(params);
    isConnected=false;
    m_Info = info;
    
    StartThread();
  }

  void Client::Shutdown()
  {
    StopThread();
    peer->Shutdown();
  }
  //-----------------------------------------------------------------------------------
  
  void Client::Connect(const char* host, unsigned short port, const std::string& password)
  {
    bool b = peer->Connect(host, port, password);
    if (!b)
    {
      printf("Client connect call failed!\n");
    }
  }

  void Client::Disconnect()
  {
    peer->Disconnect();
    isConnected=false;
  }
  //-----------------------------------------------------------------------------------

  bool Client::IsExitRequired() const
  {
    return IsStopping();
  }
  //-----------------------------------------------------------------------------------

  void Client::EventLoop()
  {
    while (true)
    {
      if (IsExitRequired())
      {
        break;
      }
      Update();

      WaitForStopEvent(10);

      //Sleep(10);
    }
  }
  //-----------------------------------------------------------------------------------

  void Client::OnRunThread()
  { 
    EventLoop();
  }

  //-----------------------------------------------------------------------------------

  void Client::Send(std::vector<unsigned char>& bytes, bool timed)
  {
    peer->Send(&bytes.front(), bytes.size(), m_ServerId, false, timed);
  }
  //-----------------------------------------------------------------------------------

  void Client::SendStringMessage(const std::string& str)
  {
    Bytes bytes;

    BinarySerializer sr(bytes);
    sr.Serialize((char)ID_MESSAGE_STRING);
    sr.Serialize(str);
    
    Send(bytes);
  }
  //-----------------------------------------------------------------------------------

  int Client::GetAvgPing() const
  {
    return peer->GetPing(m_ServerId);
  }
  //-----------------------------------------------------------------------------------

  void Client::OnAccepted(NetId id)
  {
    m_ServerId = id;
    isConnected=true;
    m_Info.m_Id = peer->GetId();

    Bytes bytes;

    BinarySerializer sr(bytes);
    sr.Serialize((char)ID_CLIENT_INFO);
    sr.Serialize(m_Info.m_Name);

    Send(bytes);
  }
  //-----------------------------------------------------------------------------------

  void Client::OnConnectionAttemptFailed(NetId id)
  {
  }

  void Client::OnNoFreeConnections(NetId id)
  {
    isConnected=false;
  }

  void Client::OnDisconnected(NetId id)
  {
    isConnected=false;
    printf("Server disconnected\n");
    m_Handler.OnDisconnected();
  }

  void Client::OnConnectionLost(NetId id)
  {
    isConnected=false;
    printf("Connection to server has been lost\n");
    m_Handler.OnDisconnected();
  }
  //-----------------------------------------------------------------------------------

  void Client::OnData(NetId id, unsigned char* data, int size, Network::NetTime time)
  {
    const unsigned char pid = data[0];
    switch (pid)
    {
    case ID_SERVER_INFO:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        NetServerInfo nsi;

        dsr.Deserialize(nsi.m_Name);

        dsr.Deserialize((int&)nsi.m_MaxConnections);
        dsr.Deserialize((char&)nsi.m_Protected);

        printf("Joined to server: %s (%d), %s\n", nsi.m_Name.c_str(), nsi.m_MaxConnections, nsi.m_Protected ? "protected" : "unprotected");

        m_Handler.OnConnected(nsi);
        break;
      }

    case ID_MESSAGE_STRING:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        std::string res;
        dsr.Deserialize(res);
        printf("Message: %s (%d ms)\n", res.c_str(),  Common::Timer::GetInstance().GetTime() - time);
        break;
      }
    default:
        m_Handler.OnData(data, size, time);
      break;
    }
  }
  //-----------------------------------------------------------------------------------

  void Client::Update()
  {
    unsigned int curTime = Common::Timer::GetInstance().GetTime();

    peer->Update();
    /*
    if (curTime > nextSendTime && isConnected)
    {
      if (rand() % 10==0)
      {
        peer->Send((const char*)&randomData2, RANDOM_DATA_SIZE_2, EmptyNetId, true);
      }
      else
      {
        peer->Send((const char*)&randomData1, RANDOM_DATA_SIZE_1, EmptyNetId, true);
      }

      nextSendTime = curTime + 50;
    }*/
  }
  //-----------------------------------------------------------------------------------

  void Client::PrintStat()
  {
    printf("Logging client statistics to ClientStats.txt\n");
    if (FILE* fp = fopen("ClientStats.txt","wt"))
    {
      std::string ss = peer->GetStatistics();
      fprintf(fp,"%s\n\n", ss.c_str());
      fclose(fp);
    }
  }
  //-----------------------------------------------------------------------------------

  std::auto_ptr<INetClient> CreateNetClient(INetPeerFactory& factory, INetClientHandler& h)
  {
    return std::auto_ptr<INetClient>(new Client(factory, h));
  }
}
