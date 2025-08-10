#include "ArchiveFilesActions.h"
#include "PublicMacro.h"
#include "StyleSheet.h"

ArchiveFilesActions::ArchiveFilesActions(QObject* parent) : QObject{parent} {
  COMPRESSED_HERE->setToolTip("Compresse selected files to current path.\n(folders not supported now)");
  DECOMPRESSED_HERE->setToolTip("Decompressed one selected .qz files to current path.");
  COMPRESSED_IMAGES->setToolTip("Compress images in current folder and its directly sub folder.");
  ARCHIVE_PREVIEW->setToolTip("Open the selected .qz file");
}

QToolBar* GetArchiveTooBar() {
  QToolBar* compressToolBar = new (std::nothrow) QToolBar{"Compress/Decompress"};
  CHECK_NULLPTR_RETURN_NULLPTR(compressToolBar);
  compressToolBar->addAction(g_AchiveFilesActions().COMPRESSED_HERE);
  compressToolBar->addAction(g_AchiveFilesActions().COMPRESSED_IMAGES);
  compressToolBar->addAction(g_AchiveFilesActions().DECOMPRESSED_HERE);
  compressToolBar->setOrientation(Qt::Orientation::Vertical);
  compressToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  compressToolBar->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  compressToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
  return compressToolBar;
}

ArchiveFilesActions& g_AchiveFilesActions() {
  static ArchiveFilesActions ins;
  return ins;
}
