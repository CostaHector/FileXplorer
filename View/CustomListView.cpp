#include "CustomListView.h"

#include "MemoryKey.h"
#include "StyleSheet.h"
#include "NotificatorMacro.h"
#include "ViewHelper.h"

#include <QActionGroup>
#include <QContextMenuEvent>

QSet<QString> CustomListView::LISTS_SET;

CustomListView::CustomListView(const QString& name, QWidget* parent) //
  : QListView{parent}
  , m_name{name} {
  if (isNameExists(m_name)) {
#ifdef RUNNING_UNIT_TESTS
    LOG_D("Instance list name[%s] already exist, memory key will override", qPrintable(m_name));
#else
    LOG_W("Instance list name[%s] already exist, memory key will override", qPrintable(m_name));
#endif
  }
  LISTS_SET.insert(m_name);

  int iconSizeIndexHint = Configuration().value(m_name + "_ICON_SIZE_INDEX", mCurIconSizeIndex).toInt();
  mCurIconSizeIndex = std::max(0, std::min(iconSizeIndexHint, IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1)); // [0, WHEEL_CANDIDATES_N)

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
    _TEXT_ELIDE_MODE_LEFT = new (std::nothrow) QAction{QIcon{""}, "Elide Left", this};
    CHECK_FALSE_RETURN_VOID(_TEXT_ELIDE_MODE_LEFT);
    _TEXT_ELIDE_MODE_LEFT->setCheckable(true);

    _TEXT_ELIDE_MODE_RIGHT = new (std::nothrow) QAction{QIcon{""}, "Elide Right", this};
    CHECK_FALSE_RETURN_VOID(_TEXT_ELIDE_MODE_RIGHT);
    _TEXT_ELIDE_MODE_RIGHT->setCheckable(true);

    _TEXT_ELIDE_MODE_MIDDLE = new (std::nothrow) QAction{QIcon{""}, "Elide Middle", this};
    CHECK_FALSE_RETURN_VOID(_TEXT_ELIDE_MODE_MIDDLE);
    _TEXT_ELIDE_MODE_MIDDLE->setCheckable(true);

    _TEXT_ELIDE_MODE_NONE = new (std::nothrow) QAction{QIcon{""}, "Elide None", this};
    CHECK_FALSE_RETURN_VOID(_TEXT_ELIDE_MODE_NONE);
    _TEXT_ELIDE_MODE_NONE->setCheckable(true);

    mTextEditModeIntAction.init(
        {
            {_TEXT_ELIDE_MODE_LEFT, Qt::TextElideMode::ElideLeft},     //
            {_TEXT_ELIDE_MODE_RIGHT, Qt::TextElideMode::ElideRight},   //
            {_TEXT_ELIDE_MODE_MIDDLE, Qt::TextElideMode::ElideMiddle}, //
            {_TEXT_ELIDE_MODE_NONE, Qt::TextElideMode::ElideNone},
        }, //
        Qt::TextElideMode::ElideLeft,
        QActionGroup::ExclusionPolicy::Exclusive);
    int elideInt = Configuration().value(m_name + "_TEXT_ELIDE_MODE", (int) mTextEditModeIntAction.defVal()).toInt();
    QAction* checkedElideAct = mTextEditModeIntAction.setCheckedIfActionExist(elideInt);
    onTextElideModeChanged(checkedElideAct);

    _TEXT_ELIDE_MODE_MENU = new (std::nothrow) QMenu{QString{"%1 Text Elide Mode"}.arg(m_name), this};
    CHECK_NULLPTR_RETURN_VOID(_TEXT_ELIDE_MODE_MENU);
    _TEXT_ELIDE_MODE_MENU->addActions(mTextEditModeIntAction.getActionEnumAscendingList());
  }

  {
    _FLOW_ORIENTATION = new (std::nothrow) QAction{QIcon{":img/ALIGN_VERTICAL_TOP"}, "Flow Orientation(LR/TB)", this};
    CHECK_FALSE_RETURN_VOID(_FLOW_ORIENTATION);
    _FLOW_ORIENTATION->setToolTip("Set flow orientation. Unchecked for LeftToRight and Checked for TopToBottom");
    _FLOW_ORIENTATION->setCheckable(true);
    _FLOW_ORIENTATION->setChecked(Configuration().value(m_name + "_FLOW_ORIENTATION", false).toBool());

    _VIEW_MODE_LIST_ICON = new (std::nothrow) QAction{QIcon{""}, "View Mode(List/Icon)", this};
    CHECK_NULLPTR_RETURN_VOID(_VIEW_MODE_LIST_ICON);
    _VIEW_MODE_LIST_ICON->setToolTip("Set View Mode List or Icon. Unchecked for List and Checked for Icon");
    _VIEW_MODE_LIST_ICON->setCheckable(true);
    _VIEW_MODE_LIST_ICON->setChecked(Configuration().value(m_name + "_VIEW_MODE_LIST_ICON", false).toBool());

    _RESIZED_MODE_FIXED_OR_ADJUST = new (std::nothrow) QAction{QIcon{""}, "Resize Mode(Fixed/Adjust)", this};
    CHECK_NULLPTR_RETURN_VOID(_RESIZED_MODE_FIXED_OR_ADJUST);
    _RESIZED_MODE_FIXED_OR_ADJUST->setToolTip("Set Resize Mode Fixed or Adjust. Unchecked for Fixed and Checked for Adjust");
    _RESIZED_MODE_FIXED_OR_ADJUST->setCheckable(true);
    _RESIZED_MODE_FIXED_OR_ADJUST->setChecked(Configuration().value(m_name + "_RESIZED_MODE_FIXED_OR_ADJUST", false).toBool());

    _WRAPING_ACTIONS = new (std::nothrow) QAction{QIcon{""}, "Wraping", this};
    CHECK_NULLPTR_RETURN_VOID(_WRAPING_ACTIONS);
    _WRAPING_ACTIONS->setCheckable(true);
    _WRAPING_ACTIONS->setChecked(Configuration().value(m_name + "_WRAPING_ACTIONS", false).toBool());

    _UNIFORM_ITEM_SIZES = new (std::nothrow) QAction{QIcon{""}, "Uniform items sizes", this};
    CHECK_NULLPTR_RETURN_VOID(_UNIFORM_ITEM_SIZES);
    _UNIFORM_ITEM_SIZES->setCheckable(true);
    _UNIFORM_ITEM_SIZES->setChecked(Configuration().value(m_name + "_UNIFORM_ITEM_SIZES", false).toBool());
  }

  m_menu = new (std::nothrow) AddableMenu{name + "_menu", this};
  CHECK_NULLPTR_RETURN_VOID(m_menu);

  InitListView();
  SubscribePublicActions();
}

