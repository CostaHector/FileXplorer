#ifndef SCROLLBARSTYLESHEET_H
#define SCROLLBARSTYLESHEET_H

#include "StyleSheetGetter.h"

class ScrollBarStyleSheet : public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleSheetE styleE) const override;
private:
  static bool mRegistered;
};

#endif // SCROLLBARSTYLESHEET_H
