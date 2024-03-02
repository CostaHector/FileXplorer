#include "StackedToolBar.h"

int StackedToolBar::AddToolBar(const QString& name, QToolBar* tb) {
  m_name2StackIndex[name] = m_stackedToolBar->addWidget(tb);
  return m_name2StackIndex[name];
}
