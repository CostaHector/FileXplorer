#include "MenuStyleSheet.h"

bool MenuStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new MenuStyleSheet));

QString MenuStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
      styleSheet = R"(
QMenu {
    %1
    background-color: %2;
    border-right: 5px; /* preserve for border */
}
QMenu::item {
    background-color: transparent;
    color: #000000;
}
QMenu::item:checked {
    background-color: %3;
    color: #000000;
}
QMenu::item:selected {
    background-color: %4;
    color: #000000;
    border-right: 5px solid #3C3C3C;
}
QMenu::separator {
    height: 1px;
    background-color: %3;
}

QMenuBar {
    background-color: %2;
    border: none;
}
QMenuBar::item {
    background: transparent;
    color: #000000;
}
QMenuBar::item:checked {
    background-color: %3;
    color: #000000;
}
QMenuBar::item:selected {
    background-color: %4;
    color: #000000;
    border-right: 5px solid %5;
}
)";
      break;
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QMenu {
    %1
    background-color: %2;
    border-right: 5px; /* preserve for border */
}
QMenu::item {
    background-color: transparent;
    color: #FFFFFF;
}
QMenu::item:checked {
    background-color: %3;
    color: #000000;
}
QMenu::item:selected {
    background-color: %4;
    color: #000000;
    border-right: 5px solid %5;
}
QMenu::separator {
    height: 1px;
    background-color: %3;
}

QMenuBar {
    background-color: %2;
    border: none;
}
QMenuBar::item {
    background: transparent;
    color: #FFFFFF;
}
QMenuBar::item:checked {
    background: %3;
    color: #000000;
}
QMenuBar::item:selected {
    background: %4;
    color: #000000;
}
)";
      break;
    default:
      return "";
  }
  QString fontStr = FontCfg::ReadFontString();
  QString menuBgColor = ColorCfg::GetColorBackgroundMenu(styleE);
  QString menuCheckedColor = ColorCfg::GetColorBackgroundMenuChecked(styleE);
  QString menuSelectedColor = ColorCfg::GetColorBackgroundMenuSelected(styleE);
  QString menuBorderRightColor = ColorCfg::GetColorBorderMenuRight(styleE);
  return styleSheet.arg(fontStr, menuBgColor, menuCheckedColor, menuSelectedColor, menuBorderRightColor);
}
