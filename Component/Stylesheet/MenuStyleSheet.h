#ifndef MENUSTYLESHEET_H
#define MENUSTYLESHEET_H

#include "StyleSheetGetter.h"

class MenuStyleSheet : public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleThemeE styleE) const override;
private:
  static bool mRegistered;
};

#endif // MENUSTYLESHEET_H
