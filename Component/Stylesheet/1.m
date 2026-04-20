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
