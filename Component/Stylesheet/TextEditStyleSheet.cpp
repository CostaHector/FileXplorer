#include "TextEditStyleSheet.h"

bool TextEditStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new TextEditStyleSheet));

QString TextEditStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QPlainTextEdit, QTextEdit {
    selection-background-color: #99D1FF;
    selection-color: #000000;
    background-color: %1;
}
QPlainTextEdit[UseCodeFontFamily="true"] {
    font-family: "%2";
}
QTextEdit[UseCodeFontFamily="true"] {
    font-family: "%2";
}
)";
      break;
    default:
      return "";
  }
  const auto& inst = StyleSheetGetter::GetInst();
  QString viewPanelBg{inst.GetColorValue("Background/View/Panel", styleE)};
  QString codeFontFamily{FontCfg::GetFontFamilyCode()};
  return styleSheet.arg(viewPanelBg, codeFontFamily);
}
