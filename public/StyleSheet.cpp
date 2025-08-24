#include "StyleSheet.h"
#include "MemoryKey.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QMap>
#include <QString>

namespace StyleSheet {
THEME CurrentTheme(const THEME* const pNewTheme) {
  static int themeInt = Configuration().value("STYLESHEET_NAME", THEME::DEFAULT).toInt();
  if (pNewTheme != nullptr) {
    themeInt = *pNewTheme;
  }
  if (themeInt < THEME_BEGIN || themeInt >= THEME_BUTT) {
    return THEME_BEGIN;
  }
  return (THEME)themeInt;
}

THEME GetThemeFromString(const QString& themeStr) {
  static const QMap<QString, THEME> string2Theme{
#define STYLE_ITEM(enu, val) {ENUM_2_STR(enu), enu},
      STYLE_ITEMS_MAPPING
#undef STYLE_ITEM
  };
  return string2Theme.value(themeStr, THEME::THEME_BEGIN);
};

void UpdateTitleBar(QWidget* widget) {
  if (CurrentTheme() == THEME::DARK) {
    setDarkTitleBar(widget, true);
  }
}

#if defined(Q_OS_WIN)
#include <windows.h>
#include <dwmapi.h>  // make sure "dwm.exe" already running in task manager at first
void setDarkTitleBar(QWidget* widget, bool enable) {
  if (widget == nullptr) {
    return;
  }
  HWND hwnd = reinterpret_cast<HWND>(widget->winId());
  BOOL darkMode = enable;
  // 19: DWMWA_USE_IMMERSIVE_DARK_MODE_OLD <- Windows 10 Build 18368.418（19H1）
  // 20: DWMWA_USE_IMMERSIVE_DARK_MODE
  DwmSetWindowAttribute(hwnd, 19 /*DWMWA_USE_IMMERSIVE_DARK_MODE*/, &darkMode, sizeof(darkMode));
}

void setGlobalDarkMode(bool enable) {
  foreach (QWidget* widget, qApp->topLevelWidgets()) {
    setDarkTitleBar(widget, enable);
  }
}
#else
void setGlobalDarkMode(bool enable) {
  if (enable) {
    qDebug("Not support dark mode now");
  }
}
void setDarkTitleBar(QWidget* widget, bool enable) {
  if (enable) {
    qDebug("Not support dark mode now");
  }
}
#endif
}  // namespace StyleSheet

constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_16;
constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_24;
constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_48;
int IMAGE_SIZE::IMG_WIDTH = 420;
int IMAGE_SIZE::IMG_HEIGHT = 320;
constexpr int IMAGE_SIZE::PORTAIT_IMG_WIDTH;

void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align) {
  if (lay == nullptr) {
    return;
  }
  // Only QToolBar and QToolButton need to set alignment. (QWidget like QSeperator not need)
  for (int i = 0; i < lay->count(); ++i) {
    lay->itemAt(i)->setAlignment(align);
  }
}
