#ifndef TOOLBARSTYLESHEET_H
#define TOOLBARSTYLESHEET_H

#include "StyleSheetGetter.h"

class ToolBarStyleSheet : public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleSheetE styleE) const override;
private:
  static bool mRegistered;
};

#endif // TOOLBARSTYLESHEET_H
