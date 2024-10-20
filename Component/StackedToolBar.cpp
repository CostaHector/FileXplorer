#include "StackedToolBar.h"

int StackedToolBar::AddToolBar(ViewTypeTool::ViewType vt, QToolBar* tb) {
  return m_name2StackIndex[vt] = m_stackedToolBar->addWidget(tb);
}
