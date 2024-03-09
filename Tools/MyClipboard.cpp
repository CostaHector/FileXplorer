#include "MyClipboard.h"
#include <QGuiApplication>
#include "MimeDataCX.h"
#include "ViewSelection.h"

MyClipboard::MyClipboard(QObject* parent) : QObject{parent}, m_clipboard(QGuiApplication::clipboard()) {}

int MyClipboard::FillIntoClipboardSystemBehavior(const QStringList& pathsList, const QList<QUrl>& urls, const CCMMode cutCopy) {
  QMimeData* mimedata = new QMimeData;
  mimedata->setUrls(urls);
  // files in pathsList will drop its former level relation
  mimedata->setText(pathsList.join('\n'));
  m_clipboard->setMimeData(mimedata);
  return urls.size();
}

int MyClipboard::FillIntoClipboardKeepFilesLevelBehavior(const QStringList& pathsList, const QList<QUrl>& urls, const CCMMode cutCopy) {
  if (pathsList.isEmpty()) {
    return 0;
  }
  QString fromPath;
  QStringList lRels;
  std::tie(fromPath, lRels) = PATHTOOL::GetLAndRels(pathsList);

  MimeDataCX* mimedata = new MimeDataCX(fromPath, lRels, cutCopy);
  mimedata->setUrls(urls);
  mimedata->setText(lRels.join('\n'));
  m_clipboard->setMimeData(mimedata);
  return pathsList.size();
}

int MyClipboard::FillIntoClipboardFSKeepFilesLevelBehavior(const QString& fromPath,
                                                           const QStringList& pathsList,
                                                           const QList<QUrl>& urls,
                                                           const CCMMode cutCopy) {
  if (pathsList.isEmpty()) {
    return 0;
  }
  const QStringList& lRels = PATHTOOL::GetRels(fromPath.size(), pathsList);
  MimeDataCX* mimedata = new MimeDataCX(fromPath, lRels, cutCopy);
  mimedata->setUrls(urls);
  mimedata->setText(lRels.join('\n'));
  m_clipboard->setMimeData(mimedata);
  return pathsList.size();
}

auto MyClipboard::FillSelectionIntoClipboard(QAbstractItemView* _view, QFileSystemModel* _fileSysModel, const CCMMode cutCopy) -> int {
  QStringList lRels;
  QList<QUrl> urls;
  for (const QModelIndex& ind : ViewSelection::selectedIndexes(_view)) {
    lRels.append(_fileSysModel->fileName(ind));
    urls.append(QUrl::fromLocalFile(lRels.back()));
  }
  if (lRels.isEmpty()) {
    return 0;
  }
  const QString& fromPath = _fileSysModel->rootPath();
  MimeDataCX* mimedata = new MimeDataCX(fromPath, lRels, cutCopy);
  mimedata->setUrls(urls);
  mimedata->setText(lRels.join('\n'));
  m_clipboard->setMimeData(mimedata);
  return urls.size();
}
