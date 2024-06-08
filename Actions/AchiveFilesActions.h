#ifndef ACHIVEFILESACTIONS_H
#define ACHIVEFILESACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class AchiveFilesActions : public QObject {
 public:
  explicit AchiveFilesActions(QObject* parent = nullptr);

  QAction* COMPRESSED_HERE{new QAction{QIcon(":/themes/COMPRESSED"), "Compressd here", this}};
  QAction* COMPRESSED_TO{new QAction{"Compressd to", this}};

  QAction* DECOMPRESSED_TO{new QAction{"Decompressd to", this}};
  QAction* DECOMPRESSED_HERE{new QAction{"Decompressd here", this}};

  QAction* COMPRESSED_IMAGES{new QAction{"Compress Images", this}};

  QAction* ARCHIVE_PREVIEW{new QAction{QIcon(":/themes/COMPRESS_ITEM"), "Archive", this}};
};

AchiveFilesActions& g_AchiveFilesActions();

#endif  // ACHIVEFILESACTIONS_H
