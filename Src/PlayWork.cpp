#define _UNICODE

#include "Launcher.h"
#include "Common/Localization.h"
#include "Common/Utils.h"
#include "Network/INetwork.h"
#include "Network/Impl/RakNetworkImpl.h"

#include <locale>
//-------------------------------------

namespace
{
  bool IsWin7()
  {
#if defined(WIN32) || defined(WINNT)
    OSVERSIONINFO ver;
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ver);
    //printf("OS version: %d.%d\n", ver.dwMajorVersion, ver.dwMinorVersion);
    return (ver.dwMajorVersion >= 6 && ver.dwMinorVersion >= 1);
#else
    return false;
#endif
  }

  void TestBuffers()
  {
   /* struct CBS
    {
      int i;
      float f;
      double d;
    };
    CBS testData;

    printf("Testing circularbuffer class..\n");
    const int bufferSize = 4096;
    const int numIterations = 4000;
    CircularBuffer<CBS> buffer1;
    buffer1.Resize(bufferSize);
    Network::NetTime start = Utils::Timer::GetInstance().GetTime();
    for (int i = 0; i < numIterations; ++i)
    {
      //fill buffer
      for (int j = 0; j < bufferSize-1; ++j)
      {
        buffer1.Add(testData);
      }

      //read filled data
      for (int j = 0; j < bufferSize-1; ++j)
      {
        CBS readData = buffer1[j];
      }

      //erase filled data
      //buffer1.Resize(bufferSize);
    }
    Network::NetTime end = Utils::Timer::GetInstance().GetTime();
    printf("Time: %llu ms\n", end - start);

    printf("Testing RingBuffer..\n");
    RingBuffer<bufferSize, CBS> buffer2;
    start = Utils::Timer::GetInstance().GetTime();
    for (int i = 0; i < numIterations; ++i)
    {
      //fill buffer
      for (int j = 0; j < bufferSize-1; ++j)
      {
        buffer2.Write(testData);
      }

      //read filled data
      for (int j = 0; j < bufferSize-1; ++j)
      {
        CBS readData;
        buffer2.Read(readData);
      }

      //erase filled data
      //buffer2.Clear();
    }
    end = Utils::Timer::GetInstance().GetTime();
    printf("Time: %llu ms\n", end - start);*/
  }
  //-----------------------------------------------------------------------

#if defined(WIN32) || defined(WINNT)
  typedef BOOL (WINAPI *pfnGetCurrentConsoleFontEx)(__in HANDLE hConsoleOutput, __in BOOL bMaximumWindow, __out PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx);
  typedef BOOL (WINAPI *pfnSetCurrentConsoleFontEx)(HANDLE __in hConsoleOutput, __in BOOL bMaximumWindow, __in PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx);
  typedef BOOL (WINAPI *pfnSetConsoleFont)(HANDLE hConsoleOutput, DWORD number);

  class CustomConsole
  {
  public:
    CustomConsole()
      : ConHandle(GetStdHandle(STD_OUTPUT_HANDLE))
      , FnGetCurrentConsoleFontEx(0)
      , FnSetCurrentConsoleFontEx(0)
      , FnSetConsoleFont(0)
    {
      if (HMODULE lib = LoadLibrary("kernel32.dll"))
      {
        FnSetCurrentConsoleFontEx = (pfnSetCurrentConsoleFontEx)GetProcAddress(lib, "SetCurrentConsoleFontEx");
        FnGetCurrentConsoleFontEx = (pfnGetCurrentConsoleFontEx)GetProcAddress(lib, "GetCurrentConsoleFontEx");
        FnSetConsoleFont = (pfnSetConsoleFont)GetProcAddress(lib, "SetConsoleFont");
        FreeLibrary(lib);
      }

      //store current settings
      BackupInitialSettings();

      //set new values
      ApplySettings();
    }

    ~CustomConsole()
    {
      RestoreInitialSettings();
    }
    //-----------------------------------------------------------------------

  private:
    void BackupInitialSettings()
    {
      if (FnGetCurrentConsoleFontEx)
      {
        InitialFontEx.cbSize = sizeof(CONSOLE_FONT_INFOEX);
        FnGetCurrentConsoleFontEx(ConHandle, FALSE, &InitialFontEx);
      }
      else
      {
        GetCurrentConsoleFont(ConHandle, FALSE, &InitialFont);
      }

      GetConsoleScreenBufferInfo(ConHandle, &InitialInfo);
    }
    //-----------------------------------------------------------------------

    void ApplySettings() const
    {
      SetConsoleTextAttribute(ConHandle, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);

      if (FnSetCurrentConsoleFontEx)
      {
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);
        cfi.nFont = 2;
        cfi.dwFontSize.X = 6;
        cfi.dwFontSize.Y = 9;
        cfi.FontFamily = 48;
        cfi.FontWeight = 700;
        wcscpy(cfi.FaceName, L"Lucida Console");
        FnSetCurrentConsoleFontEx(ConHandle, FALSE, &cfi);
      }
      else if (FnSetConsoleFont)
      {
        FnSetConsoleFont(ConHandle, 2);
      }

      system("cls"); //apply to entire console
    }
    //-----------------------------------------------------------------------

    void RestoreInitialSettings()
    {
      SetConsoleTextAttribute(ConHandle, InitialInfo.wAttributes);
      if (FnSetCurrentConsoleFontEx)
      {
        FnSetCurrentConsoleFontEx(ConHandle, FALSE, &InitialFontEx);
      }
      else if (FnSetConsoleFont)
      {
        //FnSetConsoleFont(ConHandle, InitialFont.nFont);//crashes here =(
      }

      system("cls"); //apply to entire console
    }
    //-----------------------------------------------------------------------

  private:
    HANDLE ConHandle;
    CONSOLE_FONT_INFOEX InitialFontEx;
    CONSOLE_FONT_INFO InitialFont; // Windows <= XP does not support CONSOLE_FONT_INFOEX
    CONSOLE_SCREEN_BUFFER_INFO InitialInfo;
    pfnGetCurrentConsoleFontEx FnGetCurrentConsoleFontEx;
    pfnSetCurrentConsoleFontEx FnSetCurrentConsoleFontEx;
    pfnSetConsoleFont FnSetConsoleFont;
  };
  //-----------------------------------------------------------------------
#endif
}

int main(int argc, char* argv[])
{
#if defined(WIN32) || defined(WINNT)
  CustomConsole console;
#endif
  //TestBuffers();

  //Loc::LocaleManager::GetInstance().LoadLocale(3, L"localization.xml");
  //printf("\n%S\n", LOCALIZE(ID_HELLO));
  /*
  for (int i = 1; i <= 4; ++i) { Loc::LocaleManager::GetInstance().LoadLocale(i, L"localization.xml"); }
  for (int i = 1; i <= 4; ++i)
  {
    Loc::LocaleManager::GetInstance().SetLocale(i);
    printf("\n%S\n", LOCALIZE(ID_HELLO));
  }
  */

  Network::INetPeerFactory& factory = Network::GetRakNetPeerFactory();
  printf("Input desired mode: ");
  const std::string mode = Utils::GetString();//argc > 1 ? argv[1] : std::string();
  
  std::auto_ptr<ILauncher> launcher;
  if (mode == std::string("numbers"))
  {
    launcher.reset(new Numbers::NumbersLauncher(factory));
  }
  else
  if (mode == std::string("aero"))
  {
    launcher.reset(new Aero::AeroLauncher(factory));
  }
  else
  {
    launcher.reset(new Test::ServerClientTestLauncher(factory));
  }

  launcher->Run();

  return 0;
}
