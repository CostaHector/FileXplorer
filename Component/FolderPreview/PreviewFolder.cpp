#include "PreviewFolder.h"

#include "public/PublicVariable.h"

constexpr int PreviewFolder::NEXT_FOLDER_TIME_INTERVAL;        // ms

QSize PreviewFolder::sizeHint() const {
  auto w = PreferenceSettings().value("dockerFolderPreviewWidth", DOCKER_DEFAULT_SIZE.width()).toInt();
  auto h = PreferenceSettings().value("dockerFolderPreviewHeight", DOCKER_DEFAULT_SIZE.height()).toInt();
  return QSize(w, h);
}
