#ifndef TOOLBUTTONSTYLESHEET_H
#define TOOLBUTTONSTYLESHEET_H

#include "StyleSheetGetter.h"

class ToolButtonStyleSheet : public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleSheetE styleE) const override;
private:
  static bool mRegistered;
};

#endif // TOOLBUTTONSTYLESHEET_H
