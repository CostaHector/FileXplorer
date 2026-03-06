#include "ToolBarWidget.h"
#include "PublicMacro.h"
#include <QLabel>

ToolBarWidget::ToolBarWidget(QBoxLayout::Direction direction, QWidget* parent) : QWidget{parent} {
  mLayout = new (std::nothrow) QBoxLayout{direction, this};
  CHECK_NULLPTR_RETURN_VOID(mLayout);
  mLayout->setContentsMargins(0, 0, 0, 0);
  mLayout->setSpacing(0);
  setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
}

void ToolBarWidget::setDirection(QBoxLayout::Direction direction) {
  if (mLayout->direction() == direction) {
    LOG_D("Skip, direction remains[%d] unchange no need set", direction);
    return;
  }
  mLayout->setDirection(direction);
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

  if (mLayout->direction() == QBoxLayout::LeftToRight || mLayout->direction() == QBoxLayout::RightToLeft) {  // 水平布局使用垂直分隔线
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedWidth(1);
    separator->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  } else {  // 垂直布局使用水平分隔线
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedHeight(1);
    separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  }

  mLayout->addWidget(separator);
  return separator;
}

int ToolBarWidget::setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle, bool bRecursive) {
  CHECK_NULLPTR_RETURN_INT(mLayout, -1);
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
  }
  return toolButtons.size();
}

QToolButton* ToolBarWidget::createToolButton(const QIcon& icon,
                                             const QString& text,
                                             const QString& toolTip,
                                             bool checkable,
                                             Qt::ToolButtonStyle toolButtonStyle,
                                             bool bToolButtonStyleFixed) {
  QToolButton* btn = new (std::nothrow) QToolButton{this};
  CHECK_NULLPTR_RETURN_NULLPTR(btn);
  QAction* tmpAct = new (std::nothrow) QAction{icon, text, this};
  tmpAct->setObjectName(text);
  CHECK_NULLPTR_RETURN_NULLPTR(tmpAct);
  tmpAct->setToolTip(QString{"<b>%1 (%2)</b><br/>"}.arg(text, "") + toolTip);
  tmpAct->setCheckable(checkable);

  btn->setObjectName(text);
  btn->setDefaultAction(tmpAct);
  btn->setToolButtonStyle(toolButtonStyle);
  btn->setAutoRaise(true);
  if (bToolButtonStyleFixed) {
    mToolButtonStyleFixedObjNameSet.insert(btn->objectName());
  }
  return btn;
}

QToolButton* ToolBarWidget::createToolButton(QAction* act, Qt::ToolButtonStyle toolButtonStyle, bool bToolButtonStyleFixed) {
  CHECK_NULLPTR_RETURN_NULLPTR(act);
  QToolButton* btn = new (std::nothrow) QToolButton{this};
  CHECK_NULLPTR_RETURN_NULLPTR(btn);
  btn->setObjectName(act->objectName().isEmpty() ? act->text() : act->objectName());
  btn->setDefaultAction(act);
  btn->setToolButtonStyle(toolButtonStyle);
  btn->setAutoRaise(true);
  if (bToolButtonStyleFixed) {
    mToolButtonStyleFixedObjNameSet.insert(btn->objectName());
  }
  return btn;
}

bool ToolBarWidget::addAction(QAction* act) {
  CHECK_NULLPTR_RETURN_FALSE(act);
  QToolButton* btn = createToolButton(act);
  CHECK_NULLPTR_RETURN_FALSE(btn);
  addWidget(btn);
  return true;
}
