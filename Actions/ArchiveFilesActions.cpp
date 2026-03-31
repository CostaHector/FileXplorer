#include "ArchiveFilesActions.h"
#include "PublicMacro.h"
#include "ImageTool.h"

ArchiveFilesActions::ArchiveFilesActions(QObject* parent) : QObject{parent} {
  COMPRESSED_HERE = new (std::nothrow) QAction{QIcon(":img/COMPRESSED"), tr("Compressd here"), this};
  COMPRESSED_TO = new (std::nothrow) QAction{tr("Compressd to"), this};

  DECOMPRESSED_TO = new (std::nothrow) QAction{tr("Decompressd to"), this};
  DECOMPRESSED_HERE = new (std::nothrow) QAction{QIcon{":img/EXTRACT_ITEM"}, tr("Decompressd here"), this};

  COMPRESSED_IMAGES = new (std::nothrow) QAction{tr("Compress Images"), this};

  ARCHIVE_PREVIEW = new (std::nothrow) QAction{QIcon(":img/COMPRESS_ITEM"), tr("Archive"), this};
  ARCHIVE_PREVIEW->setToolTip("Open the selected .qz file");
  ARCHIVE_PREVIEW->setCheckable(true);

  COMPRESSED_HERE->setToolTip("Compresse selected files to current path.\n(folders not supported now)");
  DECOMPRESSED_HERE->setToolTip("Decompressed one selected .qz files to current path.");
  COMPRESSED_IMAGES->setToolTip("Compress images in current folder and its directly sub folder.");
}

QToolBar* ArchiveFilesActions::GetArchiveTooBar(QWidget* parent) {
  QToolBar* compressToolBar = new (std::nothrow) QToolBar{"Compress/Decompress", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(compressToolBar);
  compressToolBar->addAction(COMPRESSED_HERE);
  compressToolBar->addAction(COMPRESSED_IMAGES);
  compressToolBar->addAction(DECOMPRESSED_HERE);
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
