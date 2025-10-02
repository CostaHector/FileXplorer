#include "StyleSheet.h"
#include "MemoryKey.h"
#include "PreferenceActions.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>

namespace StyleSheet {
void UpdateTitleBar(QWidget* widget) {
  CHECK_NULLPTR_RETURN_VOID(widget);
  if (g_PreferenceActions().CurStyleSheet() == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
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
    LOG_D("Not support dark mode now");
  }
}
void setDarkTitleBar(QWidget* widget, bool enable) {
  if (enable) {
    LOG_D("Not support dark mode now");
  }
}
#endif
}  // namespace StyleSheet

constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_16;
constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_24;
constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_48;
constexpr QSize IMAGE_SIZE::ICON_SIZE_CANDIDATES[];
constexpr int IMAGE_SIZE::ICON_SIZE_CANDIDATES_N;

QString IMAGE_SIZE::HumanReadFriendlySize(int scaleIndex, bool* isValidScaledIndex) {
  if (scaleIndex < 0 || scaleIndex >= IMAGE_SIZE::ICON_SIZE_CANDIDATES_N) {
    if (isValidScaledIndex != nullptr) {
      *isValidScaledIndex = false;
    }
    return QString::asprintf("[%d] out of range[0, %d)", scaleIndex, ICON_SIZE_CANDIDATES_N);
  }
  if (isValidScaledIndex != nullptr) {
    *isValidScaledIndex = true;
  }
  return QString::asprintf("[%d] %d-by-%d", scaleIndex, //
                           ICON_SIZE_CANDIDATES[scaleIndex].width(), ICON_SIZE_CANDIDATES[scaleIndex].height());
}

void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align) {
  if (lay == nullptr) {
    return;
  }
  // Only QToolBar and QToolButton need to set alignment. (QWidget like QSeperator not need)
  for (int i = 0; i < lay->count(); ++i) {
    QLayoutItem* item = lay->itemAt(i);
    if (item == nullptr || item->widget() == nullptr) {
      continue;
    }
    if (item->widget()->metaObject()->className() == QLatin1String("QToolBarSeparator")) {
      continue;
    }
    item->setAlignment(align);
  }
}
