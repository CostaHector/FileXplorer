#include "ImageViewerActions.h"
#include "ImageViewerKey.h"
#include "Configuration.h"

ImageViewerActions &ImageViewerActions::GetInst() {
  static ImageViewerActions inst;
  return inst;
}

ImageViewerActions::ImageViewerActions(QObject *parent) : QObject{parent} {
  NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT = new (std::nothrow) QAction{QIcon{":img/INCLUDEING_SUBDIRS"}, "Include Subfolders"};
  NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT->setCheckable(true);
  NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT->setChecked(getConfig(ImageViewerKey::NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE).toBool());
  NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT->setToolTip(QString{"<b>%1 (%2)</b><br/>Include files inside subfolders when navigating to the previous or next image."}
                                                           .arg(NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT->text(), NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT->shortcut().toString()));

  RECOGNIZE_FACE_ON_SELECTION_COMPLETE_ACT = new (std::nothrow) QAction(QIcon{":img/FACE_RECOGNIZE_AUTO"}, "Auto Recognize Face");
  RECOGNIZE_FACE_ON_SELECTION_COMPLETE_ACT->setCheckable(true);
  RECOGNIZE_FACE_ON_SELECTION_COMPLETE_ACT->setChecked(getConfig(ImageViewerKey::RECOGNIZE_FACE_ON_SELECTION_COMPLETE).toBool());
  RECOGNIZE_FACE_ON_SELECTION_COMPLETE_ACT->setToolTip(QString{"<b>%1 (%2)</b><br/>Automatically recognize faces immediately after making a selection."}
                                        .arg(RECOGNIZE_FACE_ON_SELECTION_COMPLETE_ACT->text(), RECOGNIZE_FACE_ON_SELECTION_COMPLETE_ACT->shortcut().toString()));

  AUTO_SAVE_ON_SELECTION_COMPLETE_ACT = new (std::nothrow) QAction(QIcon{":img/AUTO_SAVE"}, "Auto Save on Selection");
  AUTO_SAVE_ON_SELECTION_COMPLETE_ACT->setCheckable(true);
  AUTO_SAVE_ON_SELECTION_COMPLETE_ACT->setChecked(getConfig(ImageViewerKey::AUTO_SAVE_ON_SELECTION_COMPLETE).toBool());
  AUTO_SAVE_ON_SELECTION_COMPLETE_ACT->setToolTip(QString{"<b>%1 (%2)</b><br/>Automatically prompt the save dialog after making a selection to name and save the cropped image."}
                                                      .arg(AUTO_SAVE_ON_SELECTION_COMPLETE_ACT->text(), AUTO_SAVE_ON_SELECTION_COMPLETE_ACT->shortcut().toString()));

  SILENT_SAVE_ON_AUTO_SAVE_ACT = new (std::nothrow) QAction(QIcon{":img/SILENT_SAVE"}, "Save Silently (Skip Dialog)");
  SILENT_SAVE_ON_AUTO_SAVE_ACT->setCheckable(true);
  SILENT_SAVE_ON_AUTO_SAVE_ACT->setChecked(getConfig(ImageViewerKey::SILENT_SAVE_ON_AUTO_SAVE).toBool());
  SILENT_SAVE_ON_AUTO_SAVE_ACT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  SILENT_SAVE_ON_AUTO_SAVE_ACT->setToolTip(QString{"<b>%1 (%2)</b><br/>Skip the save dialog and automatically save the cropped image using the default name."}
                                               .arg(SILENT_SAVE_ON_AUTO_SAVE_ACT->text(), SILENT_SAVE_ON_AUTO_SAVE_ACT->shortcut().toString()));

  subscribe();
}

void ImageViewerActions::subscribe() {
  connect(NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE_ACT, &QAction::toggled, [](bool bChecked) {
    setConfig(ImageViewerKey::NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE, bChecked);
  });
  connect(RECOGNIZE_FACE_ON_SELECTION_COMPLETE_ACT, &QAction::toggled, [](bool bChecked) {
    setConfig(ImageViewerKey::RECOGNIZE_FACE_ON_SELECTION_COMPLETE, bChecked);
  });
  connect(AUTO_SAVE_ON_SELECTION_COMPLETE_ACT, &QAction::toggled, [](bool bChecked) {
    setConfig(ImageViewerKey::AUTO_SAVE_ON_SELECTION_COMPLETE, bChecked);
  });
  connect(SILENT_SAVE_ON_AUTO_SAVE_ACT, &QAction::toggled, [](bool bChecked) {
    setConfig(ImageViewerKey::SILENT_SAVE_ON_AUTO_SAVE, bChecked);
  });
}
