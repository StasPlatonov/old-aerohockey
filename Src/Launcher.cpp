#include "Launcher.h"
#include "Common/Utils.h"

#include <Kbhit.h>
#include <ctime>

#if defined(WIN32) || defined(WINNT)
#else
  #include <termios.h>
  #include <stdio.h>
  #include <sys/ioctl.h>
#endif

namespace
{
  const unsigned short AERO_PORT = 2345;
/*
#if !defined(WIN32) && !defined(WINNT)
  #define STDIN 0
  class Term
  {
  public:
    Term()
  	  {}
    bool KBHit()
    {
      tcgetattr(STDIN, &Old);
      termios term = Old;
      term.c_lflag &= ~ICANON;
      tcsetattr(STDIN, TCSANOW, &term);
      //setbuf(stdin, NULL);

      int bytesWaiting = 0;
      ioctl(STDIN, FIONREAD, &bytesWaiting);

      tcsetattr(STDIN, TCSANOW, &Old);
      return (bytesWaiting > 0);
    }
  private:
    termios Old;
  };
#endif
*/
  char GetChar()
  {
#if defined(WIN32) || defined(WINNT)
    return _getche();
#else
    return getchar();
#endif
  }

  bool KbHit()
  {
#if defined(WIN32) || defined(WINNT)
    return _kbhit();
#else
    //return Term().KBHit();
    return kbhit();
#endif
  }
}

namespace Numbers
{
  NumbersLauncher::NumbersLauncher(Network::INetPeerFactory& factory)
    : m_Factory(factory)
  {
    srand(time(NULL));
    printf("Welcome to Numbers!!!\n");

    printf("Run as (S)erver, (P)layer, (A)AI player?");
    const char ch = GetChar();
    if (ch == 's' || ch == 'S')
    {
      printf("\nEnter password for this server(empty by default): ");
      const std::string password = Utils::GetString();
      m_Server = Numbers::CreateNumbersServer(AERO_PORT, password, factory);
    }
    else
      if (ch == 'p' || ch == 'P')
      {
        printf("\nEnter your name: ");
        const std::string name = Utils::GetString();

        printf("Enter IP to connect client to('localhost' by default): ");
        const std::string host = Utils::GetString();

        printf("Enter password to connect(empty by default): ");
        const std::string password = Utils::GetString();

        m_Client.reset(new Numbers::NumbersConsoleClient(name, host.empty() ? "localhost" : host, password, factory));
      }
      if (ch == 'a' || ch == 'A')
      {
        printf("\nEnter AI name: ");
        const std::string name = Utils::GetString();

        printf("Enter IP to connect AI to('localhost' by default): ");
        const std::string host = Utils::GetString();

        printf("Enter password to connect(empty by default): ");
        const std::string password = Utils::GetString();
        m_AIClient.reset(new Numbers::NumbersConsoleAIClient(name, host.empty() ? "localhost" : host, password, factory));
      }
  }

  NumbersLauncher::~NumbersLauncher()
  {
  }
  //-----------------------------------------------------------------------------------

  void NumbersLauncher::Run()
  {
    if (m_Server.get())
    {
      m_Server->Run();
    }
    else
    if (m_Client.get())
    {
      m_Client->Run();
    }
    else
    if (m_AIClient.get())
    {
      m_AIClient->Run();
    }
  }
  //-----------------------------------------------------------------------------------
}

namespace Aero
{
  AeroLauncher::AeroLauncher(Network::INetPeerFactory& factory)
    : m_Factory(factory)
  {
    srand(time(NULL));
    printf("Welcome to Aero!!!\n");

    printf("Run as (S)erver, (P)layer?\n");

    const char ch = GetChar();
    if (ch == 's' || ch == 'S')
    {
      printf("\nEnter password for this server(empty by default): ");
      const std::string password = Utils::GetString();
      m_Server = Aero::CreateAeroServer(AERO_PORT, password, factory);
    }
    else
      if (ch == 'p' || ch == 'P')
      {
        printf("\nEnter your name: ");
        const std::string name = Utils::GetString();

        printf("Enter IP to connect client to('localhost' by default): ");
        const std::string host = Utils::GetString();

        printf("Enter password to connect(empty by default): ");
        const std::string password = Utils::GetString();
        m_Client.reset(new Aero::AeroGLClient(name, host.empty() ? "localhost" : host, password, factory));
      }
  }

  AeroLauncher::~AeroLauncher()
  {
  }
  //-----------------------------------------------------------------------------------

  void AeroLauncher::Run()
  {
    if (m_Server.get())
    {
      m_Server->Run();
    }
    else
    if (m_Client.get())
    {
      m_Client->Run();
    }
  }
}
//-----------------------------------------------------------------------------------

namespace Test
{
  namespace
  {
    static const int NUM_CLIENTS = 10;
    const int MAX_CLIENTS = 400;
    const unsigned short SERVER_PORT = 1234;

    class ConsoleClientHandler : public Network::INetClientHandler
    {
    public:
      virtual void OnConnected(const Network::NetServerInfo& info) {}
      virtual void OnDisconnected() {}
      virtual void OnData(unsigned char* data, int size, Network::NetTime time) {}
    };

    class ServerHandler : public Network::INetServerHandler
    {
    public:
      virtual void OnClientConnected(const Network::NetClientInfo& info) {};
      virtual void OnClientDisconnected(const Network::NetClientInfo& info) {};
      virtual void OnData(Network::NetId id, unsigned char* data, int size, Network::NetTime time) {};
    };
  }
  //-----------------------------------------------------------------------------------

