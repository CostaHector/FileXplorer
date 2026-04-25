#include "ToolBarStyleSheet.h"

bool ToolBarStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new ToolBarStyleSheet));

QString ToolBarStyleSheet::GetStyleSheet(Style::StyleThemeE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleThemeE::THEME_LIGHT:
    case Style::StyleThemeE::THEME_DARK_MOON_FOG:
      styleSheet = R"(
QLabel {
    background-color: %1;
}
/* For Custom Status Bar Message Label Only. 0: normal, 1: abnormal */
QLabel#statusMessageLabel[alertLevel="0"] {
    font-weight: normal;
}
QLabel#statusMessageLabel[alertLevel="1"] {
    color: red;
    font-weight: bold;
}

QLineEdit {
    border: 1px solid #CCEBFF;
    margin: 1px;
    padding: 0px;
    background-color: %2;
}

QComboBox {
    margin: 0px;
    padding: 0px;
    background-color: %3;
    border: 1px solid %5;
}
QComboBox:editable {
    background-color: %2;
}

QToolBar {
    margin: 0px;
    border: none;
    background-color: %4;
}

QToolBar QLineEdit {
    margin: 0px;
    border: 1px solid #CCEBFF;
}
QToolBar::separator:horizontal {
    width: 1px;
    background-color: #CCEBFF;
}
QToolBar::separator:vertical {
    height: 1px;
    background-color: #CCEBFF;
}

QComboBox QAbstractItemView {
}
)";
      break;
    default:
      return "";
  }

  const auto& inst = GetInst();

  QString labelBg = inst.GetColorValue("Background/Label", styleE);
  QString lineEditBg = inst.GetColorValue("Background/LineEdit", styleE);
  QString comboBoxBg = inst.GetColorValue("Background/ComboBox", styleE);
  QString toolBarBg = inst.GetColorValue("Background/ToolBar", styleE);
  QString comboBoxBorderBg = inst.GetColorValue("Border/ComboBox", styleE);

  return styleSheet.arg(labelBg, lineEditBg, comboBoxBg, toolBarBg, comboBoxBorderBg);
}
