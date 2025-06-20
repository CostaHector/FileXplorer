#include "SplitterInsertIndexHelper.h"
#include <QDebug>
#include <set>

void MoveElementFrontOf(QVector<int>& v, int fromIndex, int destIndex) {
  if (fromIndex < 0 || fromIndex >= v.size() || destIndex < 0 || destIndex > v.size()) {
    return;
  }

  if (fromIndex == destIndex || fromIndex + 1 == destIndex) {
    return;
  }

  int val = v[fromIndex];
  v.erase(v.begin() + fromIndex);
  v.insert(v.begin() + (destIndex > fromIndex ? destIndex - 1 : destIndex), val);
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

  // 检查范围[0,N)和连续性
  const std::set<int> uniqueSorted(result.begin(), result.end());
  return ((int)uniqueSorted.size() == N) && (*uniqueSorted.begin() == 0) && (*uniqueSorted.rbegin() == N - 1);

  return true;
}

QString MediaTypeSeqStr(const QVector<int>& result) {
  QString ans;
  ans.reserve(result.size());
  for(int digit : result) {
    ans.append(QChar('0' + digit));
  }
  return ans;
}