  ServerClientTestLauncher::ServerClientTestLauncher(Network::INetPeerFactory& factory)
    : m_Factory(factory)
    , m_Mode(0)
  {
    printf("Welcome to simple network test!\n");

    printf("Run as (S)erver or (C)lient or (B)oth?");
    char ch = GetChar();
    static char *defaultRemoteIP = "127.0.0.1";
    std::string remoteIP;
    std::string password;
    std::string name;
    static char *localIP="127.0.0.1";
    printf("\n");

    ConsoleClientHandler clientHandler;
    ServerHandler serverHandler;

    if (ch == 's' || ch == 'S')
    {
      m_Server = Network::CreateNetServer(m_Factory, serverHandler);
      m_Mode = 0;
    }
    else if (ch == 'c' || ch == 'C')
    {
      std::auto_ptr<Network::INetClient> client = Network::CreateNetClient(m_Factory, clientHandler);
      m_Clients.push_back(ClientPtr(client.release()));
      m_Mode = 1;
    }
    else if (ch == 'b' || ch == 'B')
    {
      m_Mode = 2;
      remoteIP = localIP;
      m_Server = Network::CreateNetServer(m_Factory, serverHandler);
      for (int i = 0; i < NUM_CLIENTS; ++i)
      {
        std::auto_ptr<Network::INetClient> client = Network::CreateNetClient(m_Factory, clientHandler);
        m_Clients.push_back(ClientPtr(client.release()));
      }
    }
    printf("\n");

    if (m_Mode == 0 || m_Mode==2)
    {
      if (m_Mode == 0)
      {
        printf("Enter password for this server: ");
        password = Utils::GetString();
      }
      m_Server->Start(SERVER_PORT, Network::NetServerInfo("Test server v.1.0.", MAX_CLIENTS, !password.empty()), password);
      printf("Started server\n");
    }

    if (m_Mode == 1 || m_Mode == 2)
    {
      if (m_Mode == 1)
      {
        printf("Enter your name: ");
        name = Utils::GetString();

        printf("Enter IP to connect client to: ");
        remoteIP = Utils::GetString();

        printf("Enter password to connect: ");
        password = Utils::GetString();
      }
      if (remoteIP[0]==0)
      {
        remoteIP = defaultRemoteIP;
        printf("Using %s\n", defaultRemoteIP);
      }

      printf("Starting clients..\n");
      for (Clients::iterator it = m_Clients.begin(); it != m_Clients.end(); ++it)
      {
        ClientPtr client = *it;
        client->Startup(Network::NetClientInfo(name));
      }
      for (Clients::iterator it = m_Clients.begin(); it != m_Clients.end(); ++it)
      {
        ClientPtr client = *it;
        client->Connect(remoteIP.c_str(), SERVER_PORT, password);
      }
      printf("Done.\n");
    }
  }
  //-----------------------------------------------------------------------------------

  ServerClientTestLauncher::~ServerClientTestLauncher()
  {
    if (m_Server.get())
    {
      m_Server->Shutdown();
    }

    for (Clients::iterator it = m_Clients.begin(); it != m_Clients.end(); ++it)
    {
      ClientPtr client = *it;
      client->Shutdown();
    }

    printf("Test completed");
  }
  //-----------------------------------------------------------------------------------

  void ServerClientTestLauncher::Run()
  {
    while (true)
    {
      if (KbHit())
      {
        const std::string command = Utils::GetString();

        if (command == "help")
        {
          if (m_Mode == 0)
          {
            printf("\nCommands list: \n\thelp - print this text.\n\tsend - send text message.\n\tstat - print connection statistics to file.\n\tkick - kick client.\n\tban - ban ip address.\n\tquit, exit - quit program.\n");
          }
          else if (m_Mode == 1)
          {
            printf("\nCommands list: \n\thelp - print this text.\n\tsend - send text message.\n\tstat - print connection statistics to file.\n\tquit, exit - quit program.\n");
          }
          continue;
        }

        if (command == "stat")
        {
          if (m_Server.get())
          {
            m_Server->PrintStat();
          }
          if (m_Mode == 1)
          {
            m_Clients.front()->PrintStat();
          }
          continue;
        }	

        if (command == "quit" || command == "exit")
        {
          break;
        }

        if (command == "send")
        {
          printf("Enter message to be sent: ");
          const std::string ms = Utils::GetString();

          if (m_Server.get())//server
          {
            printf("\nEnter client name to send message to: ");
            const std::string nameStr = Utils::GetString();
            m_Server->SendStringMessage(nameStr, ms);
          }
          if (m_Mode == 1)//client
          {
            m_Clients.front()->SendStringMessage(ms);
          }

          continue;
        }

        if (command == "kick")
        {
          if (m_Server.get())//server
          {
            printf("\nEnter client name to kick: ");
            const std::string nameStr = Utils::GetString();
            m_Server->Kick(nameStr);
            continue;
          }
        }

        if (command == "ban")
        {
          if (m_Server.get())//server
          {
            printf("\nEnter ip address to ban: ");
            const std::string nameStr = Utils::GetString();
            printf("\nEnter time(ms) to ban: ");
            const std::string timeStr = Utils::GetString();
            unsigned int time = atoi(timeStr.c_str());
            m_Server->Ban(nameStr, time);
            continue;
          }
        }

        printf("Unknown command: %s\n", command.c_str());
      }

#if defined(WIN32) || defined(WINNT)
      Sleep(30);
#else
      usleep(30 * 1000);
#endif
    }
  }
  //-----------------------------------------------------------------------------------
}
