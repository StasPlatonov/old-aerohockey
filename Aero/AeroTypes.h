#ifndef AERO_TYPES_H
#define AERO_TYPES_H

#include "Network/Types.h"
#include "Network/INetwork.h"

#include <string>
#include <deque>
#include <list>

namespace Aero
{
  enum
  {
    MSG_CLIENT_CONNECTED = Network::ID_LAST, //reset state to initial
    MSG_CLIENT_DISCONNECTED,
    MSG_CURRENT_WORLD_STATE,
    MSG_SYNCHRONIZE,
    MSG_CLIENT_CHANGED,
    //MSG_CLIENT_FORCE,
    MSG_CLIENT_SHOOT_REQ,
    MSG_CLIENT_SHOOT_RESP,

    MSG_CLIENT_INPUT,
    MSG_CLIENT_INPUT_CONFIRM,
  };

  struct DynamicClientData
  {
    Network::NetId m_Id;
    float m_X;
    float m_Y;
    float m_VX;
    float m_VY;
  };
  typedef std::vector<DynamicClientData> DynamicClientDatas;
  typedef std::map<Network::NetId, DynamicClientData> DynamicClientDataMap;

  struct PlayerState
  {
    PlayerState()
      : m_X(0), m_Y(0), m_VX(0), m_VY(0), m_VA(0)
    {}

    bool operator ==(const PlayerState& other) const
    {
      return m_X == other.m_X && m_Y == other.m_X && m_VX == other.m_VX && m_VY == other.m_VY && m_VA == other.m_VA;
    }

    bool operator !=(const PlayerState& other) const
    {
      return !(*this == other);
    }

    float m_X;//position
    float m_Y;
    float m_VX;//linear velocity
    float m_VY;
    float m_VA;//angular velocity
  };

  struct InputState
  {
    InputState()
      : m_X(0), m_Y(0), m_Buttons(0)
      , m_Left(false), m_Right(false), m_Up(false), m_Down(false)
    {}

    int m_X;
    int m_Y;

    enum
    {
      M_LEFT    = 0x01,
      M_RIGHT   = 0x02,
      M_MIDDLE  = 0x04
    };

    bool operator ==(const InputState& other) const
    {
      return m_Buttons == other.m_Buttons && m_Left == other.m_Left && m_Right == other.m_Right && m_Up == other.m_Up && m_Down == other.m_Down;
    }

    bool operator !=(const InputState& other) const
    {
      return !(*this == other);
    }

    char m_Buttons;

    bool m_Left;
    bool m_Right;
    bool m_Up;
    bool m_Down;
  };

  struct PlayerMove
  {
    unsigned int m_Time;
    PlayerState m_State;
    InputState m_Input;
  };
  typedef CircularBuffer<PlayerMove> Moves;

  struct InputEvent
  {
    unsigned int m_Time;
    InputState m_Input;
    Moves m_ImportantMoves;
  };

  typedef PlayerMove SyncEvent;

  struct NetMessage
  {
    NetMessage() : Id(0), Time(0) {}
    NetMessage(unsigned char id, Network::NetTime time, const unsigned char* data, int dataSize)
     : Id(id)
     , Time(time)
     , Data(data, data + dataSize)
    {}

    unsigned char Id;
    Network::NetTime Time;
    Bytes Data;
  };
  typedef std::list<NetMessage> NetMessages;
}

#endif //AERO_TYPES_H