#include "WidgetStyleSheet.h"

bool WidgetStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new WidgetStyleSheet));

QString WidgetStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QWidget {
    background-color: %1;
    color: %2;
    selection-background-color: #99D1FF;
    selection-color: #000000;
}
QWidget[isToolBar="true"] {
    background-color: %1;
    color: %2;
}

QMainWindow {
    background-color: %1;
}

QMainWindow::separator {
    width: 1px;
    height: 0px;
}

QDockWidget::separator {
    width: 0px;
    height: 0px;
}

QPlainTextEdit, QTextEdit {
    background-color: %1;
    color: %2;
    selection-background-color: #99D1FF;
    selection-color: #000000;
}

QMainWindow QStackedWidget {
    background-color: %1;
}

QStackedWidget QAbstractItemView {
    background-color: %1;
}

QPushButton {
    background-color: %1;
    border: 1px solid #CCEBFF;
    min-width: 80px;
}
QPushButton:hover {
    background-color: #CCEBFF;
    color: #000000;
}

QLineEdit {
    background-color: %1;
    border: 1px solid #CCEBFF;
    margin: 1px;
    color: %2;
}

QStatusBar, QRadioButton, QCheckBox {
    background-color: %1;
    color: %2;
}

QLabel {
    background-color: %1;
    color: %2;
}
/* For Custom Status Bar Message Label Only. 0: normal, 1: abnormal */
QLabel#statusMessageLabel[alertLevel="0"] {
    font-weight: normal;
}
QLabel#statusMessageLabel[alertLevel="1"] {
    color: red;
    font-weight: bold;
}

QToolBar {
    background: %1;
    border: 1px solid #CCEBFF;
}

QToolBar QToolBar {
    margin: 0px;
    border: 0px;
}
QToolBar QComboBox {
    margin: 0px;
    border: 0px;
    border: 1px solid #CCEBFF;
}

QToolBar QLineEdit {
    margin: 0px;
    border: 1px solid #CCEBFF;
}
QToolBar[orientation="1"]::separator {
    width: 1px;
    background-color: #CCEBFF;
}
QToolBar[orientation="2"]::separator {
    height: 1px;
    background-color: #CCEBFF;
}

QComboBox QLineEdit {
    margin: 0px;
    padding: 0px 1px;
    border: none;
}

QComboBox {
    padding: 0px;
    background-color: %1;
    color: %2;
    font-size: 14px;
}

#VideoPlayerPauseShieldButton {
    background: #CCFFFFFF;
    border: none;
}

QLineEdit {
    padding: 0px;
    font-size: 14px;
}

QProgressBar {
    background-color: %1;
    border: 1px solid #CCEBFF;
    text-align: center;
    color: %2;
}
QProgressBar:indeterminate::chunk {
    background-color: #555555;
}

QComboBox QAbstractItemView {
    background-color: %1;
    selection-background-color: #CCEBFF;
    selection-color: #000000;
}
)";
      break;
    default:
      return "";
  }
  QString bgGeneral{ColorCfg::GetColorBackgroundGeneral(styleE)}; // %1
  QString foregroundGeneral{ColorCfg::GetColorForegroundGeneral(styleE)}; // %2
  styleSheet = styleSheet.arg(bgGeneral, foregroundGeneral);
  return styleSheet;
}
