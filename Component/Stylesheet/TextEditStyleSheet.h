#ifndef TEXTEDITSTYLESHEET_H
#define TEXTEDITSTYLESHEET_H

#include "StyleSheetGetter.h"

class TextEditStyleSheet : public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleThemeE styleE) const override;
private:
  static bool mRegistered;
};

#endif // TEXTEDITSTYLESHEET_H
