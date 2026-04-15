#include "ToolButtonStyleSheet.h"

bool ToolButtonStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new ToolButtonStyleSheet));

QString ToolButtonStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
      styleSheet = R"(
/* 基础样式 */
QToolBar QToolButton {
    border: 1px solid transparent; /* 预先占用边框空间 */
    font-size: 14px;
}
/* 悬停状态 */
QToolBar QToolButton:hover {
    border: 1px solid #5e5e60;
}

/* 按下状态 */
QToolBar QToolButton:pressed {
    background: #CCEBFF;
}

/* checked状态 */
QToolBar QToolButton:checked {
    background: #CCEBFF;
    border: 1px solid #ECEBFF;
}

/* checked时的悬停状态 */
QToolBar QToolButton:checked:hover {
    background: #CCEBFF;
    border: 1px solid #5597F3;
}

/* !checked时的悬停状态 */
QToolBar QToolButton:!checked:hover {
    background: #CCEBFF;
    border: 1px solid #5597F3;
}

/* 禁用状态 */
QToolBar QToolButton:disabled {
    color: #7f7f7f;
}

/* 基础样式 */
QWidget QToolButton {
    border: 1px solid transparent; /* 预先占用边框空间 */
    font-size: 14px;
}
/* 悬停状态 */
QWidget QToolButton:hover {
    border: 1px solid #5e5e60;
}

/* 按下状态 */
QWidget QToolButton:pressed {
    background: #CCEBFF;
}

/* checked状态 */
QWidget QToolButton:checked {
    background: #CCEBFF;
    border: 1px solid #ECEBFF;
}

/* checked时的悬停状态 */
QWidget QToolButton:checked:hover {
    background: #CCEBFF;
    border: 1px solid #5597F3;
}

/* !checked时的悬停状态 */
QWidget QToolButton:!checked:hover {
    background: #CCEBFF;
    border: 1px solid #5597F3;
}

/* 禁用状态 */
QWidget QToolButton:disabled {
    color: #7f7f7f;
}

QToolButton {
  %1
}

QToolButton[popupMode="1"] {
    padding-right:16px;
}

/* 主按钮选中状态 */
QToolButton:pressed,
QToolButton:checked {
    background-color: #CCEBFF;
}
QToolButton::menu-button {

}

QToolButton::menu-button:pressed {
    background-color: #CCEBFF;
}
QToolButton::menu-button:hover {
    background-color: #CCEBFF;
    border: 1px solid #5597F3;
}
QToolButton::menu-indicator {
    image:none;
}
QToolButton::menu-arrow {

}
)";
      break;
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
/* 基础样式 */
QToolBar QToolButton {
    background: transparent;
    color: #FFFFFF;
    border: 1px solid transparent; /* 预先占用边框空间 */
    font-size: 14px;
}
/* 悬停状态 */
QToolBar QToolButton:hover {
    background: #CCEBFF;
    color: #000000;
    border: 1px solid #5e5e60;
}

/* 按下状态 */
QToolBar QToolButton:pressed {
    background: #6F6F6F;
    color: #FFFFFF;
}

/* checked状态 */
QToolBar QToolButton:checked {
    background: #5F5F5F;
    color: #FFFFFF;
    border: 1px solid #7F7F7F;
}

/* checked时的悬停状态 */
QToolBar QToolButton:checked:hover {
    background: #6F6F6F;
    color: #FFFFFF;
    border: 1px solid #7F7F7F;
}

/* !checked时的悬停状态 */
QToolBar QToolButton:!checked:hover {
    background: #5F5F5F;
    color: #FFFFFF;
    border: 1px solid #7F7F7F;
}

/* 禁用状态 */
QToolBar QToolButton:disabled {
    color: #7f7f7f;
}

/* 基础样式 */
QWidget QToolButton {
    background: transparent;
    color: #FFFFFF;
    border: 1px solid transparent; /* 预先占用边框空间 */
    font-size: 14px;
}
/* 悬停状态 */
QWidget QToolButton:hover {
    background: #CCEBFF;
    color: #000000;
    border: 1px solid #5e5e60;
}

/* 按下状态 */
QWidget QToolButton:pressed {
    background: #6F6F6F;
    color: #FFFFFF;
}

/* checked状态 */
QWidget QToolButton:checked {
    background: #5F5F5F;
    color: #FFFFFF;
    border: 1px solid #7F7F7F;
}

/* checked时的悬停状态 */
QWidget QToolButton:checked:hover {
    background: #6F6F6F;
    color: #FFFFFF;
    border: 1px solid #7F7F7F;
}

/* !checked时的悬停状态 */
QWidget QToolButton:!checked:hover {
    background: #5F5F5F;
    color: #FFFFFF;
    border: 1px solid #7F7F7F;
}

/* 禁用状态 */
QWidget QToolButton:disabled {
    color: #7f7f7f;
}

QToolButton {
  %1
}

QToolButton[popupMode="1"] {
    padding-right:16px;
}

/* 主按钮选中状态 */
QToolButton:pressed,
QToolButton:checked {
    background-color: #7F7F7F;
    color: #FFFFFF;
}
QToolButton::menu-button {
    border-left:1px solid #333333;
}

QToolButton::menu-button:pressed {
    background-color: #6F6F6F;
}
QToolButton::menu-button:hover {
    background: #5F5F5F;
    border: 1px solid #7F7F7F;
}
QToolButton::menu-indicator {
    image:none;
}
QToolButton::menu-arrow {

}
)";
      break;
    default:
      return "";
  }
  QString fontStr = FontCfg::ReadFontString();
  styleSheet = styleSheet.arg(fontStr);
  return styleSheet;
}


