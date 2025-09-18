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
};

struct IntoNewPathMocker {
  explicit IntoNewPathMocker(IntoNewPathParms* pOutPara) { mOutPara = pOutPara; }
  bool operator()(QString newPath, bool isNewPath, bool isF5Force) const {
    if (mOutPara == nullptr) {
      return false;
    }
    mOutPara->m_newPath = newPath;
    mOutPara->m_isNewPath = isNewPath;
    mOutPara->m_isF5Force = isF5Force;
    return true;
  }

  IntoNewPathParms* mOutPara{nullptr};
  mutable int callCount = 0;
};

#endif  // INTONEWPATHMOCKER_H
