﻿#include "FolderPreviewActions.h"
#include "public/MemoryKey.h"
#include "public/PublicVariable.h"

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
