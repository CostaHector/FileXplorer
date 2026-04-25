#include "HeaderViewStyleSheet.h"

bool HeaderViewStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new HeaderViewStyleSheet));

QString HeaderViewStyleSheet::GetStyleSheet(Style::StyleThemeE styleE) const {
// padding: 2px 0px 2px 2px; /* top right bottom left */
  QString styleSheet;
  switch (styleE) {
    case Style::StyleThemeE::THEME_LIGHT:
    case Style::StyleThemeE::THEME_DARK_MOON_FOG:
      styleSheet = R"(
QHeaderView::section {
    border: none;
    background-color: %1;
}
QHeaderView::section:hover {
    background-color: %2;
}
QHeaderView::section:pressed {
    background-color: %3;
}
QHeaderView::section:horizontal {
    border-right: 1px solid %4;
    padding: 5px 0px 0px 5px;
}
QHeaderView::section:vertical {
}
)";
      break;
    default:
      return "";
  }

  const auto& inst = GetInst();
  QString notHoverColor{inst.GetColorValue("Background/HeaderView/NotHover", styleE)};
  QString hoverColor{inst.GetColorValue("Background/HeaderView/Hover", styleE)};
  QString pressedColor{inst.GetColorValue("Background/HeaderView/Pressed", styleE)};
  QString borderColor{inst.GetColorValue("Border/HeaderView", styleE)};

  return styleSheet.arg(notHoverColor, hoverColor, pressedColor, borderColor);
}
