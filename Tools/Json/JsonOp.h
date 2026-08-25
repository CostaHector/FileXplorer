#ifndef JSONOP_H
#define JSONOP_H

namespace JsonOp {
enum class ResultE {
  ERROR = -1,
  IGNORE_NO_NEED_FURTHER_PROCESS = 0,
  OK_NEED_FURTHER_PROCESS = 1
};

struct Counter {
  explicit Counter(int jsonUpdatedCnt = 0, int jsonUsedCnt = 0, int imgNameKeyFieldUpdatedCnt = 0, int vidNameKeyFieldUpdatedCnt = 0)
      : m_jsonUpdatedCnt{jsonUpdatedCnt},
        m_jsonUsedCnt{jsonUsedCnt},
        m_ImgNameKeyFieldUpdatedCnt{imgNameKeyFieldUpdatedCnt},
        m_VidNameKeyFieldUpdatedCnt{vidNameKeyFieldUpdatedCnt} {}
  int m_jsonUpdatedCnt;
  int m_jsonUsedCnt;
  int m_ImgNameKeyFieldUpdatedCnt;
  int m_VidNameKeyFieldUpdatedCnt;
  Counter& operator+=(const Counter& rhs) {
    m_jsonUpdatedCnt += rhs.m_jsonUpdatedCnt;
    m_jsonUsedCnt += rhs.m_jsonUsedCnt;
    m_ImgNameKeyFieldUpdatedCnt += rhs.m_ImgNameKeyFieldUpdatedCnt;
    m_VidNameKeyFieldUpdatedCnt += rhs.m_VidNameKeyFieldUpdatedCnt;
    return *this;
  }
  bool operator==(const Counter& rhs) const {
    return m_jsonUpdatedCnt == rhs.m_jsonUpdatedCnt                           //
           && m_jsonUsedCnt == rhs.m_jsonUsedCnt                              //
           && m_VidNameKeyFieldUpdatedCnt == rhs.m_VidNameKeyFieldUpdatedCnt  //
           && m_ImgNameKeyFieldUpdatedCnt == rhs.m_ImgNameKeyFieldUpdatedCnt;
  }
  bool isEmpty() const { return m_jsonUpdatedCnt == 0 && m_jsonUsedCnt == 0; }
};
}

#endif // JSONOP_H
