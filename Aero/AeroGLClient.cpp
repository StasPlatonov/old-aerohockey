#include "AeroGLClient.h"

#include "Common/Utils.h"
#include "Render/IRender.h"

#if defined(_WIN32)
	#include "GL/freeglut.h"
#else
	#include "freeglut/freeglut.h"
#endif

#include <gl/glui.h>
#include <Kbhit.h>

namespace Aero
{
  namespace
  {
    const unsigned short AERO_PORT = 2345;
    const unsigned CL_TICKS_RATE = 30;
    const int32 CL_RENDER_TICK = 16; // ~60 fps

	  Settings settings;
	  int32 width = 160;
	  int32 height = 160;
	  int32 mainWindow;
	  float settingsHz = 70.0;
	  GLUI *glui;
	  float32 viewZoom = 1.0f;
	  int tx, ty, tw, th;
    bool rMouseDown;
    b2Vec2 lastp;

    AeroGLClient* g_Client = 0;

    bool IsNumber(const std::string& str)
    {
      std::string::const_iterator it = str.begin();
      while (it != str.end() && isdigit(*it)) ++it;
      return !str.empty() && it == str.end();
    }

  #ifdef USE_GL
    static void Resize_S(int32 w, int32 h)
    {
      g_Client->Resize(w, h);
    }

    void RenderTimer_S(int i)
    {
      g_Client->OnRenderTimer(i);
    }

    static void RenderFrame_S()
    {
      g_Client->RenderFrame();
    }

    static void Keyboard_S(unsigned char key, int x, int y)
    {
      g_Client->Keyboard(key, x, y, true);
    }

    static void KeyboardUp_S(unsigned char key, int x, int y)
    {
      g_Client->Keyboard(key, x, y, false);
    }

    static void KeyboardSpecial_S(int key, int x, int y)
    {
      g_Client->KeyboardSpecial(key, x, y, true);
    }

    static void KeyboardSpecialUp_S(int key, int x, int y)
    {
      g_Client->KeyboardSpecial(key, x, y, false);
    }

    static void Mouse_S(int32 button, int32 state, int32 x, int32 y)
    {
      g_Client->Mouse(button, state, x, y);
    }

    static void MouseMotion_S(int32 x, int32 y)
    {
      g_Client->MouseMotion(x, y);
    }

    static void MouseWheel_S(int wheel, int direction, int x, int y)
    {
      g_Client->MouseWheel(wheel, direction, x, y);
    }

    static void SingleStep_S(int i)
    {
      g_Client->SingleStep(i);
    }

/*    static void Restart_S(int i)
    {
      g_Client->Restart(i);
    }

    static void Pause_S(int i)
    {
      g_Client->Pause(i);
    }*/

    static void Exit_S(int code)
    {
      g_Client->Exit(code);
    }
  #endif
  }

  //---------------------------------------------------------------------


  void AeroGLClient::Resize(int32 w, int32 h)
  {
	  width = w;
	  height = h;

	  //GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
	  tx = 0; ty = 0;
	  tw = width; th = height;
	  
	  glViewport(tx, ty, tw, th);

	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
	  float32 ratio = float32(tw) / float32(th);

	  b2Vec2 extents(ratio * 25.0f, 25.0f);
	  extents *= viewZoom;

	  b2Vec2 lower = settings.viewCenter - extents;
	  b2Vec2 upper = settings.viewCenter + extents;

	  // L/R/B/T
	  gluOrtho2D(lower.x, upper.x, lower.y, upper.y);
  }
  //-----------------------------------------------------------------------------------

  static b2Vec2 ConvertScreenToWorld(int32 x, int32 y)
  {
	  float32 u = x / float32(tw);
	  float32 v = (th - y) / float32(th);

	  float32 ratio = float32(tw) / float32(th);
	  b2Vec2 extents(ratio * 25.0f, 25.0f);
	  extents *= viewZoom;

	  b2Vec2 lower = settings.viewCenter - extents;
	  b2Vec2 upper = settings.viewCenter + extents;

	  b2Vec2 p;
	  p.x = (1.0f - u) * lower.x + u * upper.x;
	  p.y = (1.0f - v) * lower.y + v * upper.y;
	  return p;
  }
  //-----------------------------------------------------------------------------------

  void AeroGLClient::OnRenderTimer(int i)
  {
	  glutSetWindow(mainWindow);
	  glutPostRedisplay();
#ifdef USE_GL
    glutTimerFunc(CL_RENDER_TICK, RenderTimer_S, 0);
#endif
  }
  //-----------------------------------------------------------------------------------

  void AeroGLClient::RenderFrame()
  {
    glClearColor(255, 255, 255, 255);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();

    m_Client->DebugDraw(m_Render);

    /*b2Vec2 oldCenter = settings.viewCenter;
	  if (oldCenter.x != settings.viewCenter.x || oldCenter.y != settings.viewCenter.y)
	  {
		  Resize(width, height);
	  }*/

    glutSwapBuffers();
  }
  //-----------------------------------------------------------------------------------

