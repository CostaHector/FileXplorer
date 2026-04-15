#include "HeaderViewStyleSheet.h"

bool HeaderViewStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new HeaderViewStyleSheet));

QString HeaderViewStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
      styleSheet = R"(
QHeaderView::section {

}
QHeaderView#Horizontal::section {
    %1
}
QHeaderView#Vertical::section {
    %2
}
)";
      break;
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QHeaderView::section {
    background-color: #3F3B39;
    color: #FFFFFF;
}
QHeaderView#Horizontal::section {
    %1
}
QHeaderView#Vertical::section {
    %2
}
)";
      break;
    default:
      return "";
  }
  QString fontStr = FontCfg::ReadFontString();
  return styleSheet.arg(fontStr, fontStr);
}
