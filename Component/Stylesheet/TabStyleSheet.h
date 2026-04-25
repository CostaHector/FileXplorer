#ifndef TABSTYLESHEET_H
#define TABSTYLESHEET_H

#include "StyleSheetGetter.h"

class TabStyleSheet : public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleThemeE styleE) const override;
private:
  static bool mRegistered;
};

#endif // TABSTYLESHEET_H
