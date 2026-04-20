#include "HeaderViewStyleSheet.h"

bool HeaderViewStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new HeaderViewStyleSheet));

QString HeaderViewStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QHeaderView::section {
    background-color: %1;
    border: 1px solid %4;
}
QHeaderView::section:horizontal {
    border-top: none;
    border-bottom: none;
    border-left: none;
}
QHeaderView::section:vertical {
    border: none;
}
QHeaderView::section:hover {
    background-color: %2;
}
QHeaderView::section:pressed {
    background-color: %3;
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
