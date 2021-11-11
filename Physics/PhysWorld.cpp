#include "PhysWorld.h"
#include <cstdio>
using namespace std;

#if defined (_WIN32)
    #pragma comment(lib, "Box2D.lib")
#endif

DestructionListener::DestructionListener(PhysWorld& pw)
  : m_World(pw)
{
}

void DestructionListener::SayGoodbye(b2Joint* joint)
{
	if (m_World.m_mouseJoint == joint)
	{
		m_World.m_mouseJoint = NULL;
	}
	else
	{
		m_World.JointDestroyed(joint);
	}
}

PhysWorld::PhysWorld(IRender::Ptr render)
  : m_Render(render)
  , m_destructionListener(*this)
{
	b2Vec2 gravity;
	gravity.Set(0.0f, -10.0f);
	m_Box2DWorld.reset(new b2World(gravity));
	m_bomb = NULL;
	m_textLine = 30;
	m_mouseJoint = NULL;
	m_pointCount = 0;

	m_Box2DWorld->SetDestructionListener(&m_destructionListener);
	m_Box2DWorld->SetContactListener(this);
	m_Box2DWorld->SetDebugDraw(m_Render.get());
	
	m_bombSpawning = false;

	m_stepCount = 0;

	b2BodyDef bodyDef;
	m_groundBody = m_Box2DWorld->CreateBody(&bodyDef);

	//memset(&m_maxProfile, 0, sizeof(b2Profile));
	//memset(&m_totalProfile, 0, sizeof(b2Profile));
}

PhysWorld::~PhysWorld()
{
}

void PhysWorld::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
	const b2Manifold* manifold = contact->GetManifold();

	if (manifold->pointCount == 0)
	{
		return;
	}

	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	b2PointState state1[b2_maxManifoldPoints], state2[b2_maxManifoldPoints];
	b2GetPointStates(state1, state2, oldManifold, manifold);

	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);

	for (int32 i = 0; i < manifold->pointCount && m_pointCount < k_maxContactPoints; ++i)
	{
		ContactPoint* cp = m_points + m_pointCount;
		cp->fixtureA = fixtureA;
		cp->fixtureB = fixtureB;
		cp->position = worldManifold.points[i];
		cp->normal = worldManifold.normal;
		cp->state = state2[i];
		++m_pointCount;
	}
}

void PhysWorld::DrawTitle(int x, int y, const char *string)
{
    m_Render->DrawString(x, y, string);
}

class QueryCallback : public b2QueryCallback
{
public:
	QueryCallback(const b2Vec2& point)
	{
		m_point = point;
		m_fixture = NULL;
	}

	bool ReportFixture(b2Fixture* fixture)
	{
		b2Body* body = fixture->GetBody();
		if (body->GetType() == b2_dynamicBody)
		{
			bool inside = fixture->TestPoint(m_point);
			if (inside)
			{
				m_fixture = fixture;

				// We are done, terminate the query.
				return false;
			}
		}

		// Continue the query.
		return true;
	}

	b2Vec2 m_point;
	b2Fixture* m_fixture;
};

b2Body* PhysWorld::Pick(const b2Vec2& p)
{
  b2AABB aabb;
	b2Vec2 d;
	d.Set(0.001f, 0.001f);
	aabb.lowerBound = p - d;
	aabb.upperBound = p + d;

	// Query the world for overlapping shapes.
	QueryCallback callback(p);
	m_Box2DWorld->QueryAABB(&callback, aabb);

	if (callback.m_fixture)
	{
		return callback.m_fixture->GetBody();
  }
  return 0;
}

void PhysWorld::MouseDown(const b2Vec2& p)
{
	m_mouseWorld = p;
	
	if (m_mouseJoint != NULL)
	{
		return;
	}

	// Make a small box.
	b2AABB aabb;
	b2Vec2 d;
	d.Set(0.001f, 0.001f);
	aabb.lowerBound = p - d;
	aabb.upperBound = p + d;

	// Query the world for overlapping shapes.
	QueryCallback callback(p);
	m_Box2DWorld->QueryAABB(&callback, aabb);

	if (callback.m_fixture)
	{
    return;
		b2Body* body = callback.m_fixture->GetBody();
		b2MouseJointDef md;
		md.bodyA = m_groundBody;
		md.bodyB = body;
		md.target = p;
		md.maxForce = 1000.0f * body->GetMass();
		m_mouseJoint = (b2MouseJoint*)m_Box2DWorld->CreateJoint(&md);
		body->SetAwake(true);
	}
}

