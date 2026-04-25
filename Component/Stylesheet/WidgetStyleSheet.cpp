#include "WidgetStyleSheet.h"

bool WidgetStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new WidgetStyleSheet));

QString WidgetStyleSheet::GetStyleSheet(Style::StyleThemeE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleThemeE::THEME_LIGHT:
    case Style::StyleThemeE::THEME_DARK_MOON_FOG:
      styleSheet = R"(
QWidget {
    %1
    color: %2;
    background-color: %3;
    selection-background-color: #99D1FF;
    selection-color: #000000;
}

QWidget:disabled {
    color: %4;
    background-color: %5;
}

QAbstractItemView {
    background-color: %6;
}

QMainWindow::separator {
    width: 1px;
    height: 0px;
}

QDockWidget::separator {
    width: 0px;
    height: 0px;
}

#VideoPlayerPauseShieldButton {
    background: #CCFFFFFF;
    border: none;
}

QStatusBar {
    background-color: %7;
}

QProgressBar {
    border: 1px solid #CCEBFF;
    text-align: center;
}
QProgressBar:indeterminate::chunk {
}
)";
      break;
    default:
      return "";
  }
  const auto& inst = GetInst();
  QString fontStr = FontCfg::ReadFontGeneralString();                           // %1
  QString fontColor{inst.GetColorValue("Foreground/General", styleE)};          // %2
  QString bgColor{inst.GetColorValue("Background/General", styleE)};            // %3
  QString fontDisabledColor{inst.GetColorValue("Foreground/Disabled", styleE)}; // %4
  QString bgDisabledColor{inst.GetColorValue("Background/Disabled", styleE)};   // %5
  QString viewPanelBg{inst.GetColorValue("Background/View/Panel", styleE)};     // %6
  QString statusBarBg{inst.GetColorValue("Background/StatusBar", styleE)};      // %7

  return styleSheet.arg(fontStr, fontColor, bgColor, fontDisabledColor, bgDisabledColor, viewPanelBg, statusBarBg);
}
