#ifndef ARCHIVEFILESACTIONS_H
#define ARCHIVEFILESACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>
#include <QToolBar>

class ArchiveFilesActions : public QObject {
 public:
  explicit ArchiveFilesActions(QObject* parent = nullptr);
  QToolBar* GetArchiveTooBar();

  QAction* COMPRESSED_HERE{new (std::nothrow) QAction{QIcon(":img/COMPRESSED"), "Compressd here", this}};
  QAction* COMPRESSED_TO{new (std::nothrow) QAction{"Compressd to", this}};

  QAction* DECOMPRESSED_TO{new (std::nothrow) QAction{"Decompressd to", this}};
  QAction* DECOMPRESSED_HERE{new (std::nothrow) QAction{"Decompressd here", this}};

  QAction* COMPRESSED_IMAGES{new (std::nothrow) QAction{"Compress Images", this}};

  QAction* ARCHIVE_PREVIEW{new (std::nothrow) QAction{QIcon(":img/COMPRESS_ITEM"), "Archive", this}};
};

ArchiveFilesActions& g_AchiveFilesActions();

#endif  // ARCHIVEFILESACTIONS_H