void PhysWorld::MouseUp(const b2Vec2& p)
{
	if (m_mouseJoint)
	{
		m_Box2DWorld->DestroyJoint(m_mouseJoint);
		m_mouseJoint = NULL;
	}
	
	if (m_bombSpawning)
	{
		CompleteBombSpawn(p);
	}
}

void PhysWorld::MouseMove(const b2Vec2& p)
{
	m_mouseWorld = p;
	
	if (m_mouseJoint)
	{
		m_mouseJoint->SetTarget(p);
	}
}

void PhysWorld::SpawnBomb(const b2Vec2& worldPt)
{
	m_bombSpawnPoint = worldPt;
	m_bombSpawning = true;
}
    
void PhysWorld::CompleteBombSpawn(const b2Vec2& p)
{
	if (m_bombSpawning == false)
	{
		return;
	}

	const float multiplier = 30.0f;
	b2Vec2 vel = m_bombSpawnPoint - p;
	vel *= multiplier;
	LaunchBomb(m_bombSpawnPoint,vel);
	m_bombSpawning = false;
}

void PhysWorld::ShiftMouseDown(const b2Vec2& p)
{
	m_mouseWorld = p;
	
	if (m_mouseJoint != NULL)
	{
		return;
	}

	SpawnBomb(p);
}

void PhysWorld::LaunchBomb()
{
	b2Vec2 p(RandomFloat(-15.0f, 15.0f), 30.0f);
	b2Vec2 v = -5.0f * p;
	LaunchBomb(p, v);
}

void PhysWorld::LaunchBomb(const b2Vec2& position, const b2Vec2& velocity)
{
	if (m_bomb)
	{
		m_Box2DWorld->DestroyBody(m_bomb);
		m_bomb = NULL;
	}

	b2BodyDef bd;
	bd.type = b2_dynamicBody;
	bd.position = position;
	bd.bullet = true;
	m_bomb = m_Box2DWorld->CreateBody(&bd);
	m_bomb->SetLinearVelocity(velocity);
	
	b2CircleShape circle;
	circle.m_radius = 0.3f;

	b2FixtureDef fd;
	fd.shape = &circle;
	fd.density = 20.0f;
	fd.restitution = 0.0f;
	
	b2Vec2 minV = position - b2Vec2(0.3f,0.3f);
	b2Vec2 maxV = position + b2Vec2(0.3f,0.3f);
	
	b2AABB aabb;
	aabb.lowerBound = minV;
	aabb.upperBound = maxV;

	m_bomb->CreateFixture(&fd);
}

