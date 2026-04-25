#include "TabStyleSheet.h"

bool TabStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new TabStyleSheet));

QString TabStyleSheet::GetStyleSheet(Style::StyleThemeE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleThemeE::THEME_LIGHT:
    case Style::StyleThemeE::THEME_DARK_MOON_FOG:
      styleSheet = R"(
QTabBar {
    color: %1;
}
QTabBar::tab {
    padding-top: 2px;
    padding-bottom: 2px;
    padding-left: 15px;
    padding-right: 15px;
    min-width: 75px;
    min-height: 20px;
    font-size: %2px;
}
QTabBar::tab:!selected {
    background-color: %3;
    border-bottom: 1px solid #FFA0A0A0;
}
QTabBar::tab:selected {
    background-color: %4;
    border-top: 1px solid #FFA0A0A0;
    border-right: 1px solid #FFA0A0A0;
    border-left: 1px solid #FFA0A0A0;
    font-weight: bold;
}
QTabBar::tab:hover {
    background-color: %5;
    border: 1px solid #5E5E60;
}

QTabWidget {
    alignment: left;
}

QTabWidget::tab {
}
QTabWidget::pane {
    position: absolute;
    top: -1px; /* overlap to tab */
    border-top: 1px solid %6;
    border-bottom: 1px solid %6;
}
)";
      break;
    default:
      return "";
  }
  const auto& inst = StyleSheetGetter::GetInst();
  QString fontForegroundColor = inst.GetColorValue("Foreground/General", styleE);
  int tabFontSize = FontCfg::GetFontSizeTab();
  QString tabNotSelectedColor = inst.GetColorValue("Background/TabBar/tabNotSelected", styleE);
  QString tabSelectedColor = inst.GetColorValue("Background/TabBar/tabSelected", styleE);
  QString tabHoverColor = inst.GetColorValue("Background/TabBar/tabHover", styleE);
  QString tabWidgetBorderColor = inst.GetColorValue("Border/TabWidget", styleE);
  return styleSheet.arg(fontForegroundColor).arg(tabFontSize).arg(tabNotSelectedColor).arg(tabSelectedColor).arg(tabHoverColor).arg(tabWidgetBorderColor);
}
