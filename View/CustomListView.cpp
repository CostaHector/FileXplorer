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

  const auto fontSize = PreferenceSettings().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(font());
  defaultFont.setPointSize(fontSize);
  setFont(defaultFont);

  _ORIENTATION_LEFT_TO_RIGHT = new (std::nothrow) QAction{"left to right", this};
  _ORIENTATION_TOP_TO_BOTTOM = new (std::nothrow) QAction{"top to bottom", this};
  _ORIENTATION_GRP = new (std::nothrow) QActionGroup{this};
  _ORIENTATION_GRP->addAction(_ORIENTATION_LEFT_TO_RIGHT);
  _ORIENTATION_GRP->addAction(_ORIENTATION_TOP_TO_BOTTOM);
  _ORIENTATION_LEFT_TO_RIGHT->setCheckable(true);
  _ORIENTATION_TOP_TO_BOTTOM->setCheckable(true);
  _ORIENTATION_GRP->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  if (PreferenceSettings().value(m_name + "_ORIENTATION_LEFT_TO_RIGHT", true).toBool()) {
    _ORIENTATION_LEFT_TO_RIGHT->setChecked(true);
    onOrientationChange(_ORIENTATION_LEFT_TO_RIGHT);
  } else {
    _ORIENTATION_TOP_TO_BOTTOM->setChecked(true);
    onOrientationChange(_ORIENTATION_TOP_TO_BOTTOM);
  }
  connect(_ORIENTATION_GRP, &QActionGroup::triggered, this, &CustomListView::onOrientationChange);
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

void CustomListView::onOrientationChange(const QAction* pOrientation) {
  if (pOrientation == nullptr) {
    qCritical("pOrientation is nullptr");
    return;
  }
  if (pOrientation->text() == "left to right") {
    setFlow(QListView::Flow::LeftToRight);
    PreferenceSettings().setValue(m_name + "_ORIENTATION_LEFT_TO_RIGHT", true);
  } else {
    setFlow(QListView::Flow::TopToBottom);
    PreferenceSettings().setValue(m_name + "_ORIENTATION_LEFT_TO_RIGHT", false);
  }
}

void CustomListView::InitListView() {
  sizeHintForRow(StyleSheet::ROW_SECTION_HEIGHT);
}
