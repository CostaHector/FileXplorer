#include "StyleSheet.h"
#include "PreferenceActions.h"
#include "PublicMacro.h"
#include "StyleEnum.h"
#include <QApplication>
#include <QWidget>
#include <QLayout>
#include <QStyle>
#include <unordered_set>

namespace StyleSheet {
void UpdateTitleBar(QWidget* widget) {
  CHECK_NULLPTR_RETURN_VOID(widget);
  if (g_PreferenceActions().CurStyleTheme() == Style::StyleThemeE::THEME_DARK_MOON_FOG) {
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
  SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
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

void UpdateApplyPushButton(QWidget* pushButton) {
  if (pushButton == nullptr) {
    return;
  }
  pushButton->setProperty("applyButtonFlag", true);
  pushButton->style()->unpolish(pushButton);
  pushButton->style()->polish(pushButton);
}

}  // namespace StyleSheet

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
