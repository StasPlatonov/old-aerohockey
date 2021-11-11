#ifndef NUMBERS_SERVER_H
#define NUMBERS_SERVER_H

#include "Network/Server.h"
#include "NumbersTypes.h"

namespace Numbers
{
  class NumbersServer : public Network::INetServerHandler
  {
  public:
    NumbersServer(unsigned short port, const std::string& password, Network::INetPeerFactory& factory);
    ~NumbersServer();

    //INetServerHandler
    virtual void OnClientConnected(const Network::NetClientInfo& info);
    virtual void OnClientDisconnected(const Network::NetClientInfo& info);
    virtual void OnData(Network::NetId id, unsigned char* data, int size, Network::NetTime time);

    void Reset();
    void Run();

  private:
    void Move(const std::string& player, int number);
    void NewMove();

  private:
    std::auto_ptr<Network::INetServer> m_Server;

    int m_SecretNumber;
    typedef std::vector<std::string> Names;
    Names m_ConnectedNames;
    int m_CurrentClient;
    std::string m_PrevWinner;
    bool m_InProgress;

    unsigned int m_Time;
  };

  std::auto_ptr<Numbers::NumbersServer> CreateNumbersServer(unsigned short port, const std::string& password, Network::INetPeerFactory& factory);
}

#endif