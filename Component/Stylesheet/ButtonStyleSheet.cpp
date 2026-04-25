#include "ButtonStyleSheet.h"

bool ButtonStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new ButtonStyleSheet));

QString ButtonStyleSheet::GetStyleSheet(Style::StyleThemeE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleThemeE::THEME_LIGHT:
    case Style::StyleThemeE::THEME_DARK_MOON_FOG:
      styleSheet = R"(
  QToolButton {
    background-color: %1;
    border: 1px solid %1;
  }
  QToolButton:hover {
    background-color: %2;
  }
  QToolButton:pressed {
    background-color: %3;
  }
  QToolButton:checked {
    background-color: %4;
  }


  QPushButton {
    background-color: %5;
    border: 1px solid #CCEBFF;
    min-width: 80px;
  }
  QPushButton:hover {
    background-color: %6;
  }
  QPushButton:pressed {
    background-color: %7;
  }
  QPushButton:checked {
    background-color: %8;
  }
  QPushButton[applyButtonFlag] {
    border: 1px solid #1E90FF;
  }

  QRadioButton {
    background-color: %5;
  }

  QCheckBox {
    background-color: %5;
  }

  QDialogButtonBox {
  }
)";
      break;
    default:
      return "";
  }

  const auto& inst = GetInst();

  QString toolBtnGeneral = inst.GetColorValue("Background/AbstractButton/ToolButton/General", styleE);
  QString toolBtnHovered = inst.GetColorValue("Background/AbstractButton/ToolButton/Hovered", styleE);
  QString toolBtnPressed = inst.GetColorValue("Background/AbstractButton/ToolButton/Pressed", styleE);
  QString toolBtnChecked = inst.GetColorValue("Background/AbstractButton/ToolButton/Checked", styleE);

  QString pushBtnGeneral = inst.GetColorValue("Background/AbstractButton/PushButton/General", styleE);
  QString pushBtnHovered = inst.GetColorValue("Background/AbstractButton/PushButton/Hovered", styleE);
  QString pushBtnPressed = inst.GetColorValue("Background/AbstractButton/PushButton/Pressed", styleE);
  QString pushBtnChecked = inst.GetColorValue("Background/AbstractButton/PushButton/Checked", styleE);

  return styleSheet                                                        //
      .arg(toolBtnGeneral, toolBtnHovered, toolBtnPressed, toolBtnChecked) //
      .arg(pushBtnGeneral, pushBtnHovered, pushBtnPressed, pushBtnChecked);
}
