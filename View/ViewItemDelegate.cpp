#include "ViewItemDelegate.h"
#include "MemoryKey.h"
#include "Configuration.h"
#include "SizeTool.h"

ViewItemDelegate::ViewItemDelegate(QObject* parent) : QStyledItemDelegate {parent} {
  m_rowHeight = Configuration().value(MemoryKey::ROW_HEIGHT.name, MemoryKey::ROW_HEIGHT.toVariant()).toInt();
}

bool ViewItemDelegate::setRowHeight(int newRowHeight) {
  if (m_rowHeight == newRowHeight) {
    return false;
  }
  m_rowHeight = newRowHeight;
  return true;
}
