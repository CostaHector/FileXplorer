#include "FolderPreviewActions.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

FolderPreviewActions& g_folderPreviewActions() {
  static FolderPreviewActions ins;
  return ins;
}

FolderPreviewActions::FolderPreviewActions(QObject* parent) : QObject{parent} {
  LISTS = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_LISTS"), "3-lists", this};
  BROWSER = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_BROWSER"), "browser", this};
  LABELS = new (std::nothrow) QAction{QIcon(":img/FOLDER_PREVIEW_LABELS"), "labels", this};

  LISTS->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key::Key_P));
  LISTS->setToolTip(QString("<b>%1 (%2)</b><br/> IMAGE/VIDEO/OTHER list item view.")  //
                        .arg(LISTS->text(), LISTS->shortcut().toString()));
  BROWSER->setToolTip(QString("<b>%1 (%2)</b><br/> QPlainTextEdit(images count dynamic expansion).")  //
                          .arg(BROWSER->text(), BROWSER->shortcut().toString()));
  LABELS->setToolTip(QString("<b>%1 (%2)</b><br/> QLabels(Slidershow and Periodic timer).")  //
                         .arg(LABELS->text(), LABELS->shortcut().toString()));

  PREVIEW_AG = new (std::nothrow) QActionGroup{this};
  PREVIEW_AG->addAction(LISTS);
  PREVIEW_AG->addAction(BROWSER);
  PREVIEW_AG->addAction(LABELS);
  PREVIEW_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);

  const QString& folderPreviewType = PreferenceSettings().value(MemoryKey::FOLDER_PREVIEW_TYPE.name, MemoryKey::FOLDER_PREVIEW_TYPE.v).toString();
  for (auto* ag : PREVIEW_AG->actions()) {
    ag->setCheckable(true);
    if (ag->text() == folderPreviewType) {
      ag->setChecked(true);
    }
  }
}

QToolBar* FolderPreviewActions::GetPreviewsToolbar(QWidget* parent) {
  auto* previewTB = new (std::nothrow) QToolBar{"previews toolbar", parent};
  previewTB->addActions(PREVIEW_AG->actions());
  previewTB->setOrientation(Qt::Orientation::Vertical);
  previewTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  previewTB->setStyleSheet("QToolBar { max-width: 256px; }");
  previewTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  return previewTB;
}
