#include "StackedToolBar.h"

StackedToolBar::StackedToolBar(const QString& title, QWidget* parent)
    : QToolBar{title, parent}, m_stackedToolBar{new QStackedWidget(this)} {
  addWidget(m_stackedToolBar);
  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
}

int StackedToolBar::AddToolBar(ViewTypeTool::ViewType vt, QWidget* tb) {
  return m_name2StackIndex[vt] = m_stackedToolBar->addWidget(tb);
}
