#ifndef AERO_PLAYER_H
#define AERO_PLAYER_H

#include "Network/INetwork.h"
#include "AeroTypes.h"
#include "AeroPhysWorld.h"

#include <vector>
#include <boost/shared_ptr.hpp>

namespace Aero
{
  struct Player
  {
    typedef boost::shared_ptr<Player> Ptr;

    Player(const Network::NetClientInfo& info, AeroPhysWorld::Ptr phys);
    ~Player();

    void BeginMove() { m_Moving = true; }
    void EndMove() { m_Moving = false; }
    bool IsMoving() const { return m_Moving; }

    bool IsAwake() const;

    void SetDesiredState(float x, float y, float vx, float vy);
    
    void SetPosition(const b2Vec2&);
    void SetPosition(float x, float y);
    void SetVelocity(float x, float y);
    void Impulse(float x, float y);
    void Force(float x, float y);
    b2Vec2 GetPosition() const;

    void GetState(PlayerState& state) const;
    void SetState(const PlayerState& state);

    void Update(unsigned timeDelta);

    void ProcessInput(const InputState& i);

    Network::NetClientInfo m_NetInfo;
    AeroPhysWorld::Ptr m_Phys;
    b2Body* m_Body;
    bool m_Moving;
    mutable b2Vec2 m_LastGetPosition;

    b2Vec2 m_LastPos;
    b2Vec2 m_LastVel;
    b2Vec2 m_LastAckPos;
    b2Vec2 m_LastAckVel;
    int m_SmoothTime;
  };
  typedef std::vector<Player::Ptr> Players;

  class PlayerFinder : public std::unary_function<Player::Ptr, bool>
  {
  public:
    PlayerFinder(Network::NetId id)
      : m_Id(id)
    {}

    bool operator()(const Player::Ptr& other)
    {
      return (other->m_NetInfo.m_Id == m_Id);
    }

  private:
    Network::NetId m_Id;
  };

  struct Bullet
  {
    typedef boost::shared_ptr<Bullet> Ptr;

    Bullet(int x, int y, int vx, int vy, AeroPhysWorld::Ptr phys);
    ~Bullet();

    AeroPhysWorld::Ptr m_Phys;
    b2Body* m_Body;
    int m_LifeTime;
  };
  typedef std::vector<Bullet::Ptr> Bullets;
}

#endif