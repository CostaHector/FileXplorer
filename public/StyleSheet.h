#ifndef STYLESHEET_H
#define STYLESHEET_H
#include <QString>
#include "PublicMacro.h"

class QWidget;

namespace StyleSheet {
constexpr int ROW_SECTION_HEIGHT = 10;

#define STYLE_ITEMS_MAPPING \
  STYLE_ITEM(DEFAULT, 0)    \
  STYLE_ITEM(DARK, 1)

enum THEME {
  THEME_BEGIN = 0,
#define STYLE_ITEM(enu, val) enu = val,
  STYLE_ITEMS_MAPPING
#undef STYLE_ITEM
      THEME_BUTT
};

const QString THEME_2_STRING[THEME_BUTT]{
#define STYLE_ITEM(enu, val) ENUM_2_STR(enu),
    STYLE_ITEMS_MAPPING
#undef STYLE_ITEM
};

THEME CurrentTheme(const THEME* const pNewTheme = nullptr);
THEME GetThemeFromString(const QString& themeStr);
void UpdateTitleBar(QWidget* widget);
void setDarkTitleBar(QWidget* widget, bool enable);
void setGlobalDarkMode(bool enable);
}  // namespace StyleSheet

#endif  // STYLESHEET_H
