#include "WidgetStyleSheet.h"

bool WidgetStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new WidgetStyleSheet));

QString WidgetStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
      styleSheet = R"(
QWidget:disabled {
}

QMainWindow::separator {
    width: 1px;
    height: 0px;
}

QDockWidget::separator {
    width: 0px;
    height: 0px;
}

QPushButton {
    border: 1px solid #3c3c3c;
    min-width: 80px;
}

QLineEdit {
    border: 1px solid;
    margin: 1px;
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
    border: 1px solid #FFFFFF;
}

QToolBar QToolBar {
    margin: 0px;
    border: 0px;
}
QToolBar QComboBox {
    margin: 0px;
    border: 0px;
    border: 1px solid #000000;
}

QToolBar QLineEdit {
    margin: 0px;
    border: 1px solid #000000;
}

QComboBox QLineEdit {
    margin: 0px;
    padding: 0px 1px;
    border: none;          /* 移除QLineEdit自身边框 */
}

QComboBox {
    padding: 0px;
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
    border: 1px solid;
    text-align: center;
}
QProgressBar:indeterminate::chunk {
}

QComboBox QAbstractItemView {
    selection-background-color: #CCEBFF;
    selection-color: #000000;
}
)";
      break;
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QWidget {
    background-color: #000000;
    color: #FFFFFF;
    selection-background-color: #264f78;
}

QWidget:disabled {
    color: #7a7a7a;
}

QWidget[isToolBar="true"] {
    background-color: #3F3B39;
    color: #FFFFFF;
}

QMainWindow {
    background-color: #3F3B39;
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
    background-color: #252525;
    color: #FFFFFF;
    selection-background-color: #264f78;
}

QPushButton {
    background-color: #333333;
    border: 1px solid #3c3c3c;
    min-width: 80px;
}

QPushButton:hover {
    background-color: #3c3c3c;
}

QLineEdit {
    background-color: #3F3B39;
    border: 1px solid rgb(160, 160, 160);
    margin: 1px;
    color: #FFFFFF;
}

QStatusBar, QRadioButton, QCheckBox {
    background-color: #3F3B39;
    color: #FFFFFF;
}

QLabel {
    background-color: #3F3B39;
    color: #FFFFFF;
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
    background: #3F3B39;
    border: 1px solid #000000;
}

QToolBar QToolBar {
    margin: 0px;
    border: 0px;
}
QToolBar QComboBox {
    margin: 0px;
    border: 0px;
    border: 1px solid #FFFFFF;
}

QToolBar QLineEdit {
    margin: 0px;
    border: 1px solid #FFFFFF;
}

QComboBox QLineEdit {
    margin: 0px;
    padding: 0px 1px;
    border: none;          /* 移除QLineEdit自身边框 */
}

QComboBox {
    padding: 0px;
    background-color: #3F3B39;
    color: #FFFFFF;
    font-size: 14px;
}

#VideoPlayerPauseShieldButton {
    background: #FFF0F0F0;
    border: none;
}

QLineEdit {
    padding: 0px;
    font-size: 14px;
}

QProgressBar {
    background-color: #323232;
    border: 1px solid #444444;
    text-align: center;
    color: #E0E0E0;
}
QProgressBar:indeterminate::chunk {
    background-color: #555555;
}

QComboBox QAbstractItemView {
    background-color: #333333;
    selection-background-color: #5F5F5F;
    selection-color: #FFFFFF;
}
)";
      break;
    default:
      return "";
  }

  return styleSheet;
}