void PhysWorld::DebugDraw(const Settings& settings)
{
  if (settings.pause)
	{
    m_Render->DrawString(5, m_textLine, "****PAUSED****");
		m_textLine += 15;
  }
  uint32 flags = 0;

	flags += settings.drawShapes			* b2Draw::e_shapeBit;
	flags += settings.drawJoints			* b2Draw::e_jointBit;
	flags += settings.drawAABBs			* b2Draw::e_aabbBit;
	flags += settings.drawPairs			* b2Draw::e_pairBit;
	flags += settings.drawCOMs				* b2Draw::e_centerOfMassBit;

	m_Render->SetFlags(flags);

  m_Box2DWorld->DrawDebugData();

  if (settings.drawStats)
	{
		int32 bodyCount = m_Box2DWorld->GetBodyCount();
		int32 contactCount = m_Box2DWorld->GetContactCount();
		int32 jointCount = m_Box2DWorld->GetJointCount();
		m_Render->DrawString(5, m_textLine, "bodies/contacts/joints = %d/%d/%d", bodyCount, contactCount, jointCount);
		m_textLine += 15;


		int32 proxyCount = m_Box2DWorld->GetProxyCount();
		int32 height = m_Box2DWorld->GetTreeHeight();
		int32 balance = m_Box2DWorld->GetTreeBalance();
		float32 quality = m_Box2DWorld->GetTreeQuality();
		m_Render->DrawString(5, m_textLine, "proxies/height/balance/quality = %d/%d/%d/%g", proxyCount, height, balance, quality);
		m_textLine += 15;

	}

  if (settings.drawProfile)
	{
		const b2Profile& p = m_Box2DWorld->GetProfile();

		b2Profile aveProfile;
		memset(&aveProfile, 0, sizeof(b2Profile));
		if (m_stepCount > 0)
		{
			float32 scale = 1.0f / m_stepCount;
			aveProfile.step = scale * m_totalProfile.step;
			aveProfile.collide = scale * m_totalProfile.collide;
			aveProfile.solve = scale * m_totalProfile.solve;
			aveProfile.solveInit = scale * m_totalProfile.solveInit;
			aveProfile.solveVelocity = scale * m_totalProfile.solveVelocity;
			aveProfile.solvePosition = scale * m_totalProfile.solvePosition;
			aveProfile.solveTOI = scale * m_totalProfile.solveTOI;
			aveProfile.broadphase = scale * m_totalProfile.broadphase;
		}

		m_Render->DrawString(5, m_textLine, "step [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.step, aveProfile.step, m_maxProfile.step);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "collide [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.collide, aveProfile.collide, m_maxProfile.collide);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "solve [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solve, aveProfile.solve, m_maxProfile.solve);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "solve init [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveInit, aveProfile.solveInit, m_maxProfile.solveInit);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "solve velocity [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveVelocity, aveProfile.solveVelocity, m_maxProfile.solveVelocity);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "solve position [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solvePosition, aveProfile.solvePosition, m_maxProfile.solvePosition);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "solveTOI [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveTOI, aveProfile.solveTOI, m_maxProfile.solveTOI);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "broad-phase [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.broadphase, aveProfile.broadphase, m_maxProfile.broadphase);
		m_textLine += 15;
	}


  if (m_mouseJoint)
	{
		b2Vec2 p1 = m_mouseJoint->GetAnchorB();
		b2Vec2 p2 = m_mouseJoint->GetTarget();

		b2Color c;
		c.Set(0.0f, 1.0f, 0.0f);
		m_Render->DrawPoint(p1, 4.0f, c);
		m_Render->DrawPoint(p2, 4.0f, c);

		c.Set(0.8f, 0.8f, 0.8f);
		m_Render->DrawSegment(p1, p2, c);
	}
	
	if (m_bombSpawning)
	{
		b2Color c;
		c.Set(0.0f, 0.0f, 1.0f);
		m_Render->DrawPoint(m_bombSpawnPoint, 4.0f, c);

		c.Set(0.8f, 0.8f, 0.8f);
		m_Render->DrawSegment(m_mouseWorld, m_bombSpawnPoint, c);
	}

	if (settings.drawContactPoints)
	{
		//const float32 k_impulseScale = 0.1f;
		const float32 k_axisScale = 0.3f;

		for (int32 i = 0; i < m_pointCount; ++i)
		{
			ContactPoint* point = m_points + i;

			if (point->state == b2_addState)
			{
				// Add
				m_Render->DrawPoint(point->position, 10.0f, b2Color(0.3f, 0.95f, 0.3f));
			}
			else if (point->state == b2_persistState)
			{
				// Persist
				m_Render->DrawPoint(point->position, 5.0f, b2Color(0.3f, 0.3f, 0.95f));
			}

			if (settings.drawContactNormals == 1)
			{
				b2Vec2 p1 = point->position;
				b2Vec2 p2 = p1 + k_axisScale * point->normal;
				m_Render->DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.9f));
			}
			else if (settings.drawContactForces == 1)
			{
				//b2Vec2 p1 = point->position;
				//b2Vec2 p2 = p1 + k_forceScale * point->normalForce * point->normal;
				//DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.3f));
			}

			if (settings.drawFrictionForces == 1)
			{
				//b2Vec2 tangent = b2Cross(point->normal, 1.0f);
				//b2Vec2 p1 = point->position;
				//b2Vec2 p2 = p1 + k_forceScale * point->tangentForce * tangent;
				//DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.3f));
			}
		}
	}
}
//--------------------------------------------------------------------------------------------

