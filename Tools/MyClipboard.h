#ifndef MYCLIPBOARD_H
#define MYCLIPBOARD_H

#include <QAbstractItemView>
#include <QClipboard>
#include <QFileSystemModel>
#include "PublicVariable.h"

class MyClipboard : public QObject {
  Q_OBJECT
 public:
  explicit MyClipboard(QObject* parent = nullptr);

  int FillIntoClipboardSystemBehavior(const QStringList& pathsList, const QList<QUrl>& urls, const CCMMode cutCopy);
  int FillIntoClipboardKeepFilesLevelBehavior(const QStringList& pathsList, const QList<QUrl>& urls, const CCMMode cutCopy);
  int FillIntoClipboardFSKeepFilesLevelBehavior(const QString& fromPath, const QStringList& pathsList, const QList<QUrl>& urls, const CCMMode cutCopy);

  int FillSelectionIntoClipboard(QAbstractItemView* _view, QFileSystemModel* _fileSysModel, const CCMMode cutCopy);

  const QMimeData* mimeData(QClipboard::Mode mode = QClipboard::Clipboard) const { return m_clipboard->mimeData(); }

 signals:

 private:
  QClipboard* m_clipboard;
};

#endif  // MYCLIPBOARD_H
