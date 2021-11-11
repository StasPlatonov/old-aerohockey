#include "NumbersConsoleClient.h"
#include "Common/Utils.h"

#include <conio.h>
#include <ctime>

#if defined(WIN32) || defined(WINNT)
  #include <windows.h>
  #include <algorithm>
#endif

namespace
{
  const unsigned short AERO_PORT = 2345;
}

namespace Numbers
{
  namespace
  {
    bool IsNumber(const std::string& str)
    {
      std::string::const_iterator it = str.begin();
      while (it != str.end() && isdigit(*it)) ++it;
      return !str.empty() && it == str.end();
    }
  }
  //-----------------------------------------------------------------------------------

  NumbersConsoleClient::NumbersConsoleClient(const std::string& name, const std::string& host, const std::string& password, Network::INetPeerFactory& factory)
    : m_Client(Numbers::CreateNumbersClient(factory, *this))
    , m_Active(true)
    , m_MoveRequested(false)
  {
    m_Client->Init(name, host.c_str(), AERO_PORT, password);
  }

  NumbersConsoleClient::~NumbersConsoleClient()
  {
    m_Client->Done();
  }
  //-----------------------------------------------------------------------------------

  void NumbersConsoleClient::Move(int number)
  {
    m_Client->Move(number);
  }
  //-----------------------------------------------------------------------------------

  void NumbersConsoleClient::OnMessage(const std::string& text)
  {
    printf("%s", text.c_str());
  }
  //-----------------------------------------------------------------------------------

  void NumbersConsoleClient::OnError(const std::string& text)
  {
    printf("ERROR: %s", text.c_str());
  }
  //-----------------------------------------------------------------------------------

  void NumbersConsoleClient::OnMoveRequested()
  {
    m_MoveRequested = true;
  }
  //-----------------------------------------------------------------------------------

  void NumbersConsoleClient::OnReset()
  {
    printf("\n\n******Game restarted\n");
  }
  //-----------------------------------------------------------------------------------

  void NumbersConsoleClient::OnMoveResult(const MoveResult& mr)
  {
    char str[255] = {0};
    switch (mr.m_Result)
    {
    case Numbers::MR_WIN:
      if (m_Client->GetInfo().m_Name == mr.m_Name)
        sprintf(str, "You win! Congratulations, '%s'!\n", mr.m_Name.c_str());
      else
        sprintf(str, "Player '%s' win!\nNumber was: %d\n", mr.m_Name.c_str(), mr.m_Number);
      break;

    case Numbers::MR_LESS:
      if (m_Client->GetInfo().m_Name == mr.m_Name)
        sprintf(str, "You entered too small number.\n");
      else
        sprintf(str, "Player '%s' entered too small number %d\n", mr.m_Name.c_str(), mr.m_Number);
      break;

    case Numbers::MR_GREATER:
      if (m_Client->GetInfo().m_Name == mr.m_Name)
        sprintf(str, "You entered too large number.\n");
      else
        sprintf(str, "Player '%s' entered too large number %d\n", mr.m_Name.c_str(), mr.m_Number);
      break;

    case Numbers::MR_LOOSE:
      if (m_Client->GetInfo().m_Name == mr.m_Name)
        sprintf(str, "You're surrender! Too bad!\n");
      else
        sprintf(str, "Player '%s' gave up! Surrender!\n", mr.m_Name.c_str());
      break;
    }

    OnMessage(str);
  }
  //-----------------------------------------------------------------------------------

  void NumbersConsoleClient::Idle()
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
        m_Active = false;
      }
      else
      if (command == "cls")
      {
        system("cls");
      }
      else
      if (command.find("m ") == 0)
      {
        const std::string msg = command.substr(2, command.size() - 2);
        m_Client->SendMessage(msg);
      }
      else
      if (m_MoveRequested)
      {
        if (IsNumber(command))
        {
          unsigned int num = atoi(command.c_str());
          Move(num);
          m_MoveRequested = false;
        }
        else
        {
          printf("\nYou have to enter an integer value or a valid command\n");
        }
      }
      else
      {
        printf("\nYou have to enter a valid command\n");
      }
    }
  }

  void NumbersConsoleClient::Run()
  {
    while (IsActive())
    {
      Idle();
#if defined(WIN32) || defined(WINNT)
      Sleep(30);
#else
      usleep(30 * 1000);
#endif
    }
  }
  //-----------------------------------------------------------------------------------

  NumbersConsoleAIClient::NumbersConsoleAIClient(const std::string& name, const std::string& host, const std::string& password, Network::INetPeerFactory& factory)
    : NumbersConsoleClient(name, host, password, factory)
    , m_MoveRequestedEvent(true, false)
    , m_Min(0)
    , m_Max(100)
  {
  }

  NumbersConsoleAIClient::~NumbersConsoleAIClient()
  {
  }
  //-----------------------------------------------------------------------------------

  void NumbersConsoleAIClient::OnMoveRequested()
  {
    m_MoveRequestedEvent.Set();
  }
  //-----------------------------------------------------------------------------------

  bool NumbersConsoleAIClient::IsMoveRequested() const
  {
    return m_MoveRequestedEvent.IsSet();
  }

  void NumbersConsoleAIClient::Idle()
  {
    if (IsMoveRequested())
    {
      int answer = m_Min;

      const int numberOfAnswers = m_Max - m_Min + 1;
      if (numberOfAnswers % 2 == 0)//no middle answer
      {
        //choose floor or ceil with 50% of probability
        if (rand() % 2 == 0)
        {
          answer = m_Min + numberOfAnswers / 2;
        }
        else
        {
          answer = m_Min + numberOfAnswers / 2 + 1;
        }
      }
      else
      //choose exactly middle
      {
        answer = m_Min + (numberOfAnswers - 1) / 2;
      }

      printf("%d\n", answer);
      Move(answer);
      m_MoveRequestedEvent.Reset();
    }
  }
  //-----------------------------------------------------------------------------------
  
  void NumbersConsoleAIClient::OnMoveResult(const MoveResult& mr)
  {
    NumbersConsoleClient::OnMoveResult(mr);
    switch (mr.m_Result)
    {
      case Numbers::MR_WIN:
      case Numbers::MR_LOOSE:
        break;
      case Numbers::MR_LESS:
        m_Min = 
#if !defined(WIN32) && !defined(WINNT)          
          std::
#endif          
          max(m_Min, mr.m_Number + 1);
        break;
      case Numbers::MR_GREATER:
        m_Max = 
#if !defined(WIN32) && !defined(WINNT)          
          std::
#endif          
          min(m_Max, mr.m_Number - 1);
        break;
    }
    printf("Working interval: [%d...%d]\n", m_Min, m_Max);
  }
  //-----------------------------------------------------------------------------------

  void NumbersConsoleAIClient::OnReset()
  {
    NumbersConsoleClient::OnReset();
    m_Min = 0;
    m_Max = 100;
  }
  //-----------------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------------
