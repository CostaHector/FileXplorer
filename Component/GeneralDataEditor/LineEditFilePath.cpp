#include "LineEditFilePath.h"
#include "PublicVariable.h"
#include "NotificatorMacro.h"
#include <QFileDialog>

void LineEditFilePath::onActionTriggered() {
  const QFileDialog::Options parmOptions{QFileDialog::Option::DontUseNativeDialog};
  QString fileAbsPath = QFileDialog::getOpenFileName(this,
                                                   GetCaption(type()),
                                                   GetDialogDefaultLocation(type()),
                                                   GetFileFilters(type()), //
                                                   nullptr,
                                                   parmOptions);
  if (fileAbsPath.isEmpty()) {
    LOG_INFO_NP("Skip", "User cancel select an file");
    return;
  }
  setText(normalizePath(fileAbsPath));
}

const QString& LineEditFilePath::GetFileFilters(GeneralDataType::Type gDataType) {
  switch (gDataType) {
    case GeneralDataType::Type::IMAGE_PATH_OPTIONAL: {
      static const QString imgFilters = []() -> QString {
        QString filtersStr;
        filtersStr.reserve(100);
        filtersStr += "Image Files (";
        filtersStr += TYPE_FILTER::IMAGE_TYPE_SET.join(' ');
        filtersStr += ")";
        return filtersStr;
      }();
      return imgFilters;
    }
    default:
      static const QString filters;
      return filters;
  }
}

const QString& LineEditFilePath::GetCaption(GeneralDataType::Type gDataType) {
  switch (gDataType) {
    case GeneralDataType::Type::IMAGE_PATH_OPTIONAL: {
      static const QString imgFileCaption{"Select an image file"};
      return imgFileCaption;
    }
    default:
      static const QString fileCaption{"Select an file"};
      return fileCaption;
  }
}

const QString& LineEditFilePath::GetDialogDefaultLocation(GeneralDataType::Type gDataType) {
  switch (gDataType) {
    case GeneralDataType::Type::IMAGE_PATH_OPTIONAL: {
      static const QString dialogImageDefaultLocation{SystemPath::HOME_PATH() + "/Pictures"};
      return dialogImageDefaultLocation;
    }
    default:
      static const QString dialogDefaultLocation{SystemPath::HOME_PATH()};
      return dialogDefaultLocation;
  }
}
