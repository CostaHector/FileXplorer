#include "ArchiveFilesActions.h"

ArchiveFilesActions::ArchiveFilesActions(QObject* parent) : QObject{parent} {
  COMPRESSED_HERE->setToolTip("Compresse selected files to current path.\n(folders not supported now)");
  DECOMPRESSED_HERE->setToolTip("Decompressed one selected .qz files to current path.");
  COMPRESSED_IMAGES->setToolTip("Compress images in current folder and its directly sub folder.");
  ARCHIVE_PREVIEW->setToolTip("Open the selected .qz file");
}

ArchiveFilesActions& g_AchiveFilesActions() {
  static ArchiveFilesActions ins;
  return ins;
}
