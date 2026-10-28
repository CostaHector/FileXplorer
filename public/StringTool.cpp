#include "StringTool.h"
#include <QRegularExpression>
#include <QFile>
#include <QSet>
#include <QCache>

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

void ImgsSortByNumber(QStringList& imgs) {
  QStringList tnImgs;                              // _tn 等非数字后缀，最后追加
  std::map<int, QList<int>> number2Index;          // 编号 -> 原列表索引

  for (int i = 0; i < imgs.size(); ++i) {
    const QString& s = imgs[i];
    const int dotPos = s.lastIndexOf('.');
    if (dotPos <= 0) {                             // 没有扩展名，或 '.' 在开头
      tnImgs.append(s);
      continue;
    }
    if (!s[dotPos - 1].isDigit()) {                // '.' 前不是数字，视为 _tn
      tnImgs.append(s);
      continue;
    }
    const int spacePos = s.lastIndexOf(' ', dotPos);
    if (spacePos < 0) {                            // 没有空格，无法取编号
      tnImgs.append(s);
      continue;
    }
    bool bIsNumber{false};
    const int number = s.midRef(spacePos + 1, dotPos - spacePos - 1).toInt(&bIsNumber);
    if (!bIsNumber) {
      tnImgs.append(s);
      continue;
    }
    number2Index[number].append(i);                // 同编号按原顺序保留
  }

  QStringList sorted;
  sorted.reserve(imgs.size());
  for (auto it = number2Index.cbegin(); it != number2Index.cend(); ++it) {
    for (int idx : it->second) {
      sorted.append(imgs[idx]);
    }
  }
  sorted += tnImgs;
  imgs = std::move(sorted);
}

QString ImgsMinByNumber(const QStringList& imgs) {
  if (imgs.isEmpty()) return {};

  int bestIndex{0};
  int bestNum{INT_MAX};

  for (int i = 0; i < imgs.size(); ++i) {
    const QString& s = imgs[i];
    int dotPos = s.lastIndexOf('.');

    if (dotPos > 0 && s[dotPos - 1].isDigit()) {
      int spacePos = s.lastIndexOf(' ', dotPos);
      if (spacePos >= 0) {
        bool bIsNumber = false;
        const int number = s.midRef(spacePos + 1, dotPos - spacePos - 1).toInt(&bIsNumber);
        if (bIsNumber && number < bestNum) {
          bestNum   = number;
          bestIndex = i;
        }
      }
    }
  }
  return imgs[bestIndex];
}

QStringList GetImgsVidsListFromField(const QString& imgsField) {
  return imgsField.split(IMG_VID_SEP_COMP, Qt::SkipEmptyParts);
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

QString PathJoinPixmapSize(QString path, int width, int height, bool bSmoothTrans) {
  path.reserve(path.size() + 12);
  path += QString::asprintf("_%dx%d_%d", width, height, bSmoothTrans);
  return path;
}

QByteArray EscapeCsv(const QString &value) {
  if (value.isEmpty()) {
    return {};
  }
  bool bContainsDoubleQuote{false};
  if (!(value.contains(',') || (bContainsDoubleQuote = value.contains('"')) || value.contains('\n'))){
    return value.toUtf8();
  }
  QByteArray escaped;
  escaped.reserve(value.size() * 4 + 2);
  escaped.append('z'); // nothing will be replaced
  escaped += value.toUtf8();
  if (bContainsDoubleQuote) {
    escaped.replace('"', R"("")");
  }
  escaped.front() = '"';
  escaped.push_back('"');
  return escaped;
}

QRegularExpression GetRegularExpression(const QString& targetStr) {
  static QCache<QString, const QRegularExpression> exprCache(100);
  const QRegularExpression* pExpr = exprCache[targetStr];
  if (pExpr != nullptr) {
    return *pExpr;
  }
  pExpr = new (std::nothrow) QRegularExpression(targetStr);
  if (pExpr == nullptr) {
    return {};
  }
  exprCache.insert(targetStr, pExpr);
  return *pExpr;
}
}
