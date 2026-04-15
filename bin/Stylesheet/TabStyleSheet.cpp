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
    font-weight: bold; /* 选中状态加粗 */
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
    background-color: #252525;  /* 右侧空白区域背景色 */
    border: none;
}

QTabWidget::tab {
    background: #252525;
    color: #FFFFFF;
    border: 1px solid #3c3c3c;
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
    background-color: #252525;  /* 右侧空白区域背景色 */
    %1
}

QTabBar::tab {
    background-color: #252525;
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
    background-color: #3F3B39;
    border-top: 1px solid #FFA0A0A0;
    border-right: 1px solid #FFA0A0A0;
    border-left: 1px solid #FFA0A0A0;
    font-weight: bold; /* 选中状态加粗 */
}
QTabBar::tab:hover {
    background-color: #FF3E3E40;
    border: 1px solid #5e5e60;
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
