#include "LongPathFinder.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

int LongPathFinder::MAX_PATH_LENGTH{260};

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
  for (const QString& oldFolderName: olds) {
    const QString& newFolderName = GetNewFolderName(oldFolderName);
    news.append(newFolderName);
  }
}

QString LongPathFinder::GetNewFolderName(const QString& oldName) const{
  auto oldNameSectionsList = oldName.split(" - ");
  if (oldNameSectionsList.size() <= 1 || oldNameSectionsList.size() < std::abs(dropSection)) {
    return oldName;
  }
  oldNameSectionsList.erase(oldNameSectionsList.end() + dropSection);
  return oldNameSectionsList.join(" - ");
}

// folderNameLen = N - backSlashInd - 1
// N + 1 + 4 + N - backSlashInd - 1 = 2N - backSlashInd + 4
bool LongPathFinder::LengthBelow(const QString& path) {
  const int backSlashInd = path.lastIndexOf('/');
  const int N = path.size();
  return 2 * N - backSlashInd + 4 < MAX_PATH_LENGTH;
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
    if (LengthBelow(currentPath)) {
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

int LongPathFinder::Check() {
  m_status.clear();
  if (pres.size() != olds.size() || olds.size() != news.size()) {
    qWarning("length not match[%d, %d, %d]", pres.size(), olds.size(), news.size());
    return 0;
  }
  m_status.reserve(pres.size());
  // len(pre) + 1 + len(newName) + 1 + len(newName) + 4 < MAX_PATH_LENGTH
  for (int i = 0; i < pres.size(); ++i) {
    if (pres[i].size() + 2 * news[i].size() + 6 < MAX_PATH_LENGTH) {
      m_status += '\n';
      continue;
    }
    m_status += "x\n";
  }
  return m_status.size() - pres.size();
}
