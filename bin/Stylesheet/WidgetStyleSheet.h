#ifndef WIDGETSTYLESHEET_H
#define WIDGETSTYLESHEET_H

#include "StyleSheetGetter.h"

class WidgetStyleSheet : public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleSheetE styleE) const override;
private:
  static bool mRegistered;
};


#endif // WIDGETSTYLESHEET_H
