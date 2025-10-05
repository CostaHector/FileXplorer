#ifndef INTONEWPATHMOCKER_H
#define INTONEWPATHMOCKER_H
#include <QString>

struct IntoNewPathParms {
  static IntoNewPathParms& GetInst() {
    static IntoNewPathParms inst;
    return inst;
  }

  explicit IntoNewPathParms(const QString& newPath="", bool isNewPath=false, bool isF5Force=false)//
      : m_newPath{newPath}, m_isNewPath{isNewPath}, m_isF5Force{isF5Force} {}

  bool operator==(const IntoNewPathParms& rhs) const {
    return m_newPath == rhs.m_newPath && m_isNewPath == rhs.m_isNewPath && m_isF5Force == rhs.m_isF5Force;
  }

  void clear() {
    m_newPath.clear();
    m_isNewPath = m_isF5Force = false;
  }

  QString m_newPath;
  bool m_isNewPath;
  bool m_isF5Force;
};

struct IntoNewPathMocker {
  bool operator()(QString newPath, bool isNewPath, bool isF5Force) const {
    IntoNewPathParms::GetInst().m_newPath = newPath;
    IntoNewPathParms::GetInst().m_isNewPath = isNewPath;
    IntoNewPathParms::GetInst().m_isF5Force = isF5Force;
    return true;
  }
};

struct SearchTextParms {
  explicit SearchTextParms(const QString& newSearchText="") : m_newSearchText{newSearchText} {}
  bool operator==(const SearchTextParms& rhs) const { return m_newSearchText == rhs.m_newSearchText; }
  void clear() { m_newSearchText.clear(); }
  QString m_newSearchText;
};

struct SearchTextChangedParms : public SearchTextParms {
  using SearchTextParms::SearchTextParms;
  static SearchTextChangedParms& GetInst() {
    static SearchTextChangedParms inst;
    return inst;
  }
};

struct SearchTextEnterParms : public SearchTextParms {
  using SearchTextParms::SearchTextParms;
  static SearchTextEnterParms& GetInst() {
    static SearchTextEnterParms inst;
    return inst;
  }
};

struct SearchTextChangedMocker {
  bool operator()(QString searchText) const {
    SearchTextChangedParms::GetInst().m_newSearchText = searchText;
    return true;
  }
};

struct SearchTextEnterMocker {
  bool operator()(QString searchText) const {
    SearchTextEnterParms::GetInst().m_newSearchText = searchText;
    return true;
  }
};

inline void PathSearchMockerClear() {
  IntoNewPathParms::GetInst().clear();
  SearchTextChangedParms::GetInst().clear();
  SearchTextEnterParms::GetInst().clear();
}

#endif  // INTONEWPATHMOCKER_H
