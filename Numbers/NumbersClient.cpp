#include "NumbersClient.h"

#include "Common/Serialization.h"
#include "Common/Utils.h"
#include "Common/Timer.h"

#include <stdlib.h>

#if defined(WIN32) || defined(WINNT)
#else
  #include <stdio.h>
#endif

namespace Numbers
{
  using namespace Network;
  using namespace Common;

  //-----------------------------------------------------------------------------------

  NumbersClient::NumbersClient(INetPeerFactory& factory, INumbersClientHandler& h)
    : m_Impl(CreateNetClient(factory, *this))
    , m_Handler(h)
  {
  }

  NumbersClient::~NumbersClient()
  {
  }
  //-----------------------------------------------------------------------------------

  void NumbersClient::OnConnected(const Network::NetServerInfo& info)
  {
  }

  void NumbersClient::OnDisconnected()
  {
  }
  //-----------------------------------------------------------------------------------

  void NumbersClient::OnData(unsigned char* data, int size, Network::NetTime time)
  {
    const unsigned char pid = data[0];
    switch (pid)
    {
    case MSG_RESET:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        dsr.Deserialize((unsigned long long&) m_Time);

        m_Current.clear();
        m_Handler.OnReset();
        break;
      }

    case MSG_NEW_MOVE:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        dsr.Deserialize(m_Current);

        char str[255] = {0};
        if (GetInfo().m_Name == m_Current)
        {
          sprintf(str, "Make your move: ");
        }
        else
        {
          sprintf(str, "Waiting for player '%s' to move..\n", m_Current.c_str());
        }
        m_Handler.OnMessage(str);

        if (GetInfo().m_Name == m_Current)
        {
          m_Handler.OnMoveRequested();
        }
        break;
      }

    case MSG_MOVE_RESULT:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        std::string client;
        dsr.Deserialize(client);

        int num = 0;
        dsr.Deserialize(num);

        int moveResult;
        dsr.Deserialize(moveResult);

        m_Handler.OnMoveResult(MoveResult(client, num, moveResult));
        break;
      }

    case MSG_CLIENT_CONNECTED:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        NetId id;
        dsr.Deserialize((unsigned long long&)id);       //ID
        std::string name;
        dsr.Deserialize(name);                          //Name

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
        break;
      }

    case MSG_CLIENT_DISCONNECTED:
      {
        BinaryDeserializer dsr(data, size);
        dsr.Ignore(sizeof(unsigned char)); // Ignore the packet type enum

        NetId id;
        dsr.Deserialize((unsigned long long&)id);
        break;
      }

    case MSG_NOT_ENOUGH_CLIENTS:
      {
        m_Handler.OnMessage("Not enought clients to play. Please, wait..\n");
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

        unsigned long long senderLatency;
        dsr.Deserialize((unsigned long long&)senderLatency);

        if (sender != m_Impl->GetInfo().m_Name)
        {
          char str[255] = {0};
          //total latency is sender_to_server_latency + server_to_us_latency
          const NetTime thisLatency = Common::Timer::GetInstance().GetTime() - time;
          const NetTime totalLatency = senderLatency + thisLatency;
          sprintf(str, "[%s]: %s (%llu ms)\n", sender.c_str(), msg.c_str(), totalLatency);
          m_Handler.OnMessage(str);
        }
        break;
      }
    }
  }
  //-----------------------------------------------------------------------------------

  void NumbersClient::Init(const std::string& name, const char* host, unsigned short port, const std::string& password)
  {
    m_Impl->Startup(Network::NetClientInfo(name));
    m_Impl->Connect(host, port, password);
  }

  void NumbersClient::Done()
  {
    m_Impl->Disconnect();
    m_Impl->Shutdown();
  }
  //-----------------------------------------------------------------------------------

  const Network::NetClientInfo& NumbersClient::GetInfo() const
  {
    return m_Impl->GetInfo();
  }
  //-----------------------------------------------------------------------------------
  
  //INumbersClient
  void NumbersClient::Move(int number)
  {
    if (m_Current != GetInfo().m_Name)
    {
      char str[255] = {0};
      sprintf(str, "You can not move until player '%s' finish his move.\n", m_Current.c_str());
      m_Handler.OnError(str);
      return;
    }
    
    Bytes bytes;

    BinarySerializer sr(bytes);
    sr.Serialize((char)MSG_CLIENT_MOVE);
    sr.Serialize(m_Impl->GetInfo().m_Name);
    sr.Serialize(number);

    m_Impl->Send(bytes);
  }
  //-----------------------------------------------------------------------------------

  void NumbersClient::SendMessage(const std::string& m)
  {
    Bytes bytes;

    BinarySerializer sr(bytes);
    sr.Serialize((char)MSG_CHAT_MESSAGE);
    sr.Serialize(m_Impl->GetInfo().m_Name);
    sr.Serialize(m);

    m_Impl->Send(bytes, true);
  }
  //-----------------------------------------------------------------------------------

  std::auto_ptr<Numbers::INumbersClient> CreateNumbersClient(Network::INetPeerFactory& factory, INumbersClientHandler& h)
  {
    return std::auto_ptr<Numbers::INumbersClient>(new Numbers::NumbersClient(factory, h));
  }
}
