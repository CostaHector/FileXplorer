#ifndef INTONEWPATHMOCKER_H
#define INTONEWPATHMOCKER_H
#include <QString>

struct IntoNewPathParms {
  QString m_newPath;
  bool m_isNewPath;
  bool m_isF5Force;

  bool operator==(const IntoNewPathParms& rhs) const {
    return m_newPath == rhs.m_newPath && m_isNewPath == rhs.m_isNewPath && m_isF5Force == rhs.m_isF5Force;
  }

  static IntoNewPathParms& GetInst() {
    static IntoNewPathParms inst;
    return inst;
  }

  void clear() {
    m_newPath.clear();
    m_isNewPath = m_isF5Force = false;
  }
};

struct IntoNewPathMocker {
  bool operator()(QString newPath, bool isNewPath, bool isF5Force) const {
    IntoNewPathParms::GetInst().m_newPath = newPath;
    IntoNewPathParms::GetInst().m_isNewPath = isNewPath;
    IntoNewPathParms::GetInst().m_isF5Force = isF5Force;
    return true;
  }
};

#endif  // INTONEWPATHMOCKER_H
