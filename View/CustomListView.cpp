#include "CustomListView.h"

#include "MemoryKey.h"
#include "ImageTool.h"
#include "StyleSheet.h"
#include "NotificatorMacro.h"
#include "ViewHelper.h"
#include "PublicMacro.h"

#include <QActionGroup>
#include <QContextMenuEvent>

QSet<QString> CustomListView::LISTS_SET;

CustomListView::CustomListView(const QString& name, QWidget* parent)  //
    : QListView{parent}, m_name{name} {
  if (isNameExists(m_name)) {
#ifdef RUNNING_UNIT_TESTS
    LOG_D("Instance list name[%s] already exist, memory key will override", qPrintable(m_name));
#else
    LOG_W("Instance list name[%s] already exist, memory key will override", qPrintable(m_name));
#endif
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

  {
    const QString iconSizeMenuName{m_name + " " + tr("Icon size")};
    _ICON_SIZE_MENU = new (std::nothrow) IconSizeMenu{iconSizeMenuName, m_name, this};

    const QString textElideModeMenuName{m_name + " " + tr("Text elide mode")};
    _TEXT_ELIDE_MODE_MENU = new (std::nothrow) TextElideModeMenu{textElideModeMenuName, m_name, this};
  }

  {
    _FLOW_ORIENTATION_TTB = new (std::nothrow) QAction{QIcon{":img/ALIGN_VERTICAL_TOP"}, tr("Flow Orientation: TopToBottom"), this};
    CHECK_FALSE_RETURN_VOID(_FLOW_ORIENTATION_TTB);
    _FLOW_ORIENTATION_TTB->setCheckable(true);
    _FLOW_ORIENTATION_TTB->setChecked(Configuration().value(m_name + "_FLOW_ORIENTATION", true).toBool());
    _FLOW_ORIENTATION_TTB->setToolTip("The items layout should flow TopToBottom direction(default) if enabled, other LeftToRight");

    _VIEW_MODE_LIST_ICON = new (std::nothrow) QAction{QIcon{":img/VIEW_MODE_ICON"}, tr("View Mode: Icon"), this};
    CHECK_NULLPTR_RETURN_VOID(_VIEW_MODE_LIST_ICON);
    _VIEW_MODE_LIST_ICON->setCheckable(true);
    _VIEW_MODE_LIST_ICON->setChecked(Configuration().value(m_name + "_VIEW_MODE_LIST_ICON", false).toBool());
    _VIEW_MODE_LIST_ICON->setToolTip("Icon if enabled, other List");

    _RESIZED_MODE_ADJUST = new (std::nothrow) QAction{QIcon{":img/RESIZE_MODE_FIXED"}, tr("Resize Mode: Adjust"), this};
    CHECK_NULLPTR_RETURN_VOID(_RESIZED_MODE_ADJUST);
    _RESIZED_MODE_ADJUST->setCheckable(true);
    _RESIZED_MODE_ADJUST->setChecked(Configuration().value(m_name + "_RESIZED_MODE_FIXED_OR_ADJUST", true).toBool());
    _RESIZED_MODE_ADJUST->setToolTip("The items will be laid out again when the view is resized if enabled, otherwise fixed. by default: adjust");

    _WRAPPING_ACTIONS = new (std::nothrow) QAction{QIcon{":img/LIST_WRAPPING"}, tr("Wrapping"), this};
    CHECK_NULLPTR_RETURN_VOID(_WRAPPING_ACTIONS);
    _WRAPPING_ACTIONS->setCheckable(true);
    _WRAPPING_ACTIONS->setChecked(Configuration().value(m_name + "_WRAPPING_ACTIONS", false).toBool());
    _WRAPPING_ACTIONS->setToolTip("The layout should wrap when there is no more space in the visible area if enabled, by default: no wrap false");

    _UNIFORM_ITEM_SIZES = new (std::nothrow) QAction{QIcon{":img/UNIFORM_ITEM_SIZES"}, tr("Uniform items sizes"), this};
    CHECK_NULLPTR_RETURN_VOID(_UNIFORM_ITEM_SIZES);
    _UNIFORM_ITEM_SIZES->setCheckable(true);
    _UNIFORM_ITEM_SIZES->setChecked(Configuration().value(m_name + "_UNIFORM_ITEM_SIZES", false).toBool());
    _UNIFORM_ITEM_SIZES->setToolTip("all items in the listview have the same size, by default: false");
  }

  m_menu = new (std::nothrow) AddableMenu{name + "_menu", this};
  CHECK_NULLPTR_RETURN_VOID(m_menu);
  m_menu->setToolTipsVisible(true);
  AddItselfAction2Menu();

  InitListView();
  SubscribePublicActions();
}

void CustomListView::SubscribePublicActions() {
  connect(_ICON_SIZE_MENU, &IconSizeMenu::iconScaledIndexChanged, this, &CustomListView::onIconScaledIndexChanged);
  connect(_TEXT_ELIDE_MODE_MENU, &TextElideModeMenu::reqTextElideModeChanged, this, &QListView::setTextElideMode);

  connect(_FLOW_ORIENTATION_TTB, &QAction::toggled, this, &CustomListView::onOrientationChanged);
  connect(_VIEW_MODE_LIST_ICON, &QAction::toggled, this, &CustomListView::onViewModeListIconToggled);
  connect(_RESIZED_MODE_ADJUST, &QAction::toggled, this, &CustomListView::onResizeModeToggled);
  connect(_WRAPPING_ACTIONS, &QAction::toggled, this, &CustomListView::onWrapingToggled);
  connect(_UNIFORM_ITEM_SIZES, &QAction::toggled, this, &CustomListView::onUniformItemSizedToggled);

  setTextElideMode(_TEXT_ELIDE_MODE_MENU->GetTextElideMode());
  onOrientationChanged(_FLOW_ORIENTATION_TTB->isChecked());
  onViewModeListIconToggled(_VIEW_MODE_LIST_ICON->isChecked());
  onResizeModeToggled(_RESIZED_MODE_ADJUST->isChecked());
  onWrapingToggled(_WRAPPING_ACTIONS->isChecked());
  onUniformItemSizedToggled(_UNIFORM_ITEM_SIZES->isChecked());
}

CustomListView::~CustomListView() {
  Configuration().setValue(m_name + "_FLOW_ORIENTATION", (flow() == QListView::Flow::TopToBottom));
  Configuration().setValue(m_name + "_VIEW_MODE_LIST_ICON", (viewMode() == QListView::ViewMode::IconMode));
  Configuration().setValue(m_name + "_RESIZED_MODE_FIXED_OR_ADJUST", (resizeMode() == QListView::ResizeMode::Adjust));
  Configuration().setValue(m_name + "_WRAPPING_ACTIONS", isWrapping());
  Configuration().setValue(m_name + "_UNIFORM_ITEM_SIZES", uniformItemSizes());
}

void CustomListView::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (m_menu != nullptr) {
#ifndef RUNNING_UNIT_TESTS
    m_menu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
#endif
    event->accept();
    return;
  }
  QListView::contextMenuEvent(event);
}

