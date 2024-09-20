#include "LongPathFinder.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

constexpr int LongPathFinder::MAX_PATH_LENGTH;

void LongPathFinder::SetDropSectionWhenTooLong(int section) {
  dropSection = section;
  if (pres.size() != olds.size() || olds.size() != news.size()) {
    qWarning("length not match[%d, %d, %d]", pres.size(), olds.size(), news.size());
    return;
  }
  if (pres.size() == 0) {
    return;
  }
  news.clear();
  for (const QString& oldFolderName : olds) {
    const QString& newFolderName = GetNewFolderName(oldFolderName);
    news.append(newFolderName);
  }
}

QString LongPathFinder::GetNewFolderName(const QString& oldName) const {
  auto oldNameSectionsList = oldName.split(" - ");
  if (oldNameSectionsList.size() <= 1 || oldNameSectionsList.size() < std::abs(dropSection)) {
    return oldName;
  }
  oldNameSectionsList.erase(oldNameSectionsList.end() + dropSection);
  return oldNameSectionsList.join(" - ");
}

int LongPathFinder::operator()(const QString& path) {
  pres.clear();
  olds.clear();
  news.clear();
  QFileInfo fi(path);
  if (not fi.exists()) {
    qWarning("file not exists[%s], skip", qPrintable(path));
    return 0;
  }
  if (fi.isFile()) {
    qDebug("skip file[%s]", qPrintable(path));
    return 0;
  }
  int overlongCnt = 0;
  QDirIterator it(path, {}, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    const QString& currentPath = it.next();
    if (IsLengthBelowSysLimit(currentPath)) {
      continue;
    }
    const QString& oldFolderName = QFileInfo(currentPath).fileName();
    const QString& newFolderName = GetNewFolderName(oldFolderName);
    pres.append(it.fileInfo().absolutePath());
    olds.append(oldFolderName);
    news.append(newFolderName);
    ++overlongCnt;
  }
  qDebug("%d folder(s) path too long", overlongCnt);
  return overlongCnt;
}

int LongPathFinder::CheckTooLongPathCount() {
  m_status.clear();
  if (pres.size() != olds.size() || olds.size() != news.size()) {
    qWarning("length not match[%d, %d, %d]", pres.size(), olds.size(), news.size());
    return 0;
  }
  m_status.reserve(pres.size());
  // len(pre) + 1 + len(newName) + 1 + len(oldName) + 5 < MAX_PATH_LENGTH
  for (int i = 0; i < pres.size(); ++i) {
    if (pres[i].size() + news[i].size() + olds[i].size() + 7 < MAX_PATH_LENGTH) {
      m_status += '\n';
      continue;
    }
    m_status += "x\n";
  }
  return m_status.size() - pres.size();
}
