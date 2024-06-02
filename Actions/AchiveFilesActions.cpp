#include "AchiveFilesActions.h"

AchiveFilesActions::AchiveFilesActions(QObject* parent) : QObject{parent} {
  COMPRESSED_HERE->setToolTip("Compresse selected files to current path.\n(folders not supported now)");
  DECOMPRESSED_HERE->setToolTip("Decompressed one selected .qz files to current path.");
  COMPRESSED_IMAGES->setToolTip("Compress images in current folder and its directly sub folder.");
  ARCHIEVE_PREVIEW->setToolTip("Show .qz file preview in QTextEdit.");
}

AchiveFilesActions& g_AchiveFilesActions() {
  static AchiveFilesActions ins;
  return ins;
}
