#ifndef NUMBERS_CONSOLE_CLIENT_H
#define NUMBERS_CONSOLE_CLIENT_H

#include "Network/INetwork.h"
#include "NumbersClient.h"

#include "Common/Thread.h"

namespace Numbers
{
  //-----------------------------------------------------------------------------------
  class NumbersConsoleClient : public Numbers::INumbersClientHandler
  {
  public:
    NumbersConsoleClient(const std::string& name, const std::string& host, const std::string& password, Network::INetPeerFactory& factory);
    virtual ~NumbersConsoleClient();

    //INumbersClientHandler
    virtual void OnMessage(const std::string& text);
    virtual void OnError(const std::string& text);
    virtual void OnMoveRequested();
    virtual void OnMoveResult(const Numbers::MoveResult& mr);
    virtual void OnReset();
    void Run();

  protected:
    void Move(int number);
    virtual void Idle();
    bool IsActive() const { return m_Active; }

  private:
    std::auto_ptr<Numbers::INumbersClient> m_Client;
    bool m_MoveRequested;
    bool m_Active;
  };
  //-----------------------------------------------------------------------------------

  class NumbersConsoleAIClient : public NumbersConsoleClient
  {
  public:
    NumbersConsoleAIClient(const std::string& name, const std::string& host, const std::string& password, Network::INetPeerFactory& factory);
    virtual ~NumbersConsoleAIClient();

    //INumbersClientHandler
    virtual void OnMoveResult(const Numbers::MoveResult& mr);
    virtual void OnMoveRequested();
    virtual void OnReset();

    //ConsoleClient
    virtual void Idle();

  private:
    bool IsMoveRequested() const;

  private:
    Common::EventObject m_MoveRequestedEvent;
    int m_Min;
    int m_Max;
  };
  //-----------------------------------------------------------------------------------
}

#endif //NUMBERS_CONSOLE_CLIENT_H
