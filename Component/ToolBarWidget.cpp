#include "ToolBarWidget.h"
#include "PublicMacro.h"
#include <QLabel>

constexpr Qt::ToolButtonStyle ToolBarWidget::DEF_TOOL_BUTTON_STYLE;
constexpr bool ToolBarWidget::DEF_TOOL_BUTTON_STYLE_FIXED;

ToolBarWidget::ToolBarWidget(QBoxLayout::Direction direction, QWidget* parent)
  : QWidget{parent} {
  setProperty("isToolBar", true);
  mLayout = new (std::nothrow) QBoxLayout{direction, this};
  CHECK_NULLPTR_RETURN_VOID(mLayout);
  mLayout->setContentsMargins(0, 0, 0, 0);
  mLayout->setSpacing(0);
  setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
}

bool ToolBarWidget::setDirection(QBoxLayout::Direction direction) {
  if (mLayout->direction() == direction) {
    LOG_D("Skip, direction remains[%d] unchange no need set", direction);
    return false;
  }
  mLayout->setDirection(direction);
  return true;
}

void ToolBarWidget::addWidget(QWidget* parent, int stretch) {
  mLayout->addWidget(parent, stretch);
}

void ToolBarWidget::addSpacing(int size) {
  mLayout->addSpacing(size);
}

void ToolBarWidget::addStretch(int stretch) {
  mLayout->addStretch(stretch);
}

QLabel* ToolBarWidget::addString(const QString& sep) {
  QLabel* pLabel = new (std::nothrow) QLabel{sep, this};
  CHECK_NULLPTR_RETURN_NULLPTR(pLabel);
  mLayout->addWidget(pLabel);
  return pLabel;
}

QFrame* ToolBarWidget::addSeparator() {
  QFrame* separator = new (std::nothrow) QFrame{this};
  CHECK_NULLPTR_RETURN_NULLPTR(separator);

  if (mLayout->direction() == QBoxLayout::LeftToRight || mLayout->direction() == QBoxLayout::RightToLeft) { // 水平布局使用垂直分隔线
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedWidth(1);
    separator->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  } else { // 垂直布局使用水平分隔线
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedHeight(1);
    separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  }

  mLayout->addWidget(separator);
  return separator;
}

int ToolBarWidget::setToolButtonSizePolicy(const QSizePolicy& newSizePolicy, bool bRecursive) {
  CHECK_NULLPTR_RETURN_INT(mLayout, -1);
  mToolButtonSizePolicy = newSizePolicy;
  int affectedWid{0};
  QList<QToolButton*> toolButtons = this->findChildren<QToolButton*>();
  for (QToolButton* toolBtn : toolButtons) {
    if (!bRecursive && toolBtn->parentWidget() != this) {
      // 嵌套的QToolButton跳过, i.e., mLayout中的QWidget中的QToolButton控件
      continue;
    }
    if (toolBtn->sizePolicy() == newSizePolicy) {
      continue;
    }
    toolBtn->setSizePolicy(newSizePolicy);
    ++affectedWid;
  }
  return affectedWid;
}

int ToolBarWidget::setIconSize(const QSize& newIconSize, bool bRecursive) {
  CHECK_NULLPTR_RETURN_INT(mLayout, -1);
  mIconSize = newIconSize;
  int affectedWid{0};
  QList<QToolButton*> toolButtons = this->findChildren<QToolButton*>();
  for (QToolButton* toolBtn : toolButtons) {
    if (!bRecursive && toolBtn->parentWidget() != this) {
      // 嵌套的QToolButton跳过, i.e., mLayout中的QWidget中的QToolButton控件
      continue;
    }
    if (toolBtn->iconSize() == newIconSize) {
      continue;
    }
    toolBtn->setIconSize(newIconSize);
    ++affectedWid;
  }
  return affectedWid;
}

int ToolBarWidget::setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle, bool bRecursive) {
  CHECK_NULLPTR_RETURN_INT(mLayout, -1);
  mToolButtonStyle = toolButtonStyle;

  int affectedWid{0};
  QList<QToolButton*> toolButtons = this->findChildren<QToolButton*>();
  for (QToolButton* toolBtn : toolButtons) {
    if (!bRecursive && toolBtn->parentWidget() != this) {
      // 嵌套的QToolButton跳过, i.e., mLayout中的QWidget中的QToolButton控件
      continue;
    }
    if (toolBtn->toolButtonStyle() == toolButtonStyle) {
      continue;
    }
    if (mToolButtonStyleFixedObjNameSet.find(toolBtn->objectName()) != mToolButtonStyleFixedObjNameSet.cend()) {
      // 需要固定toolButtonStyle的组件跳过
      continue;
    }
    toolBtn->setToolButtonStyle(toolButtonStyle);
    ++affectedWid;
  }
  return affectedWid;
}

QToolButton* ToolBarWidget::createToolButton(QAction* act, bool bToolButtonStyleFixed) {
  CHECK_NULLPTR_RETURN_NULLPTR(act);
  QToolButton* btn = new (std::nothrow) QToolButton{this};
  CHECK_NULLPTR_RETURN_NULLPTR(btn);
  btn->setObjectName(act->objectName().isEmpty() ? act->text() : act->objectName());
  btn->setDefaultAction(act);
  if (mIconSize.isValid()) {
    btn->setIconSize(mIconSize);
  }
  btn->setToolButtonStyle(mToolButtonStyle);
  btn->setSizePolicy(mToolButtonSizePolicy);
  btn->setAutoRaise(true);
  if (bToolButtonStyleFixed) {
    mToolButtonStyleFixedObjNameSet.insert(btn->objectName());
  }
  return btn;
}

QAction* ToolBarWidget::addAction(const QIcon& icon, const QString& text) {
  QAction* act = new QAction{icon, text, this};
  addAction(act);
  return act;
}

bool ToolBarWidget::addAction(QAction* act) {
  CHECK_NULLPTR_RETURN_FALSE(act);
  mActions.push_back(act);
  QToolButton* btn = createToolButton(act, DEF_TOOL_BUTTON_STYLE_FIXED);
  CHECK_NULLPTR_RETURN_FALSE(btn);
  addWidget(btn);
  return true;
}
