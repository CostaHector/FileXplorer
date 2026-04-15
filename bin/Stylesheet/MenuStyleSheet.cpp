#include "MenuStyleSheet.h"

bool MenuStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new MenuStyleSheet));

QString MenuStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
      styleSheet = R"(
QMenu {
    background-color: #FFFFFF;
    border-right: 3px solid transparent; /* preserve for border */
    %1
}
QMenu::item {
    background-color: transparent;
    color: #000000;
}
QMenu::item:checked {
    background-color: #CCEBFF;
    color: #000000;
}
QMenu::item:selected {
    background-color: #CCEBFF;
    color: #000000;
    border-right: 5px solid #3C3C3C;
}
QMenu::separator {
    height: 1px;
    background-color: #CCEBFF;
}

QMenuBar {
    background-color: #FFFFFF;
    border: none;
}
QMenuBar::item {
    background: transparent;
    color: #000000;
}
QMenuBar::item:selected {
    background-color: #CCEBFF;
    color: #000000;
}
)";
      break;
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QMenu {
    background-color: #333333;
    border-right: 3px solid #444444; /* preserve for border */
    %1
}
QMenu::item {
    background-color: transparent;
    color: #FFFFFF;
}
QMenu::item:checked {
    background-color: #5F5F5F;
    color: #FFFFFF;
}
QMenu::item:selected {
    background-color: #5F5F5F;
    color: #FFFFFF;
    border-right: 5px solid #CCEBFF;
}
QMenu::separator {
    height: 1px;
    background-color: #CCEBFF;
}

QMenuBar {
    background-color: #333333;
    border: none;
}
QMenuBar::item {
    background: transparent;
    color: #FFFFFF;
}
QMenuBar::item:selected {
    background: #5F5F5F;
    color: #FFFFFF;
}
)";
      break;
    default:
      return "";
  }
  QString fontStr = FontCfg::ReadFontString();
  return styleSheet.arg(fontStr);
}
