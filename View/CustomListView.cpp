#include "CustomListView.h"

#include "MemoryKey.h"
#include "StyleSheet.h"

#include <QActionGroup>
#include <QContextMenuEvent>

QSet<QString> CustomListView::LISTS_SET;

CustomListView::CustomListView(const QString& name, QWidget* parent) : QListView{parent}, m_name{name} {
  if (isNameExists(m_name)) {
    qWarning("Instance list name[%s] already exist, memory key will override", qPrintable(m_name));
  }
  LISTS_SET.insert(m_name);

  setAlternatingRowColors(true);

  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

  setDragDropMode(QAbstractItemView::NoDragDrop);
  setEditTriggers(QAbstractItemView::EditKeyPressed);

  const auto fontSize = Configuration().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(font());
  defaultFont.setPointSize(fontSize);
  setFont(defaultFont);

  _FLOW_ORIENTATION_ACT = new (std::nothrow) QAction{m_name + " Flow Orientation", this};
  _FLOW_ORIENTATION_ACT->setCheckable(true);
  _FLOW_ORIENTATION_ACT->setToolTip("Toggle ListView Flow Direction LeftToRight/TopToBottom");
  const bool isFlowLeftToRight = Configuration().value(m_name + "_ORIENTATION_LEFT_TO_RIGHT", true).toBool();
  _FLOW_ORIENTATION_ACT->setChecked(isFlowLeftToRight);
  onOrientationChange(isFlowLeftToRight);

  connect(_FLOW_ORIENTATION_ACT, &QAction::toggled, this, &CustomListView::onOrientationChange);
}

void CustomListView::contextMenuEvent(QContextMenuEvent* event) {
  if (m_menu != nullptr) {
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
  if (m_menu != nullptr) {
    qWarning("Don't rebind menu. m_menu is already not nullptr");
    return;
  }
  m_menu = menu;
  m_menu->addSeparator();
  m_menu->addAction(_FLOW_ORIENTATION_ACT);
}

void CustomListView::onOrientationChange(bool isLeftToRight) {
  setFlow(isLeftToRight ? QListView::Flow::LeftToRight : QListView::Flow::TopToBottom);
  Configuration().setValue(m_name + "_ORIENTATION_LEFT_TO_RIGHT", isLeftToRight);
  if (isLeftToRight) {
    _FLOW_ORIENTATION_ACT->setIcon(QIcon{":img/ALIGN_HORIZONTAL_LEFT"});
    _FLOW_ORIENTATION_ACT->setText(m_name + " Flow: Left to Right");
  } else {
    _FLOW_ORIENTATION_ACT->setIcon(QIcon{":img/ALIGN_VERTICAL_TOP"});
    _FLOW_ORIENTATION_ACT->setText(m_name + " Flow: Top to Bottom");
  }
}

void CustomListView::InitListView() {
}
