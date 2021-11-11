#include "AeroPlayer.h"

namespace Aero
{
  const unsigned int CL_SMOOTH_TIME = 200;
  //-----------------------------------------------------------------------------------

  Player::Player(const Network::NetClientInfo& info, AeroPhysWorld::Ptr phys)
    : m_NetInfo(info)
    , m_Phys(phys)
    , m_Body(phys->CreatePlayerBody())
    , m_Moving(false)
    , m_SmoothTime(0)
  {}

  Player::~Player()
  {
    m_Phys->DestroyBody(m_Body);
  }
  //-----------------------------------------------------------------------------------

  b2Vec2 Player::GetPosition() const
  {
    m_LastGetPosition = m_Body->GetPosition();

    return m_LastGetPosition;
  }

  void Player::SetPosition(const b2Vec2& pos)
  {
    SetPosition(pos.x, pos.y);
  }

  void Player::SetPosition(float x, float y)
  {
    if (m_Body->GetWorld()->IsLocked())
      return;

    m_Body->SetTransform(b2Vec2(x, y), 0);
  }
  //-----------------------------------------------------------------------------------

  void Player::SetVelocity(float x, float y)
  {
    if (m_Body->GetWorld()->IsLocked())
      return;

    m_Body->SetLinearVelocity(b2Vec2(x, y));
  }
  //-----------------------------------------------------------------------------------

  void Player::Impulse(float x, float y)
  {
    b2Vec2 dir = b2Vec2(x, y) - m_Body->GetPosition();
    dir.Normalize();
    dir *= 10;
    m_Body->ApplyLinearImpulse(dir, b2Vec2(0,0), true);
  }
  //-----------------------------------------------------------------------------------

  void Player::Force(float x, float y)
  {
    const b2Vec2 dir = b2Vec2(x, y);
	m_Body->ApplyForceToCenter(dir, true);
  }
  //-----------------------------------------------------------------------------------

  bool Player::IsAwake() const
  {
    return m_Body->IsAwake();
  }
  //-----------------------------------------------------------------------------------

  void Player::Update(unsigned/* timeDelta*/)
  {
    b2Vec2 posDiff = m_LastAckPos - GetPosition();
    b2Vec2 velDiff = m_LastAckVel - m_Body->GetLinearVelocity();
    float distApart = posDiff.Length();
    if (distApart && m_SmoothTime)
    {
      //printf("distApart: %.2f\n", distApart);
      if (distApart < 0.2f)
      {
        SetPosition(m_LastAckPos);
        SetVelocity(m_LastAckVel.x, m_LastAckVel.y);
        m_SmoothTime = 0;
      }
      else
      {
        SetPosition(GetPosition() + b2Vec2(posDiff.x * 0.1f, posDiff.y * 0.1f));

        b2Vec2 vel = m_Body->GetLinearVelocity() + b2Vec2(velDiff.x * 0.1f, velDiff.y * 0.1f);
        SetVelocity(vel.x, vel.y);
      }
    }
  }
  //-----------------------------------------------------------------------------------

  void Player::SetDesiredState(float x, float y, float vx, float vy)
  {
    b2Vec2 curPos = m_Body->GetPosition();
    if (curPos.x != x || curPos.y != y)
    {
      m_LastAckPos = b2Vec2(x, y);
      m_LastAckVel = b2Vec2(vx, vy);
      m_LastPos = curPos;
      m_LastVel = m_Body->GetLinearVelocity();
      //prediction error!
      m_SmoothTime = CL_SMOOTH_TIME;
    }
  }
  //-----------------------------------------------------------------------------------

  void Player::GetState(PlayerState& state) const
  {
    const b2Vec2 pos = GetPosition();
    state.m_X = pos.x;
    state.m_Y = pos.y;

    const b2Vec2 vel = m_Body->GetLinearVelocity();
    state.m_VX = vel.x;
    state.m_VY = vel.y;
    state.m_VA = 0;
  }
  //-----------------------------------------------------------------------------------

  void Player::SetState(const PlayerState& state)
  {
    SetPosition(state.m_X, state.m_Y);
    m_LastAckPos = b2Vec2(state.m_X, state.m_Y);
    SetVelocity(state.m_VX, state.m_VY);
    m_LastAckVel = b2Vec2(state.m_VX, state.m_VY);
  }
  //-----------------------------------------------------------------------------------

  void Player::ProcessInput(const InputState& i)
  {
    int forceValue = 10;
    int xf = 0, yf = 0;
    if (i.m_Left)
      xf += -1;
    if (i.m_Right)
      xf += 1;
    if (i.m_Up)
      yf += 1;
    if (i.m_Down)
      yf += -1;

    Force(xf * forceValue, yf * forceValue);
  }
  //-----------------------------------------------------------------------------------

  Bullet::Bullet(int x, int y, int vx, int vy, AeroPhysWorld::Ptr phys)
    : m_Phys(phys)
    , m_Body(phys->CreateBulletBody())
    , m_LifeTime(200)
  {
    m_Body->SetTransform(b2Vec2(x, y), 0);
    m_Body->SetLinearVelocity(b2Vec2(vx, vy));
  }

  Bullet::~Bullet()
  {
    m_Phys->DestroyBody(m_Body);
  }
}
