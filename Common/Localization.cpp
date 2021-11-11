#include "Common/Localization.h"
#include "Common/xmlParser.h"

#if defined(WIN32) || defined(WINNT)
  #include <io.h>
  #include <fcntl.h>

  #include <windows.h>
#else
#include <string.h>
#endif

namespace Loc
{
  namespace
  {
    const CharType* TAG_ID = STR("id\0");
    const CharType* TAG_LOC = STR("loc\0");
    const CharType* TAG_TEXT = STR("text\0");
    const CharType* TAG_LOCALIZATION = STR("localization\0");
    const CharType* TAG_DEFAULT = STR("default");
    const CharType* XML_EXT = STR("xml");
  }

  LocaleManager::LocaleManager()
    : m_CurrentLocale(-1)
  {
   // _setmode(_fileno(stdout), _O_U8TEXT);
  }

  LocaleManager::LocaleManager(const LocaleManager& other)
    : m_CurrentLocale(-1)
  {
  }
  //------------------------------------------------------------------------------------------------

  LocaleManager& LocaleManager::GetInstance()
  {
    static LocaleManager instance;
    return instance;
  }
  //------------------------------------------------------------------------------------------------

  void LocaleManager::Load(const StringType& filename)
  {
    const StringType ext = filename.substr(filename.length() - 3, 3);

    if (ext == XML_EXT)
    {
      XMLNode root = XMLNode::openFileHelper(filename.c_str(), TAG_LOCALIZATION);
      if (root.isEmpty())
        return;

      for (int i = 0; i < root.nChildNode(); ++i)
      {
        XMLNode localeNode = root.getChildNode(i);
        const int lid =
#ifdef USE_WSTRING
        _wtoi
#else
        atoi
#endif
        (localeNode.getAttribute(TAG_ID));

        //save locale information
        LocaleInfo locInfo(lid, localeNode.getAttribute(TAG_LOC));
        m_LocaleInfos.insert(std::make_pair(lid, locInfo));//rewrite if exists

        Locales::iterator lit = m_Locales.find(lid);
        if (lit == m_Locales.end())
        {
          TranslationTable tt;
          lit = m_Locales.insert(std::make_pair(lid, tt)).first;
        }
        TranslationTable& translationTable = lit->second;


        for (int j = 0; j < localeNode.nChildNode(); ++j)
        {
          XMLNode unitNode = localeNode.getChildNode(j);
          translationTable.insert(std::make_pair(unitNode.getAttribute(TAG_ID), unitNode.getAttribute(TAG_TEXT)));
        }
      }

      const int defaultLocale =
#ifdef USE_WSTRING
        _wtoi
#else
        atoi
#endif
      (root.getAttribute(TAG_DEFAULT, 0));
      SetLocale(defaultLocale);
    }
  }
  //------------------------------------------------------------------------------------------------

  void LocaleManager::LoadLocale(int lid, const StringType& filename)
  {
    Locales::iterator lit = m_Locales.find(lid);
    if (lit == m_Locales.end())
    {
      TranslationTable tt;
      lit = m_Locales.insert(std::make_pair(lid, tt)).first;
    }

    TranslationTable& tTable = lit->second;

    const StringType ext = filename.substr(filename.length() - 3, 3);

    if (ext == XML_EXT)
    {
      XMLNode root = XMLNode::openFileHelper(filename.c_str(), TAG_LOCALIZATION);
      if (root.isEmpty())
        return;

      for (int i = 0; i < root.nChildNode(); ++i)
      {
        XMLNode localeNode = root.getChildNode(i);
#ifdef USE_WSTRING
        if (_wtoi(localeNode.getAttribute(TAG_ID)) != lid)
#else
        if (atoi(localeNode.getAttribute(TAG_ID)) != lid)
#endif
        {
           continue;
        }

        LocaleInfo locInfo(lid, localeNode.getAttribute(TAG_LOC));
        m_LocaleInfos.insert(std::make_pair(lid, locInfo));

        for (int j = 0; j < localeNode.nChildNode(); ++j)
        {
          XMLNode unitNode = localeNode.getChildNode(j);
          tTable.insert(std::make_pair(unitNode.getAttribute(TAG_ID), unitNode.getAttribute(TAG_TEXT)));
        }

        SetLocale(lid);
      }
    }

    if (!tTable.empty())
    {
      SetLocale(lid);
    }
  }
  //------------------------------------------------------------------------------------------------

  void LocaleManager::SetLocale(int lid)
  {
    m_CurrentLocale = lid;

    LocaleInfos::const_iterator locIt = m_LocaleInfos.find(m_CurrentLocale);
    if (locIt != m_LocaleInfos.end())
    {
#if defined(WIN32) || defined(WINNT)
	#ifdef _UNICODE
		  _wsetlocale(LC_ALL, locIt->second.Locale.c_str());
	#else
		  setlocale(LC_ALL, locIt->second.Locale.c_str());
	#endif
#else
      setlocale(LC_ALL, locIt->second.Locale.c_str());
#endif

      //SetConsoleCP(CP_UTF8);
      //SetConsoleOutputCP(CP_UTF8);
      //std::locale::global(std::locale("Russia_Russian"));
    }
  }
  //------------------------------------------------------------------------------------------------

  const CharType* LocaleManager::Find(const StringType& id) const
  {
    return Find(m_CurrentLocale, id);
  }
  //------------------------------------------------------------------------------------------------

  const CharType* LocaleManager::Find(int lid, const StringType& id) const
  {
    Locales::const_iterator lit = m_Locales.find(lid);
    if (lit == m_Locales.end())
    {
      return id.c_str();
    }

    const TranslationTable& tTable = lit->second;

    TranslationTable::const_iterator tit = tTable.find(id);

    if (tit == tTable.end())
    {
      return id.c_str();
    }

    //const wchar_t* test = tit->second.c_str();

    return tit->second.c_str();
  }
  //------------------------------------------------------------------------------------------------
}



