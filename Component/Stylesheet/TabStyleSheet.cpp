#include "TabStyleSheet.h"

bool TabStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new TabStyleSheet));

QString TabStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
      styleSheet = R"(
QTabWidget {
    border: none;
}
QTabWidget::tab {
    border-bottom: none;
}
QTabWidget::pane {
    background-color: #FFFFFF;
    border-top: 1px solid #FFA0A0A0; /* tabWidget addTab top */
    position: absolute;
    top: -1px; /* overlap to tab */
    border-bottom: 1px solid #FFA0A0A0;  /* tabWidget addTab bottom */
}

QTabBar {
    %1
}
QTabBar::tab {
    padding-top: 2px;
    padding-bottom: 2px;
    padding-left: 15px;
    padding-right: 15px;
    min-width: 75px;
    min-height: 20px;
    font-size: %2px;
}
QTabBar::tab:!selected {
    border-bottom: 1px solid #FFA0A0A0;
}
QTabBar::tab:selected {
    background-color: #FFFFFF;
    border-top: 1px solid #FFA0A0A0;
    border-right: 1px solid #FFA0A0A0;
    border-left: 1px solid #FFA0A0A0;
    font-weight: bold;
}
QTabBar::tab:hover {
    background-color: #FFCDE8FF;
}
QTabBar::tab:selected:hover {
    background-color: #FFF0F0F0;
}
)";
      break;
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QTabWidget {
    background-color: #3C3C3C;
    border: none;
}
QTabWidget::tab {
    background: #3C3C3C;
    color: #FFFFFF;
    border: 1px solid #3C3C3C;
    border-bottom: none;
}
QTabWidget::pane {
    background-color: rgb(255, 255, 255);
    border-top: 1px solid #FFA0A0A0; /* tabWidget addTab top */
    position: absolute;
    top: -1px; /* overlap to tab */
    border-bottom: 1px solid #FFA0A0A0;  /* tabWidget addTab bottom */
}

QTabBar {
    background-color: #3C3C3C;
    %1
}
QTabBar::tab {
    background-color: #3C3C3C;
    padding-top: 2px;
    padding-bottom: 2px;
    padding-left: 15px;
    padding-right: 15px;
    min-width: 75px;
    min-height: 20px;
    font-size: %2px;
}
QTabBar::tab:!selected {
    background-color: #3C3C3C;
    border-bottom: 1px solid #FFA0A0A0;
}
QTabBar::tab:selected {
    background-color: #3C3C3C;
    border-top: 1px solid #FFA0A0A0;
    border-right: 1px solid #FFA0A0A0;
    border-left: 1px solid #FFA0A0A0;
    font-weight: bold;
}
QTabBar::tab:hover {
    background-color: #FF3E3E40;
    border: 1px solid #5E5E60;
}
QTabBar::tab:selected:hover {
    background-color: #CC1E1E1E;
}
)";
      break;
    default:
      return "";
  }
  QString fontStr = FontCfg::ReadFontString();
  return styleSheet.arg(fontStr).arg(FontCfg::GetFontSizeTab());
}
