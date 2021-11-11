#ifndef NUMBERS_TYPES_H
#define NUMBERS_TYPES_H

#include "../Network/Types.h"
#include <string>

namespace Numbers
{
  enum
  {
    MSG_RESET = Network::ID_LAST, //reset state to initial
    MSG_NEW_MOVE,
    MSG_CLIENT_MOVE, //client makes a specific move
    MSG_MOVE_RESULT, //result of client's move
    MSG_CLIENT_CONNECTED,
    MSG_CLIENT_DISCONNECTED,
    MSG_NOT_ENOUGH_CLIENTS,
    MSG_CHAT_MESSAGE,
  };

  enum
  {
    MR_WIN,
    MR_GREATER,
    MR_LESS,
    MR_LOOSE,
  };

  struct MoveResult
  {
    MoveResult(const std::string& name, int number, int mr)
      : m_Name(name)
      , m_Number(number)
      , m_Result(mr)
    {}
    std::string m_Name;
    int m_Number;
    int m_Result;
  };
}


#endif //NUMBERS_TYPES_H