void CustomListView::SubscribePublicActions() {
  connect(mTextEditModeIntAction.getActionGroup(), &QActionGroup::triggered, this, &CustomListView::onTextElideModeChanged);

  connect(_FLOW_ORIENTATION, &QAction::toggled, this, &CustomListView::onOrientationChanged);
  connect(_VIEW_MODE_LIST_ICON, &QAction::toggled, this, &CustomListView::onViewModeListIconToggled);
  connect(_RESIZED_MODE_FIXED_OR_ADJUST, &QAction::toggled, this, &CustomListView::onResizeModeToggled);
  connect(_WRAPING_ACTIONS, &QAction::toggled, this, &CustomListView::onWrapingToggled);
  connect(_UNIFORM_ITEM_SIZES, &QAction::toggled, this, &CustomListView::onUniformItemSizedToggled);

  onOrientationChanged(_FLOW_ORIENTATION->isChecked());
  onViewModeListIconToggled(_VIEW_MODE_LIST_ICON->isChecked());
  onResizeModeToggled(_RESIZED_MODE_FIXED_OR_ADJUST->isChecked());
  onWrapingToggled(_WRAPING_ACTIONS->isChecked());
  onUniformItemSizedToggled(_UNIFORM_ITEM_SIZES->isChecked());
}

CustomListView::~CustomListView() {
  Configuration().setValue(m_name + "_TEXT_ELIDE_MODE", (int) textElideMode());

  Configuration().setValue(m_name + "_FLOW_ORIENTATION", (flow() == QListView::Flow::TopToBottom));
  Configuration().setValue(m_name + "_VIEW_MODE_LIST_ICON", (viewMode() == QListView::ViewMode::IconMode));
  Configuration().setValue(m_name + "_RESIZED_MODE_FIXED_OR_ADJUST", (resizeMode() == QListView::ResizeMode::Adjust));
  Configuration().setValue(m_name + "_WRAPING_ACTIONS", isWrapping());
  Configuration().setValue(m_name + "_UNIFORM_ITEM_SIZES", uniformItemSizes());

  Configuration().setValue(m_name + "_ICON_SIZE_INDEX", mCurIconSizeIndex);
}

void CustomListView::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (m_menu != nullptr) {
#ifndef RUNNING_UNIT_TESTS
    m_menu->popup(viewport()->mapToGlobal(event->pos())); // or QCursor::pos()
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
      int newSizeIndex = mCurIconSizeIndex + (numSteps > 0 ? 1 : -1);
      if (!setIconSizeScaledIndex(newSizeIndex)) {
        return;
      }
      const QSize newIconSize = IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconSizeIndex];
      setIconSize(newIconSize);
      LOG_OK_P("[Change] Icon size", "[%d] %d x %d", mCurIconSizeIndex, newIconSize.width(), newIconSize.height());
      emit iconSizeChanged(IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconSizeIndex]);
      event->accept();
      return;
    }
  }
  QListView::wheelEvent(event);
}

void CustomListView::BindMenu(QMenu* menu) {
  if (menu == nullptr) {
    LOG_W("Don't bind a nullptr menu");
    return;
  }

  m_menu->operator+=(*menu);
  AddItselfAction2Menu();
}

void CustomListView::AddItselfAction2Menu() {
  m_menu->addSeparator();
  m_menu->addMenu(_TEXT_ELIDE_MODE_MENU);
  m_menu->addAction(_FLOW_ORIENTATION);
  m_menu->addAction(_VIEW_MODE_LIST_ICON);
  m_menu->addAction(_RESIZED_MODE_FIXED_OR_ADJUST);
  m_menu->addAction(_WRAPING_ACTIONS);
  m_menu->addAction(_UNIFORM_ITEM_SIZES);
}

void CustomListView::onTextElideModeChanged(const QAction* pAct) {
  CHECK_NULLPTR_RETURN_VOID(pAct);
  const Qt::TextElideMode newTextElideEnum = mTextEditModeIntAction.act2Enum(pAct);
  setTextElideMode(newTextElideEnum);
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
  setIconSize(IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconSizeIndex]);
  // setGridSize(IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconSizeIndex]);
}

void CustomListView::mousePressEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event)
  if (View::onMouseSidekeyBackwardForward(event->modifiers(), event->button())) {
    event->accept();
    return;
  }
  QListView::mousePressEvent(event);
}

bool CustomListView::setIconSizeScaledIndex(int newScaledIndex) {
  if (newScaledIndex < 0 || newScaledIndex >= IMAGE_SIZE::ICON_SIZE_CANDIDATES_N) {
    return false;
  }
  mCurIconSizeIndex = newScaledIndex;
  return true;
}
