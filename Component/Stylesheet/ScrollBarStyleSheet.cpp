#include "ScrollBarStyleSheet.h"

bool ScrollBarStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new ScrollBarStyleSheet));

QString ScrollBarStyleSheet::GetStyleSheet(Style::StyleThemeE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleThemeE::THEME_LIGHT:
    case Style::StyleThemeE::THEME_DARK_MOON_FOG:
      styleSheet = R"(
QScrollBar {
    border-radius: 3px;
    margin: 0px 0px 0px 0px;
    background-color: %1;
}
QScrollBar:vertical {
    width: 12px;
}
QScrollBar:horizontal {
    height: 12px;
}
QScrollBar::handle {
    border-radius: 4px;
    background-color: %2;
}
QScrollBar::handle:hover {
    background-color: %3;
}
QScrollBar::handle:pressed {
    background-color: %3;
}
QScrollBar::add-page, QScrollBar::sub-page {
    background-color: %1;
}
)";
      break;
    default:
      return "";
  }
  //  sub-line  sub-page  handle  add-page  add-line
  const auto& inst = GetInst();
  QString emptyZoneColor = inst.GetColorValue("Background/ScrollBar/EmptyZone", styleE);
  QString handleColor = inst.GetColorValue("Background/ScrollBar/Handle", styleE);
  QString checkedColor = inst.GetColorValue("Background/ScrollBar/Checked", styleE);
  return styleSheet.arg(emptyZoneColor, handleColor, checkedColor);
}
