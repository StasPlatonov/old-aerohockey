#ifndef AERO_PHYS_WORLD_H
#define AERO_PHYS_WORLD_H

#include "Physics/PhysWorld.h"

#include <memory.h>
#include <boost/shared_ptr.hpp>

#include <list>
#include <map>

class AeroPhysWorld : public PhysWorld
{
public:
  typedef boost::shared_ptr<AeroPhysWorld> Ptr;

	enum
	{
		e_columnCount = 1,
		e_rowCount = 2
	};

	explicit AeroPhysWorld(IRender::Ptr render)
    : PhysWorld(render)
	{
		{
      // Level
			b2BodyDef bd;
			b2Body* ground = m_Box2DWorld->CreateBody(&bd);

			b2EdgeShape shape;

  		shape.Set(b2Vec2(-20.0f, 0.0f), b2Vec2(20.0f, 0.0f));
			ground->CreateFixture(&shape, 0.0f);
      shape.Set(b2Vec2(-20.0f, 40.0f), b2Vec2(20.0f, 40.0f));
			ground->CreateFixture(&shape, 0.0f);

			shape.Set(b2Vec2(20.0f, 0.0f), b2Vec2(20.0f, 40.0f));
			ground->CreateFixture(&shape, 0.0f);
      shape.Set(b2Vec2(-20.0f, 0.0f), b2Vec2(-20.0f, 40.0f));
			ground->CreateFixture(&shape, 0.0f);
		}

		float32 xs[5] = {0.0f, -10.0f, -5.0f, 5.0f, 10.0f};
/*
    { // Ball
      b2CircleShape shape;
	  	shape.m_radius = 1.0f;

  		b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(0.0, 25.0f);
      bd.bullet = true;

      b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 1.0f;
			fd.restitution = 0.8f;

			m_sphere = m_Box2DWorld->CreateBody(&bd);

			m_sphere->CreateFixture(&fd);

			m_sphere->SetLinearVelocity(b2Vec2(0.0f, -0.0f));
		}
    */
		m_bullet = NULL;
	}

  b2Body* CreatePlayerBody()
  {
    b2PolygonShape shape;
    shape.SetAsBox(2.0f, 2.0f);

    b2FixtureDef fd;
    fd.shape = &shape;
    //fd.density = 0.0f;
    //fd.friction = 0.2f;
    fd.restitution = 0.3f;

    b2BodyDef bd;
    bd.type = b2_dynamicBody;
    //bd.fixedRotation = true;
    bd.linearDamping = 1.0f;
    bd.gravityScale = 0.0f;

    bd.position.Set(0,0);//p.m_X, p.m_Y);
    b2Body* body = m_Box2DWorld->CreateBody(&bd);

    body->CreateFixture(&fd);

    return body;
  }

  b2Body* CreateBulletBody()
  {
    b2PolygonShape shape;
    shape.SetAsBox(0.5f, 0.5f);

    //b2FixtureDef fd;
    //fd.shape = &shape;
    //fd.density = 0.0f;
    //fd.friction = 0.2f;
    //fd.restitution = 0.01f;

    b2BodyDef bd;
    bd.type = b2_dynamicBody;
    bd.bullet = true;
    //bd.fixedRotation = true;
    bd.linearDamping = 0.0f;
    bd.gravityScale = 0.0f;
    bd.position.Set(0,0);
    b2Body* body = m_Box2DWorld->CreateBody(&bd);

    body->CreateFixture(&shape, 200.0f);

    return body;
  }

  void DestroyBody(b2Body* b)
  {
    m_Box2DWorld->DestroyBody(b);
  }

  void SetPosition(b2Body* body, int x, int y)
  {
    body->SetTransform(b2Vec2(x, y), 0);
  }

	void Keyboard(unsigned char key)
	{
		switch (key)
		{
		case ',':
			if (m_bullet != NULL)
			{
				m_Box2DWorld->DestroyBody(m_bullet);
				m_bullet = NULL;
			}

			{
				b2CircleShape shape;
				shape.m_radius = 0.25f;

				b2FixtureDef fd;
				fd.shape = &shape;
				fd.density = 20.0f;
				fd.restitution = 0.05f;

				b2BodyDef bd;
				bd.type = b2_dynamicBody;
				bd.bullet = true;
				bd.position.Set(-31.0f, 5.0f);

				m_bullet = m_Box2DWorld->CreateBody(&bd);
				m_bullet->CreateFixture(&fd);

				m_bullet->SetLinearVelocity(b2Vec2(400.0f, 0.0f));
			}
			break;
		}
	}

	void Step(Settings& settings)
	{
		PhysWorld::Step(settings);
    /*
    typedef std::list<b2Body*> bodies;
    bodies deads;

    for (int32 i = 0; i < m_pointCount; ++i)
		{
			ContactPoint* point = m_points + i;

			b2Body* body1 = point->fixtureA->GetBody();
			b2Body* body2 = point->fixtureB->GetBody();
      if (body1->GetType() == b2_dynamicBody && body2->GetType() == b2_dynamicBody)
      {
        deads.push_back(body1);
        deads.push_back(body2);
      }
      //float32 mass1 = body1->GetMass();
			//float32 mass2 = body2->GetMass();
    }
    deads.sort();

    while (!deads.empty())
    {
      m_Box2DWorld->DestroyBody(deads.front());
      deads.pop_front();
    }*/
	}

	static std::auto_ptr<AeroPhysWorld> Create(IRender::Ptr render)
	{
		return std::auto_ptr<AeroPhysWorld>(new AeroPhysWorld(render));
	}

	b2Body* m_bullet;
  b2Body* m_sphere;
};

#endif //AERO_PHYS_WORLD_H
