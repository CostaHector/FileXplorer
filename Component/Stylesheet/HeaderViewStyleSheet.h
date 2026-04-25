#ifndef HEADERVIEWSTYLESHEET_H
#define HEADERVIEWSTYLESHEET_H

#include "StyleSheetGetter.h"

class HeaderViewStyleSheet : public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleThemeE styleE) const override;
private:
  static bool mRegistered;
};

#endif // HEADERVIEWSTYLESHEET_H