  void AeroGLClient::Keyboard(unsigned char key, int x, int y, bool pressed)
  {
	  B2_NOT_USED(x);
	  B2_NOT_USED(y);

    if (pressed)
    {
	    switch (key)
	    {
	    case 27:
    #ifndef __APPLE__
		    // freeglut specific function
		    glutLeaveMainLoop();
    #endif
		    exit(0);
		    break;

		    // Press 'z' to zoom out.
	    case 'z':
		    viewZoom = b2Min(1.1f * viewZoom, 20.0f);
		    Resize(width, height);
		    break;

		    // Press 'x' to zoom in.
	    case 'x':
		    viewZoom = b2Max(0.9f * viewZoom, 0.02f);
		    Resize(width, height);
		    break;

		    // Press 'r' to reset.
	    case 'r':
		    break;

		    // Press space to launch a bomb.
	    case ' ':
		    break;

      case 'h':
        break;
 
	    case 'p':
        m_Client->Pause();
		    break;

	    default:
        break;
	    }
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroGLClient::KeyboardSpecial(int key, int x, int y, bool pressed)
  {
	  B2_NOT_USED(x);
	  B2_NOT_USED(y);

    m_Client->Keyboard(key, pressed);
    /*
	  switch (key)
	  {
	  case GLUT_ACTIVE_SHIFT:
      break;
		
		  // Press home to reset the view.
	  case GLUT_KEY_HOME:
		  viewZoom = 1.0f;
		  settings.viewCenter.Set(0.0f, 20.0f);
		  Resize(width, height);
		  break;
	  }*/
  }
  //-----------------------------------------------------------------------------------
  
  void AeroGLClient::Mouse(int32 button, int32 state, int32 x, int32 y)
  {
	  // Use the mouse to move things around.
	  if (button == GLUT_LEFT_BUTTON)
	  {
		  int mod = glutGetModifiers();
		  b2Vec2 p = ConvertScreenToWorld(x, y);

      //b2Body* picked = m_Phys->Pick(p);

		  if (state == GLUT_DOWN)
		  {
        m_Client->Click(p.x, p.y);
		  }
		  if (state == GLUT_UP)
		  {
		  }
	  }
	  else if (button == GLUT_MIDDLE_BUTTON)
	  {
		  if (state == GLUT_DOWN)
		  {	
			  const b2Vec2 shootTarget = ConvertScreenToWorld(x, y);
        m_Client->Shoot(shootTarget.x, shootTarget.y);
		  }
	  }
  }
  //-----------------------------------------------------------------------------------

  void AeroGLClient::MouseMotion(int32 x, int32 y)
  {
    //m_Input.m_X = x;
    //m_Input.m_Y = y;

	  b2Vec2 p = ConvertScreenToWorld(x, y);

	  if (rMouseDown)
	  {
		  b2Vec2 diff = p - lastp;
		  settings.viewCenter.x -= diff.x;
		  settings.viewCenter.y -= diff.y;
		  Resize(width, height);
		  lastp = ConvertScreenToWorld(x, y);
	  }
  }
  //-----------------------------------------------------------------------------------

  void AeroGLClient::MouseWheel(int wheel, int direction, int x, int y)
  {
	  B2_NOT_USED(wheel);
	  B2_NOT_USED(x);
	  B2_NOT_USED(y);
	  if (direction > 0)
	  {
		  viewZoom /= 1.1f;
	  }
	  else
	  {
		  viewZoom *= 1.1f;
	  }
	  Resize(width, height);
  }
  //-----------------------------------------------------------------------------------

  void AeroGLClient::Exit(int code)
  {
  #ifdef FREEGLUT
	  glutLeaveMainLoop();
  #endif
	  exit(code);
  }

  void AeroGLClient::SingleStep(int)
  {
	  //settings.pause = 1;
	  //settings.singleStep = 1;
  }
  //-----------------------------------------------------------------------------------

  AeroGLClient::AeroGLClient(const std::string& name, const std::string& host, const std::string& password, Network::INetPeerFactory& factory)
    : Common::ThreadObject(1000)
    , m_Client(Aero::CreateAeroClient(factory, *this))
    , m_Active(true)
    //, m_Time(0)
  {
    g_Client = this;
    m_Render = IRender::Ptr(CreateGLRender().release());
    m_Client->Init(name, m_Render);

#ifdef USE_GL
    int argc = 1;
    char* argv = "AeroGLClient";
	  glutInit(&argc, &argv);
	  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	  glutInitWindowSize(width, height);
	  char title[32];
	  sprintf(title, "Box2D Version %d.%d.%d", b2_version.major, b2_version.minor, b2_version.revision);
	  mainWindow = glutCreateWindow(title);
	  //glutSetOption (GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	  glutDisplayFunc(RenderFrame_S);
	      //GLUI_Master.set_glutReshapeFunc(Resize_S);
		  glutReshapeFunc(Resize_S);
	  //GLUI_Master.set_glutKeyboardFunc(Keyboard_S);
	  //GLUI_Master.set_glutSpecialFunc(KeyboardSpecial_S);
	     //GLUI_Master.set_glutMouseFunc(Mouse_S);
		  glutMouseFunc(Mouse_S);
  #ifdef FREEGLUT
	  glutMouseWheelFunc(MouseWheel_S);
  #endif
	  glutMotionFunc(MouseMotion_S);

    glutKeyboardFunc(Keyboard_S);
	  glutKeyboardUpFunc(KeyboardUp_S);
    glutSpecialFunc(KeyboardSpecial_S);
    glutSpecialUpFunc(KeyboardSpecialUp_S);
 	  // Use a timer to control the frame rate.
	  glutTimerFunc(CL_RENDER_TICK, RenderTimer_S, 0);
#endif
	  Resize(width, height);

/*
	  glui = GLUI_Master.create_glui_subwindow( mainWindow, 
		  GLUI_SUBWINDOW_RIGHT );

	  glui->add_statictext("Tests");
	  GLUI_Listbox* testList = glui->add_listbox("");//, &testSelection);

	  glui->add_separator();

	  GLUI_Spinner* velocityIterationSpinner =
		  glui->add_spinner("Vel Iters", GLUI_SPINNER_INT, &settings.velocityIterations);
	  velocityIterationSpinner->set_int_limits(1, 500);

	  GLUI_Spinner* positionIterationSpinner =
		  glui->add_spinner("Pos Iters", GLUI_SPINNER_INT, &settings.positionIterations);
	  positionIterationSpinner->set_int_limits(0, 100);

	  GLUI_Spinner* hertzSpinner =
		  glui->add_spinner("Hertz", GLUI_SPINNER_FLOAT, &settingsHz);

	  hertzSpinner->set_float_limits(5.0f, 200.0f);

	  glui->add_checkbox("Warm Starting", &settings.enableWarmStarting);
	  glui->add_checkbox("Time of Impact", &settings.enableContinuous);
	  glui->add_checkbox("Sub-Stepping", &settings.enableSubStepping);

	  //glui->add_separator();

	  GLUI_Panel* drawPanel =	glui->add_panel("Draw");
	  glui->add_checkbox_to_panel(drawPanel, "Shapes", &settings.drawShapes);
	  glui->add_checkbox_to_panel(drawPanel, "Joints", &settings.drawJoints);
	  glui->add_checkbox_to_panel(drawPanel, "AABBs", &settings.drawAABBs);
	  glui->add_checkbox_to_panel(drawPanel, "Pairs", &settings.drawPairs);
	  glui->add_checkbox_to_panel(drawPanel, "Contact Points", &settings.drawContactPoints);
	  glui->add_checkbox_to_panel(drawPanel, "Contact Normals", &settings.drawContactNormals);
	  glui->add_checkbox_to_panel(drawPanel, "Contact Forces", &settings.drawContactForces);
	  glui->add_checkbox_to_panel(drawPanel, "Friction Forces", &settings.drawFrictionForces);
	  glui->add_checkbox_to_panel(drawPanel, "Center of Masses", &settings.drawCOMs);
	  glui->add_checkbox_to_panel(drawPanel, "Statistics", &settings.drawStats);
	  glui->add_checkbox_to_panel(drawPanel, "Profile", &settings.drawProfile);

	  int32 testCount = 0;
	  testList->add_item(0, "Item1");

	  glui->add_button("Pause", 0, Pause_S);
	  glui->add_button("Single Step", 0, SingleStep_S);
	  glui->add_button("Restart", 0, Restart_S);

	  glui->add_button("Quit", 0,(GLUI_Update_CB)Exit_S);
	  glui->set_main_gfx_window( mainWindow );
    */
    
    //settings.pause = true;
    StartThread();

    m_Client->Connect(host.c_str(), AERO_PORT, password);
  }

  AeroGLClient::~AeroGLClient()
  {
    StopThread(1000);
    m_Client->Done();
  }
  //-----------------------------------------------------------------------------------

  void AeroGLClient::OnMessage(const std::string& text)
  {
    printf("%s", text.c_str());
  }
  //-----------------------------------------------------------------------------------
   

  void AeroGLClient::Idle()
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
      {
        printf("\nYou have to enter a valid command\n");
      }
    }
  }
  //-----------------------------------------------------------------------------------

  void AeroGLClient::OnRunThread()
  {
    UpdateLoop();
  }
  //-----------------------------------------------------------------------------------

  void AeroGLClient::UpdateLoop()
  {
    while (true)
    {
      if (WaitForStopEvent(CL_TICKS_RATE))
      {
        break;
      }

      m_Client->UpdatePlayers();
    }
  }
  //------------------------------------------------------------------------------------------

  void AeroGLClient::OnUpdate()
  {
    //RenderFrame();
#ifdef USE_GL
    glutMainLoopEvent();//process events iteration
#endif
      
    //Idle();
  }

  void AeroGLClient::Run()
  {
    m_Client->Run();
  }
  //-----------------------------------------------------------------------------------
}