void CustomListView::wheelEvent(QWheelEvent* event) {
  if (event->modifiers() == Qt::ControlModifier) {
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
      int numSteps = numDegrees.y() / 15;
      const int oldScaledIndex = _ICON_SIZE_MENU->GetScaledIndex();
      if (!_ICON_SIZE_MENU->UpdateScaledIndexInWheelEvent(oldScaledIndex + (numSteps > 0 ? 1 : -1))) {
        return;
      }
      onIconScaledIndexChanged(_ICON_SIZE_MENU->GetScaledIndex());
      event->accept();
      return;
    }
  }
  QListView::wheelEvent(event);
}

void CustomListView::onIconScaledIndexChanged(int newScaledIndex) {
  const QSize newIconSize = IMAGE_SIZE::ICON_SIZE_CANDIDATES[newScaledIndex];
  setIconSize(newIconSize);
  emit iconSizeChanged(IMAGE_SIZE::ICON_SIZE_CANDIDATES[newScaledIndex]);
  LOG_OK_NP("[Change] Icon size", IMAGE_SIZE::HumanReadFriendlySize(newScaledIndex));
}

void CustomListView::PushFrontExclusiveActions(const QList<QAction*>& acts) {
  CHECK_NULLPTR_RETURN_VOID(m_menu);
  m_menu->push_front(acts);
}

void CustomListView::PushBackExclusiveActions(const QList<QAction*>& acts) {
  CHECK_NULLPTR_RETURN_VOID(m_menu);
  m_menu->push_back(acts);
}

void CustomListView::AddItselfAction2Menu() {
  m_menu->addSeparator();
  m_menu->addMenu(_ICON_SIZE_MENU);
  m_menu->addMenu(_TEXT_ELIDE_MODE_MENU);
  m_menu->addAction(_FLOW_ORIENTATION_TTB);
  m_menu->addAction(_VIEW_MODE_LIST_ICON);
  m_menu->addAction(_RESIZED_MODE_ADJUST);
  m_menu->addAction(_WRAPPING_ACTIONS);
  m_menu->addAction(_UNIFORM_ITEM_SIZES);
}

void CustomListView::onOrientationChanged(const bool bchecked) {
  setFlow(bchecked ? QListView::Flow::TopToBottom : QListView::Flow::LeftToRight);
}
void CustomListView::onViewModeListIconToggled(const bool bchecked) {
  setViewMode(bchecked ? QListView::ViewMode::IconMode : QListView::ViewMode::ListMode);
}
void CustomListView::onResizeModeToggled(const bool bchecked) {
  setResizeMode(bchecked ? QListView::ResizeMode::Adjust : QListView::ResizeMode::Fixed);
}
void CustomListView::onWrapingToggled(const bool bchecked) {
  setWrapping(bchecked);
}
void CustomListView::onUniformItemSizedToggled(const bool bchecked) {
  setUniformItemSizes(bchecked);
}

void CustomListView::InitListView() {
  const int sizeScaledIndex = _ICON_SIZE_MENU->GetScaledIndex();
  setIconSize(IMAGE_SIZE::ICON_SIZE_CANDIDATES[sizeScaledIndex]);
  // setGridSize(IMAGE_SIZE::ICON_SIZE_CANDIDATES[newScaledIndex]);
}

void CustomListView::mousePressEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event)
  if (View::onMouseSidekeyBackwardForward(event->modifiers(), event->button())) {
    event->accept();
    return;
  }
  QListView::mousePressEvent(event);
}
