#ifndef IRENDER_H
#define IRENDER_H

#include <Box2D/Box2D.h>
#include <boost/shared_ptr.hpp>

struct b2AABB;

// This class implements debug drawing callbacks that are invoked
// inside b2World::Step.
class IRender : public b2Draw
{
public:
  typedef boost::shared_ptr<IRender> Ptr;

  virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) = 0;
  virtual void DrawString(int x, int y, const char* string, ...) = 0; 
  virtual void DrawAABB(b2AABB* aabb, const b2Color& color) = 0;

  virtual void EnableBlend(bool e) = 0;
};

std::auto_ptr<IRender> CreateGLRender();
std::auto_ptr<IRender> CreateNullRender();

#endif
