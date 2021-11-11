#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "Network/INetwork.h"

#include "Numbers/NumbersServer.h"
#include "Numbers/NumbersConsoleClient.h"

#include "Aero/AeroServer.h"
#include "Aero/AeroGLClient.h"

#include "boost/shared_ptr.hpp"
#include <vector>
//-----------------------------------------------------------------------------------

class ILauncher
{
public:
  virtual ~ILauncher() {};

  virtual void Run() = 0;
};
//-----------------------------------------------------------------------------------

namespace Numbers
{
  class NumbersLauncher : public ILauncher
  {
  public:
    explicit NumbersLauncher(Network::INetPeerFactory& factory);
    ~NumbersLauncher();

    virtual void Run();

  private:
    Network::INetPeerFactory& m_Factory;
    std::auto_ptr<NumbersServer> m_Server;
    std::auto_ptr<NumbersConsoleClient> m_AIClient;
    std::auto_ptr<NumbersConsoleClient> m_Client;
  };
}
//-----------------------------------------------------------------------------------

namespace Aero
{
  class AeroLauncher : public ILauncher
  {
  public:
    explicit AeroLauncher(Network::INetPeerFactory& factory);
    ~AeroLauncher();

    virtual void Run();

  private:
    Network::INetPeerFactory& m_Factory;
    std::auto_ptr<AeroServer> m_Server;
    std::auto_ptr<AeroGLClient> m_Client;
  };
}
//-----------------------------------------------------------------------------------

namespace Test
{
  class ServerClientTestLauncher : public ILauncher
  {
  public:
    explicit ServerClientTestLauncher(Network::INetPeerFactory& factory);
    ~ServerClientTestLauncher();

    virtual void Run();

  private:
    Network::INetPeerFactory& m_Factory;
    int m_Mode;

    typedef boost::shared_ptr<Network::INetClient> ClientPtr;
    typedef std::vector<ClientPtr> Clients;
    Clients m_Clients;

    std::auto_ptr<Network::INetServer> m_Server;
  };
}

#endif