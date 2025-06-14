#ifndef STYLESHEET_H
#define STYLESHEET_H
#include "PublicMacro.h"
#include <QString>
#include <QRect>
#include <QSize>

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

const char SUBMIT_BTN_STYLE[] =
    "QPushButton{"
    "    color: #fff;"
    "    background-color: DodgerBlue;"
    "    border-color: DodgerBlue;"
    "}"
    "QPushButton:hover {"
    "    color: #fff;"
    "    background-color: rgb(36, 118, 199);"
    "    border-color: rgb(36, 118, 199);"
    "}";
}  // namespace StyleSheet

constexpr QRect DEFAULT_GEOMETRY{0, 0, 1024, 768};
constexpr QSize DOCKER_DEFAULT_SIZE{DEFAULT_GEOMETRY.width() / 2, DEFAULT_GEOMETRY.height()};

struct IMAGE_SIZE {
  static constexpr int TABS_ICON_IN_MENU_16 = 16;
  static constexpr int TABS_ICON_IN_MENU_24 = 24;
  static constexpr int TABS_ICON_IN_MENU_48 = 48;
  static int IMG_WIDTH;
  static int IMG_HEIGHT;
};

#endif  // STYLESHEET_H
