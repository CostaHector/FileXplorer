#include "FolderPreviewActions.h"

#include "PublicVariable.h"

FolderPreviewActions& g_folderPreviewActions() {
  static FolderPreviewActions ins;
  return ins;
}

FolderPreviewActions::FolderPreviewActions(QObject* parent) : QObject{parent} {
  PREVIEW_AG->addAction(LISTS);
  PREVIEW_AG->addAction(BROWSER);
  PREVIEW_AG->addAction(LABELS);
  PREVIEW_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  const QString& folderPreviewType = PreferenceSettings().value(MemoryKey::FOLDER_PREVIEW_TYPE.name, MemoryKey::FOLDER_PREVIEW_TYPE.v).toString();
  for (auto* ag : PREVIEW_AG->actions()) {
    ag->setCheckable(true);
    if (ag->text() == folderPreviewType) {
      ag->setChecked(true);
    }
  }
}
