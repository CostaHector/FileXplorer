#include "StringTool.h"
#include <QRegularExpression>
#include <QFile>
#include <QSet>

namespace StringTool{
const QRegularExpression IMG_VID_SEP_COMP{"\\||\r\n|\n"};
bool FileNameLengthSorter(const QString& lhs, const QString& rhs) {
  if (lhs.size() != rhs.size()) { // file name length first
    return lhs.size() < rhs.size();
  }
  return lhs < rhs; // name ascii second
}

bool FileSizeSorter(const QString& lhs, const QString& rhs) { // not consider file name length
  const qint64 left{QFile{lhs}.size()};
  const qint64 right{QFile{rhs}.size()};
  if (left != right) {   // size first
    return left < right;
  }
  return lhs < rhs; // name ascii second
}

QStringList GetImgsListFromField(const QString& imgsField) {
  // images human sort 0 < 1 < ... < 9 < 10. not in alphabeit
  QStringList imgsLst = imgsField.split(IMG_VID_SEP_COMP, Qt::SkipEmptyParts);
  std::sort(imgsLst.begin(), imgsLst.end(), FileNameLengthSorter);
  return imgsLst;
}

QStringList GetVidsListFromField(const QString& vidsField) {
  return vidsField.split(IMG_VID_SEP_COMP, Qt::SkipEmptyParts);
}

int RemoveDuplicateKeepSequence(QStringList& hists) {
  const int beforeEleCnt{hists.size()};
  QSet<QString> seen;
  auto it = hists.begin();
  while (it != hists.end()) {
    if (seen.contains(*it)) {
      it = hists.erase(it);
    } else {
      seen.insert(*it);
      ++it;
    }
  }
  return hists.size() - beforeEleCnt;
}

int TrimEachElementAndRemoveEmpty(QStringList& keywords) {
  const int beforeEleCnt{keywords.size()};
  std::transform(keywords.begin(), keywords.end(), keywords.begin(),
                 [](QString str) { return str.trimmed(); });
  keywords.erase(std::remove_if(keywords.begin(), keywords.end(),
                                [](const QString &str) { return str.isEmpty(); }),
                 keywords.end());
  return keywords.size() - beforeEleCnt;
}

void SearchHistoryListProc(QStringList& hists) {
  TrimEachElementAndRemoveEmpty(hists);
  RemoveDuplicateKeepSequence(hists);
}

}
