#ifndef CURRENTINDEXGUARD_H
#define CURRENTINDEXGUARD_H

#include <QList>

class QAbstractItemView;

struct CurrentIndexGuard {
public:
  explicit CurrentIndexGuard(QAbstractItemView& view_);
  ~CurrentIndexGuard();
  void setEnabled(bool bEnabled) {
    mEnabled = bEnabled;
  }

private:
  struct CellRange { int left, top, right, bottom; };
  QList<CellRange> m_oldRangeList;
  int m_oldCurrentIndexRow{-1}, m_oldCurrentIndexColumn{-1};
  bool mEnabled{true};
  QAbstractItemView& _view;
};
#endif // CURRENTINDEXGUARD_H
