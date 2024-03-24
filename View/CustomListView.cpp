#include "CustomListView.h"
#include "PublicVariable.h"
#include "ViewStyleSheet.h"

#include <QContextMenuEvent>

QSet<QString> CustomListView::LISTS_SET;

CustomListView::CustomListView(const QString& name, QWidget* parent) : QListView{parent}, m_name{name} {
  if (isNameExists(m_name)) {
    qWarning("Instance list name[%s] already exist", qPrintable(m_name));
    return;
  }
  LISTS_SET.insert(m_name);

  setAlternatingRowColors(true);

  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

  setDragDropMode(QAbstractItemView::NoDragDrop);
  setEditTriggers(QAbstractItemView::EditKeyPressed);

  const auto fontSize = PreferenceSettings().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(font());
  defaultFont.setPointSize(fontSize);
  setFont(defaultFont);

  setStyleSheet(ViewStyleSheet::TABLEVIEW_STYLESHEET);
}

void CustomListView::contextMenuEvent(QContextMenuEvent* event) {
  if (m_menu) {
    m_menu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
    return;
  }
  QListView::contextMenuEvent(event);
}

void CustomListView::BindMenu(QMenu* menu) {
  if (menu == nullptr) {
    qWarning("Don't bind a nullptr menu");
    return;
  }
  m_menu = menu;
}

void CustomListView::InitListView() {
  sizeHintForRow(ViewStyleSheet::ROW_SECTION_HEIGHT);
  isIndicatorHoldByRestoredStateTrustable = true;
}
