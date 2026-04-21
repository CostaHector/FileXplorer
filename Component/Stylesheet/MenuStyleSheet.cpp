#include "MenuStyleSheet.h"

bool MenuStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new MenuStyleSheet));

QString MenuStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QMenu {
    color: %1;
    background-color: %2;
    border: 1px solid %7;
}
QMenu::item {
    background-color: transparent;
    border-right: 5px solid transparent;
}
QMenu::item:checked {
    background-color: %3;
}
QMenu::item:selected {
    background-color: %4;
    border-right: 5px solid %5;
}
QMenu::separator {
    height: 1px;
    background-color: %6;
}

QMenuBar {
    color: %1;
    background-color: %2;
}
QMenuBar::item {
    background: transparent;
}
QMenuBar::item:checked {
    background-color: %3;
}
QMenuBar::item:selected {
    background-color: %4;
    border: 1px solid %5;
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

  QString menuItemRightBorder = inst.GetColorValue("Background/Menu/ItemRightBorder", styleE);
  QString separatorColor = inst.GetColorValue("Background/Menu/Separator", styleE);
  QString menuBorder = inst.GetColorValue("Background/Menu/Border", styleE);

  return styleSheet.arg(menuFontForeground, menuBg, menuCheckedColor, menuSelectedColor, menuItemRightBorder, separatorColor, menuBorder);
}
