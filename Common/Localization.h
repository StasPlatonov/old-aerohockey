#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <map>
#include <string>

#ifdef USE_WSTRING
  typedef std::wstring StringType;
  typedef wchar_t CharType;
  #define STR(X) L(X)
#else
  typedef std::string StringType;
  typedef char CharType;
  #define STR(X) (X)
#endif

//#define LOCALIZE(id) (std::string(#id).c_str())
#define LOCALIZE(id) Loc::LocaleManager::GetInstance().Find(STR(#id))
#define LOCALIZE2(locale, id) Loc::LocaleManager::GetInstance().Find(locale, STR(#id))

namespace Loc
{
  class LocaleManager
  {
  public:
    static LocaleManager& GetInstance();

    void Load(const StringType& filename);
    void LoadLocale(int lid, const StringType& filename);
    void SetLocale(int lid);

    const CharType* Find(const StringType& id) const;
    const CharType* Find(int lid, const StringType& id) const;

  private:
    LocaleManager();
    LocaleManager(const LocaleManager& other);

  private:
    typedef std::map<StringType, StringType> TranslationTable;

    struct LocaleInfo
    {
      LocaleInfo(int id = 0, const StringType& l = StringType())
        : Id(id)
        , Locale(l)
      {}

      int Id;
      StringType Locale;
    };
    typedef std::map<int, TranslationTable> Locales;
    Locales m_Locales;

    typedef std::map<int, LocaleInfo> LocaleInfos;
    LocaleInfos m_LocaleInfos;

    int m_CurrentLocale;
  };
}

#endif
