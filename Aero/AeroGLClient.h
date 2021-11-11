#ifndef AERO_GL_CLIENT_H
#define AERO_GL_CLIENT_H

#include "AeroClient.h"
#include "AeroPlayer.h"

#include "Network/INetwork.h"

#include "Common/Thread.h"

namespace Aero
{
  class AeroGLClient : public Aero::IAeroClientHandler, public Common::ThreadObject
  {
  public:
    AeroGLClient(const std::string& name, const std::string& host, const std::string& password, Network::INetPeerFactory& factory);
    ~AeroGLClient();

    //IAeroClientHandler
    virtual void OnMessage(const std::string& text);
    virtual bool IsActive() const { return m_Active; }
    virtual void OnUpdate();

    void Run();

    void Resize(int w, int h);
    void OnRenderTimer(int);
    void RenderFrame();
    void Keyboard(unsigned char key, int x, int y, bool pressed);
    void KeyboardSpecial(int key, int x, int y, bool pressed);
    void Mouse(int button, int state, int x, int y);
    void MouseMotion(int x, int y);
    void MouseWheel(int wheel, int direction, int x, int y);
    void SingleStep(int);
    void Exit(int);
  
    void UpdateLoop();

    virtual void OnRunThread();

  protected:
    void Idle();

    void StoreMove(const PlayerMove& move);
    void SendClientState();

  private:
    std::auto_ptr<Aero::IAeroClient> m_Client;
    IRender::Ptr m_Render;
    bool m_Active;
  };
  //-----------------------------------------------------------------------------------
}

#endif  //AERO_GL_CLIENT_H
