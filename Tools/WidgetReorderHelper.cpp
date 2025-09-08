#include "WidgetReorderHelper.h"
#include "Logger.h"
#include <set>

bool MoveElementFrontOf(QVector<int>& v, int fromIndex, int destIndex) {
  if (fromIndex < 0 || fromIndex >= v.size() || destIndex < 0 || destIndex > v.size()) {
    // input error
    return false;
  }

  if (fromIndex == destIndex || fromIndex + 1 == destIndex) {
    // no need move
    return true;
  }
  // {0,1,2}, 0, 2
  // {0,1,2}
  int val = v[fromIndex];
  v.erase(v.begin() + fromIndex);
  const int insertIndex{destIndex > fromIndex ? destIndex - 1 : destIndex};
  v.insert(v.begin() + insertIndex, val);
  return true;
}

bool IsValidMediaTypeSeq(const QString& seqStr, QVector<int>& result) {
  const int N = seqStr.size();
  if (N == 0 || N > 10) {
    return false;
  }
  result.clear();
  result.reserve(N);
  for (QChar ch : seqStr) {
    if (!ch.isDigit()) {
      return false;
    }
    int num = ch.digitValue();
    result.append(num);
  }
  // must be a continous sequence in [0, N)
  const std::set<int> uniqueSorted(result.begin(), result.end());
  return ((int)uniqueSorted.size() == N) && (*uniqueSorted.cbegin() == 0) && (*uniqueSorted.crbegin() == N - 1);
}

QString MediaTypeSeqStr(const QVector<int>& result) {
  QString ans;
  ans.reserve(result.size());
  for (int digit : result) {
    ans += QChar{'0' + digit};
  }
  return ans;
}

bool NeedMove(const int fromIndex, const int destIndex, const int N) {
  // out of bound/same index => no need move
  if (fromIndex == destIndex || fromIndex + 1 == destIndex) {
    LOG_D("no need move widget at index[%d] to destination in front of index[%d]", fromIndex, destIndex);
    return false;
  }
  if (fromIndex < 0 || fromIndex >= N) {
    LOG_W("fromIndex[%d] out of bound[0, %d)", fromIndex, N);
    return false;
  }
  if (destIndex < 0 || destIndex > N) {
    LOG_W("destIndex[%d] out of bound[0, %d]", destIndex, N);
    return false;
  }
  return true;
}

bool MoveWidgetAtFromIndexInFrontOfDestIndex(int fromIndex, int destIndex, QToolBar& toolbar) {
  const QList<QAction*>& oldActs = toolbar.actions();
  const int N = oldActs.size();
  if (!NeedMove(fromIndex, destIndex, N)) {
    return false;
  }
  QWidget* widget = toolbar.widgetForAction(oldActs[fromIndex]);
  if (widget == nullptr) {
    LOG_W("Only widget[index:%d] can move", fromIndex);
    return false;
  }
  LOG_D("move widget(index at %d) in front of %d", fromIndex, destIndex);
  if (destIndex >= N) {
    toolbar.addWidget(widget);
  } else {
    toolbar.insertWidget(oldActs[destIndex], widget);
  }
  const QList<QAction*>& newActs = toolbar.actions();
  const int removeIndex{destIndex > fromIndex ? fromIndex : fromIndex + 1};
  toolbar.removeAction(newActs[removeIndex]);
  return true;
}

bool MoveWidgetAtFromIndexInFrontOfDestIndex(int fromIndex, int destIndex, QSplitter& splitter) {
  const int N = splitter.count();
  if (!NeedMove(fromIndex, destIndex, N)) {
    return false;
  }
  //  LOG_D("layout.metaObject().className(): %s", layout->metaObject()->className());
  QWidget* fromWidget = splitter.widget(fromIndex);
  if (fromWidget == nullptr) {
    LOG_W("Only widget[index:%d] can move", fromIndex);
    return false;
  }
  LOG_D("move widget(index at %d) in front of %d", fromIndex, destIndex);
  const int insertIndex{destIndex > fromIndex ? destIndex - 1 : destIndex};
  splitter.insertWidget(insertIndex, fromWidget);
  return true;
}

bool MoveWidgetAtFromIndexInFrontOfDestIndex(int fromIndex, int destIndex, QBoxLayout& layout) {
  const int N = layout.count();
  if (!NeedMove(fromIndex, destIndex, N)) {
    return false;
  }
  //  LOG_D("layout.metaObject().className(): %s", layout->metaObject()->className());
  QWidget* fromWidget = layout.takeAt(fromIndex)->widget();
  if (fromWidget == nullptr) {
    LOG_W("Only widget[index:%d] can move", fromIndex);
    return false;
  }
  LOG_D("move widget(index at %d) in front of %d", fromIndex, destIndex);
  const int insertIndex{destIndex > fromIndex ? destIndex - 1 : destIndex};
  layout.insertWidget(insertIndex, fromWidget);
  return true;
}
