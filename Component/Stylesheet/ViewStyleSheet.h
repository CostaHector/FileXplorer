#ifndef VIEWSTYLESHEET_H
#define VIEWSTYLESHEET_H

#include "StyleSheetGetter.h"

class TableViewStyleSheet: public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleSheetE styleE) const override;
private:
  static bool mRegistered;
};

class ListViewStyleSheet: public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleSheetE styleE) const override;
private:
  static bool mRegistered;
};

class TreeViewStyleSheet: public StyleSheetGetter {
public:
  QString GetStyleSheet(Style::StyleSheetE styleE) const override;
private:
  static bool mRegistered;
};

#endif