void PhysWorld::Step(Settings& settings)
{
	float32 timeStep = settings.hz > 0.0f ? 1.0f / settings.hz : float32(0.0f);

	if (settings.pause)
	{
		if (settings.singleStep)
		{
			settings.singleStep = 0;
		}
		else
		{
			timeStep = 0.0f;
		}

		//m_Render->DrawString(5, m_textLine, "****PAUSED****");
		m_textLine += 15;
	}

	/*uint32 flags = 0;
	flags += settings->drawShapes			* b2Draw::e_shapeBit;
	flags += settings->drawJoints			* b2Draw::e_jointBit;
	flags += settings->drawAABBs			* b2Draw::e_aabbBit;
	flags += settings->drawPairs			* b2Draw::e_pairBit;
	flags += settings->drawCOMs				* b2Draw::e_centerOfMassBit;
	m_Render->SetFlags(flags);*/

	m_Box2DWorld->SetWarmStarting(settings.enableWarmStarting > 0);
	m_Box2DWorld->SetContinuousPhysics(settings.enableContinuous > 0);
	m_Box2DWorld->SetSubStepping(settings.enableSubStepping > 0);
	m_pointCount = 0;

	m_Box2DWorld->Step(timeStep, settings.velocityIterations, settings.positionIterations);

	//m_Box2DWorld->DrawDebugData();

	if (timeStep > 0.0f)
	{
		++m_stepCount;
	}
  /*
	if (settings->drawStats)
	{
		int32 bodyCount = m_Box2DWorld->GetBodyCount();
		int32 contactCount = m_Box2DWorld->GetContactCount();
		int32 jointCount = m_Box2DWorld->GetJointCount();
		m_Render->DrawString(5, m_textLine, "bodies/contacts/joints = %d/%d/%d", bodyCount, contactCount, jointCount);
		m_textLine += 15;

		int32 proxyCount = m_Box2DWorld->GetProxyCount();
		int32 height = m_Box2DWorld->GetTreeHeight();
		int32 balance = m_Box2DWorld->GetTreeBalance();
		float32 quality = m_Box2DWorld->GetTreeQuality();
		m_Render->DrawString(5, m_textLine, "proxies/height/balance/quality = %d/%d/%d/%g", proxyCount, height, balance, quality);
		m_textLine += 15;
	}*/

	// Track maximum profile times
	{

		const b2Profile& p = m_Box2DWorld->GetProfile();
		m_maxProfile.step = b2Max(m_maxProfile.step, p.step);
		m_maxProfile.collide = b2Max(m_maxProfile.collide, p.collide);
		m_maxProfile.solve = b2Max(m_maxProfile.solve, p.solve);
		m_maxProfile.solveInit = b2Max(m_maxProfile.solveInit, p.solveInit);
		m_maxProfile.solveVelocity = b2Max(m_maxProfile.solveVelocity, p.solveVelocity);
		m_maxProfile.solvePosition = b2Max(m_maxProfile.solvePosition, p.solvePosition);
		m_maxProfile.solveTOI = b2Max(m_maxProfile.solveTOI, p.solveTOI);
		m_maxProfile.broadphase = b2Max(m_maxProfile.broadphase, p.broadphase);

		m_totalProfile.step += p.step;
		m_totalProfile.collide += p.collide;
		m_totalProfile.solve += p.solve;
		m_totalProfile.solveInit += p.solveInit;
		m_totalProfile.solveVelocity += p.solveVelocity;
		m_totalProfile.solvePosition += p.solvePosition;
		m_totalProfile.solveTOI += p.solveTOI;
		m_totalProfile.broadphase += p.broadphase;

	}

	/*if (settings->drawProfile)
	{
		const b2Profile& p = m_Box2DWorld->GetProfile();

		b2Profile aveProfile;
		memset(&aveProfile, 0, sizeof(b2Profile));
		if (m_stepCount > 0)
		{
			float32 scale = 1.0f / m_stepCount;
			aveProfile.step = scale * m_totalProfile.step;
			aveProfile.collide = scale * m_totalProfile.collide;
			aveProfile.solve = scale * m_totalProfile.solve;
			aveProfile.solveInit = scale * m_totalProfile.solveInit;
			aveProfile.solveVelocity = scale * m_totalProfile.solveVelocity;
			aveProfile.solvePosition = scale * m_totalProfile.solvePosition;
			aveProfile.solveTOI = scale * m_totalProfile.solveTOI;
			aveProfile.broadphase = scale * m_totalProfile.broadphase;
		}

		m_Render->DrawString(5, m_textLine, "step [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.step, aveProfile.step, m_maxProfile.step);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "collide [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.collide, aveProfile.collide, m_maxProfile.collide);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "solve [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solve, aveProfile.solve, m_maxProfile.solve);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "solve init [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveInit, aveProfile.solveInit, m_maxProfile.solveInit);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "solve velocity [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveVelocity, aveProfile.solveVelocity, m_maxProfile.solveVelocity);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "solve position [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solvePosition, aveProfile.solvePosition, m_maxProfile.solvePosition);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "solveTOI [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveTOI, aveProfile.solveTOI, m_maxProfile.solveTOI);
		m_textLine += 15;
		m_Render->DrawString(5, m_textLine, "broad-phase [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.broadphase, aveProfile.broadphase, m_maxProfile.broadphase);
		m_textLine += 15;
	}*/

	//if (m_mouseJoint)
	//{
	//	b2Vec2 p1 = m_mouseJoint->GetAnchorB();
	//	b2Vec2 p2 = m_mouseJoint->GetTarget();

	//	b2Color c;
	//	c.Set(0.0f, 1.0f, 0.0f);
	//	m_Render->DrawPoint(p1, 4.0f, c);
	//	m_Render->DrawPoint(p2, 4.0f, c);

	//	c.Set(0.8f, 0.8f, 0.8f);
	//	m_Render->DrawSegment(p1, p2, c);
	//}
	//
	//if (m_bombSpawning)
	//{
	//	b2Color c;
	//	c.Set(0.0f, 0.0f, 1.0f);
	//	m_Render->DrawPoint(m_bombSpawnPoint, 4.0f, c);

	//	c.Set(0.8f, 0.8f, 0.8f);
	//	m_Render->DrawSegment(m_mouseWorld, m_bombSpawnPoint, c);
	//}

	//if (settings->drawContactPoints)
	//{
	//	//const float32 k_impulseScale = 0.1f;
	//	const float32 k_axisScale = 0.3f;

	//	for (int32 i = 0; i < m_pointCount; ++i)
	//	{
	//		ContactPoint* point = m_points + i;

	//		if (point->state == b2_addState)
	//		{
	//			// Add
	//			m_Render->DrawPoint(point->position, 10.0f, b2Color(0.3f, 0.95f, 0.3f));
	//		}
	//		else if (point->state == b2_persistState)
	//		{
	//			// Persist
	//			m_Render->DrawPoint(point->position, 5.0f, b2Color(0.3f, 0.3f, 0.95f));
	//		}

	//		if (settings->drawContactNormals == 1)
	//		{
	//			b2Vec2 p1 = point->position;
	//			b2Vec2 p2 = p1 + k_axisScale * point->normal;
	//			m_Render->DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.9f));
	//		}
	//		else if (settings->drawContactForces == 1)
	//		{
	//			//b2Vec2 p1 = point->position;
	//			//b2Vec2 p2 = p1 + k_forceScale * point->normalForce * point->normal;
	//			//DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.3f));
	//		}

	//		if (settings->drawFrictionForces == 1)
	//		{
	//			//b2Vec2 tangent = b2Cross(point->normal, 1.0f);
	//			//b2Vec2 p1 = point->position;
	//			//b2Vec2 p2 = p1 + k_forceScale * point->tangentForce * tangent;
	//			//DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.3f));
	//		}
	//	}
	//}
}
