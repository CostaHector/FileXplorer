#include "MenuStyleSheet.h"

bool MenuStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new MenuStyleSheet));

QString MenuStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QMenu {
    border-right: 5px; /* preserve for border */
    background-color: %2; /* background color */
}
QMenu::item {
    color: %1; /* foreground font color */
    background-color: transparent;
}
QMenu::item:checked {
    background-color: %3; /* checked bg color */
}
QMenu::item:selected {
    background-color: %4; /* hovered bg color */
    border-right: 5px solid %5; /* hovered border color */
}
QMenu::separator {
    height: 1px;
    background-color: %6; /* separator color */
}

QMenuBar {
    border: none;
    background-color: %2; /* background color */
}
QMenuBar::item {
    color: %1; /* foreground font color */
    background: transparent;
}
QMenuBar::item:checked {
    background-color: %3; /* checked bg color */
}
QMenuBar::item:selected {
    background-color: %4; /* hovered bg color */
    border: 1px solid %5; /* hovered border color */
}
)";
      break;
    default:
      return "";
  }
  const auto& inst = GetInst();
  QString menuFontForeground = inst.GetColorValue("Foreground/MenuFont", styleE);
  QString menuBg = inst.GetColorValue("Background/Menu/Item", styleE);
  QString menuCheckedColor = inst.GetColorValue("Background/Menu/ItemChecked", styleE);
  QString menuSelectedColor = inst.GetColorValue("Background/Menu/ItemHovered", styleE);
  QString menuBorderRightColor = inst.GetColorValue("Border/MenuRight", styleE);
  QString separatorColor = inst.GetColorValue("Background/Menu/Separator", styleE);
  
  return styleSheet.arg(menuFontForeground, menuBg, menuCheckedColor, menuSelectedColor, menuBorderRightColor, separatorColor);
}
