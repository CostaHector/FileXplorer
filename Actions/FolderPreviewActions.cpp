#include "FolderPreviewActions.h"

#include "PublicVariable.h"

FolderPreviewActions& g_folderPreviewActions() {
  static FolderPreviewActions ins;
  return ins;
}

FolderPreviewActions::FolderPreviewActions(QObject* parent) : QObject{parent} {
  LISTS = {new QAction{QIcon(":img/FOLDER_PREVIEW_LISTS"), "3-lists", this}};
  BROWSER = {new QAction{QIcon(":img/FOLDER_PREVIEW_BROWSER"), "browser", this}};
  LABELS = {new QAction{QIcon(":img/FOLDER_PREVIEW_LABELS"), "labels", this}};

  PREVIEW_AG->addAction(LISTS);
  PREVIEW_AG->addAction(BROWSER);
  PREVIEW_AG->addAction(LABELS);
  PREVIEW_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  LISTS->setToolTip(QString("<b>%1 (%2)</b><br/> IMAGE/VIDEO/OTHER list item view.")  //
                        .arg(LISTS->text(), LISTS->shortcut().toString()));
  BROWSER->setToolTip(QString("<b>%1 (%2)</b><br/> QPlainTextEdit(images count dynamic expansion).")  //
                          .arg(BROWSER->text(), BROWSER->shortcut().toString()));
  LABELS->setToolTip(QString("<b>%1 (%2)</b><br/> QLabels(Slidershow and Periodic timer).")  //
                         .arg(LABELS->text(), LABELS->shortcut().toString()));

  const QString& folderPreviewType = PreferenceSettings().value(MemoryKey::FOLDER_PREVIEW_TYPE.name, MemoryKey::FOLDER_PREVIEW_TYPE.v).toString();
  for (auto* ag : PREVIEW_AG->actions()) {
    ag->setCheckable(true);
    if (ag->text() == folderPreviewType) {
      ag->setChecked(true);
    }
  }
}
