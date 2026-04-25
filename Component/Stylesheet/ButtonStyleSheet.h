#ifndef BUTTONSTYLESHEET_H
#define BUTTONSTYLESHEET_H

#include "StyleSheetGetter.h"

class ButtonStyleSheet : public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleThemeE styleE) const override;
private:
  static bool mRegistered;
};

#endif // BUTTONSTYLESHEET_H
