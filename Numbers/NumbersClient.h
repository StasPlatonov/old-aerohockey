#ifndef NUMBERS_CLIENT_H
#define NUMBERS_CLIENT_H

#include "../Network/Client.h"
#include "NumbersTypes.h"

#include <boost/shared_ptr.hpp>

namespace Numbers
{
  class INumbersClient
  {
  public:
    virtual ~INumbersClient() {}
    virtual void Init(const std::string& name, const char* host, unsigned short port, const std::string& password = std::string()) = 0;
    virtual void Done() = 0;
    virtual void Move(int number) = 0;
    virtual const Network::NetClientInfo& GetInfo() const = 0;
    virtual void SendMessage(const std::string& m) = 0;
    typedef boost::shared_ptr<INumbersClient> Ptr;
  };
  //-----------------------------------------------------------------------------------

  class INumbersClientHandler
  {
  public:
    virtual ~INumbersClientHandler() {}
    virtual void OnMessage(const std::string& text) = 0;
    virtual void OnError(const std::string& text) = 0;
    virtual void OnMoveRequested() = 0;
    virtual void OnMoveResult(const MoveResult& mr) = 0;
    virtual void OnReset() = 0;
  };
  //-----------------------------------------------------------------------------------

  class NumbersClient : public virtual INumbersClient, public virtual Network::INetClientHandler
  {
  public:
    NumbersClient(Network::INetPeerFactory& factory, INumbersClientHandler& h);
    virtual ~NumbersClient();

    //INumbersClient
    virtual void Init(const std::string& name, const char* host, unsigned short port, const std::string& password = std::string());
    virtual void Done();
    virtual void Move(int number);
    virtual const Network::NetClientInfo& GetInfo() const;
    virtual void SendMessage(const std::string& m);

    //INetClientHandler
    virtual void OnConnected(const Network::NetServerInfo& info);
    virtual void OnDisconnected();
    virtual void OnData(unsigned char* data, int size, Network::NetTime time);

  private:
    INumbersClientHandler& m_Handler;
    std::auto_ptr<Network::INetClient> m_Impl;
    std::string m_Current;
    unsigned int m_Time;
  };
  //-----------------------------------------------------------------------------------
  
  std::auto_ptr<Numbers::INumbersClient> CreateNumbersClient(Network::INetPeerFactory& factory, INumbersClientHandler& h);
}

#endif