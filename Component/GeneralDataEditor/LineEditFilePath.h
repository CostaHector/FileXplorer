#ifndef LINEEDITFILEPATH_H
#define LINEEDITFILEPATH_H

#include "LineEditPathRelated.h"

class LineEditFilePath : public LineEditPathRelated {
public:
  using LineEditPathRelated::LineEditPathRelated;
  void onActionTriggered() override;

private:
  static const QString& GetFileFilters(GeneralDataType::Type gDataType);
  static const QString& GetCaption(GeneralDataType::Type gDataType);
  static const QString& GetDialogDefaultLocation(GeneralDataType::Type gDataType);
};

#endif // LINEEDITFILEPATH_H
