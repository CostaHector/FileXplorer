#ifndef ARCHIVEFILESACTIONS_H
#define ARCHIVEFILESACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>
#include <QToolBar>

class ArchiveFilesActions : public QObject {
  Q_OBJECT
 public:
  explicit ArchiveFilesActions(QObject* parent = nullptr);
  QToolBar* GetArchiveTooBar();

  QAction* COMPRESSED_HERE{nullptr};
  QAction* COMPRESSED_TO{nullptr};

  QAction* DECOMPRESSED_TO{nullptr};
  QAction* DECOMPRESSED_HERE{nullptr};

  QAction* COMPRESSED_IMAGES{nullptr};

  QAction* ARCHIVE_PREVIEW{nullptr};
};

ArchiveFilesActions& g_AchiveFilesActions();

#endif  // ARCHIVEFILESACTIONS_H
