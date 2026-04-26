#include "ViewItemDelegate.h"
#include "MemoryKey.h"
#include "Configuration.h"

ViewItemDelegate::ViewItemDelegate(QObject* parent) : QStyledItemDelegate {parent} {
  m_rowHeight = getConfig(MemoryKey::ROW_HEIGHT).toInt();
}

bool ViewItemDelegate::setRowHeight(int newRowHeight) {
  if (m_rowHeight == newRowHeight) {
    return false;
  }
  m_rowHeight = newRowHeight;
  return true;
}
