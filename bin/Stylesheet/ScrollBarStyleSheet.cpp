#include "ScrollBarStyleSheet.h"

bool ScrollBarStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new ScrollBarStyleSheet));

QString ScrollBarStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
      return R"(
QScrollBar:vertical {
    width: 12px;
    background: #323232;
    border-radius: 3px;
    margin: 0px 0 0px 0;
}

QScrollBar:horizontal {
    height: 12px;
    background: #323232;
    border-radius: 3px;
    margin: 0 0px 0 0px;
}
QScrollBar::handle {
    border-radius: 4px;
    background: #DADADA;
}

QScrollBar::handle:hover {
    background: #CCEBFF;
}

QScrollBar::handle:pressed {
    background: #99D1FF;
}

QScrollBar::add-page, QScrollBar::sub-page {
    background: #FFFFFF;
}
)";
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      return R"(
QScrollBar:vertical {
    width: 12px;
    background: #323232;
    border-radius: 3px;
    margin: 0px 0 0px 0;
}
QScrollBar:horizontal {
    height: 12px;
    background: #323232;
    border-radius: 3px;
    margin: 0 0px 0 0px;
}
QScrollBar::handle {
    border-radius: 4px;
    background: #c0c0c0;
}

QScrollBar::handle:hover {
    background: #4e4e50;
}

QScrollBar::handle:pressed {
    background: #5e5e60;
}

QScrollBar::add-page, QScrollBar::sub-page {
    background: #252525;
}
)";
    default:
      return "";
  }
}
