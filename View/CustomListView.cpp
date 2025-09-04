#include "CustomListView.h"

#include "MemoryKey.h"
#include "StyleSheet.h"

#include <QActionGroup>
#include <QContextMenuEvent>

QSet<QString> CustomListView::LISTS_SET;

CustomListView::CustomListView(const QString& name, QWidget* parent)//
  : QListView{parent}, m_name{name}, mflowIntAction{this} {
  if (isNameExists(m_name)) {
    qWarning("Instance list name[%s] already exist, memory key will override", qPrintable(m_name));
    return;
  }
  LISTS_SET.insert(m_name);


  int iconSizeIndexHint = Configuration().value(m_name + "_ICON_SIZE_INDEX", mCurIconSizeIndex).toInt();
  mCurIconSizeIndex = std::max(0, std::min(iconSizeIndexHint, IMAGE_SIZE::ICON_SIZE_CANDIDATES_N-1)); // [0, WHEEL_CANDIDATES_N)

  setAlternatingRowColors(true);

  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

  setDragDropMode(QAbstractItemView::NoDragDrop);
  setEditTriggers(QAbstractItemView::EditKeyPressed);

  const auto fontSize = Configuration().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(font());
  defaultFont.setPointSize(fontSize);
  setFont(defaultFont);

  _FLOW_ORIENTATION_LEFT_TO_RIGHT = new (std::nothrow) QAction{QIcon{":img/ALIGN_HORIZONTAL_LEFT"}, "Left to Right", this};
  _FLOW_ORIENTATION_LEFT_TO_RIGHT->setCheckable(true);
  _FLOW_ORIENTATION_LEFT_TO_RIGHT->setToolTip(QString{"Set %1 Flow Orientation Left to Right"}.arg(m_name));

  _FLOW_ORIENTATION_TOP_TO_BOTTOM = new (std::nothrow) QAction{QIcon{":img/ALIGN_VERTICAL_TOP"}, "Top to bottom", this};;
  _FLOW_ORIENTATION_TOP_TO_BOTTOM->setCheckable(true);
  _FLOW_ORIENTATION_TOP_TO_BOTTOM->setToolTip(QString{"Set %1 Flow Orientation Top to Bottom"}.arg(m_name));

  mflowIntAction.init({{_FLOW_ORIENTATION_LEFT_TO_RIGHT, LeftToRight}, {_FLOW_ORIENTATION_TOP_TO_BOTTOM, TopToBottom}}, LeftToRight, QActionGroup::ExclusionPolicy::Exclusive);
  int flowInt = Configuration().value(m_name + "_FLOW_ORIENTATION", (int)mflowIntAction.defVal()).toInt();
  mflowIntAction.setChecked(flowInt);

  _FLOW_ORIENTATION = new (std::nothrow) QMenu{QString{"%1 Flow Orientation"}.arg(m_name), this};
  _FLOW_ORIENTATION->addActions(mflowIntAction.mActGrp->actions());


  InitListView();

  connect(mflowIntAction.mActGrp, &QActionGroup::triggered, this, &CustomListView::onOrientationChange);
}

CustomListView::~CustomListView() {
  const auto sz = iconSize();
  Configuration().setValue(m_name + "_ORIENTATION_LEFT_TO_RIGHT", (int)flow());
  Configuration().setValue(m_name + "_ICON_SIZE_INDEX", mCurIconSizeIndex);
}

void CustomListView::contextMenuEvent(QContextMenuEvent* event) {
  if (m_menu != nullptr) {
    m_menu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
    event->accept();
    return;
  }
  QListView::contextMenuEvent(event);
}

void CustomListView::wheelEvent(QWheelEvent *event) {
  if (event->modifiers() == Qt::ControlModifier) {
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
      int numSteps = numDegrees.y() / 15;
      int newSizeIndex = mCurIconSizeIndex + (numSteps > 0 ? 1 : -1);
      if (newSizeIndex < 0) {
        return;
      } else if (newSizeIndex >= IMAGE_SIZE::ICON_SIZE_CANDIDATES_N) {
        return;
      }
      mCurIconSizeIndex = newSizeIndex;
      setIconSize(IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconSizeIndex]);
      // emit onIconSizeChanged(IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconSizeIndex]);
      event->accept();
      return;
    }
  }
  QListView::wheelEvent(event);
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
  m_menu->addMenu(_FLOW_ORIENTATION);
}

void CustomListView::onOrientationChange(const QAction* pAct) {
  const QListView::Flow newEnum = mflowIntAction.act2Enum(pAct);
  setFlow(newEnum);
}

void CustomListView::InitListView() {
  setIconSize(IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconSizeIndex]);
}
