#ifndef ARCHIVEFILESACTIONS_H
#define ARCHIVEFILESACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class ArchiveFilesActions : public QObject {
 public:
  explicit ArchiveFilesActions(QObject* parent = nullptr);

  QAction* COMPRESSED_HERE{new QAction{QIcon(":/themes/COMPRESSED"), "Compressd here", this}};
  QAction* COMPRESSED_TO{new QAction{"Compressd to", this}};

  QAction* DECOMPRESSED_TO{new QAction{"Decompressd to", this}};
  QAction* DECOMPRESSED_HERE{new QAction{"Decompressd here", this}};

  QAction* COMPRESSED_IMAGES{new QAction{"Compress Images", this}};

  QAction* ARCHIVE_PREVIEW{new QAction{QIcon(":/themes/COMPRESS_ITEM"), "Archive", this}};
};

ArchiveFilesActions& g_AchiveFilesActions();

#endif  // ARCHIVEFILESACTIONS_H
