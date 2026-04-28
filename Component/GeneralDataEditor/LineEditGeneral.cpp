#include "LineEditGeneral.h"
#include "LineEditColor.h"
#include "LineEditFilePath.h"
#include "LineEditFolderPath.h"
#include "Logger.h"
#include <QAction>

LineEditGeneral *LineEditGeneral::create(int generalDataType, QWidget *parent) {
  GeneralDataType::Type typeE = static_cast<GeneralDataType::Type>(generalDataType);
  switch (typeE) {
    case GeneralDataType::Type::COLOR:
      return new LineEditColor{typeE, ":/styles/COLOR_SELECT", parent};
    case GeneralDataType::Type::FILE_PATH:
    case GeneralDataType::Type::IMAGE_PATH_OPTIONAL:
      return new LineEditFilePath{typeE, ":img/FILE", parent};
    case GeneralDataType::Type::FOLDER_PATH:
      return new LineEditFolderPath{typeE, ":img/FOLDER", parent};
    default:
      LOG_E("type[%d] cannot edit in QLineEdit", typeE);
      return nullptr;
  }
}

LineEditGeneral::LineEditGeneral(GeneralDataType::Type gDataType, const char *iconUrl, QWidget *parent)
  : QLineEdit{parent}
  , mDataType{gDataType} {
  // no need action when iconUrl is nullptr
  if (iconUrl == nullptr) {
    return;
  }
  QAction *act = addAction(QIcon{iconUrl}, QLineEdit::LeadingPosition);
  connect(act, &QAction::triggered, this, &LineEditGeneral::onActionTriggered);
}
