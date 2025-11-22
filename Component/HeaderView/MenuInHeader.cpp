#include "MenuInHeader.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "PublicMacro.h"
#include <QContextMenuEvent>
#include <QInputDialog>

constexpr int MenuInHeader::TABLE_DEFAULT_ROW_SECTION_SIZE;
constexpr int MenuInHeader::TABLE_DEFAULT_COLUMN_SECTION_SIZE;
constexpr int MenuInHeader::TABLE_MAX_ROW_SECTION_SIZE;
constexpr int MenuInHeader::TABLE_MAX_COLUMN_SECTION_SIZE;

constexpr int MenuInHeader::INVALID_CLICKED_COLUMN;

int MenuInHeader::GetDefaultSectionSizeFirstTime() const {
  const int defValue{orientation() == Qt::Orientation::Horizontal ? TABLE_DEFAULT_COLUMN_SECTION_SIZE : TABLE_DEFAULT_ROW_SECTION_SIZE};
  return Configuration().value(m_defaultSectionSizeKey, defValue).toInt();
}

int MenuInHeader::GetMaxSectionSizeFirstTime() const {
  const int defValue{orientation() == Qt::Orientation::Horizontal ? TABLE_MAX_COLUMN_SECTION_SIZE : TABLE_MAX_ROW_SECTION_SIZE};
  return Configuration().value(m_maxSectionSizeKey, defValue).toInt();
}

MenuInHeader::MenuInHeader(const QString &proName, Qt::Orientation ori, QWidget *parent)
  : QHeaderView{ori, parent}
  , m_name{proName}
  , m_defaultSectionSizeKey{GetName() + "_DEFAULT_SECTION_SIZE"}
  , m_maxSectionSizeKey{GetName() + "_MAX_SECTION_SIZE"}
  , m_stretchLastSectionKey{GetName() + "_STRETCH_LAST_SECTION"}
  , m_resizeModeKey{GetName() + "_RESIZE_MODE"}
  , m_headerStateKey{GetName() + "_HEADER_GEOMETRY"} {
  const QString sectionSizeIconPath{orientation() == Qt::Orientation::Horizontal ? ":img/DEFAULT_COLUMN_WIDTH" : ":img/DEFAULT_ROW_HEIGHT"};
  _SET_DEFAULT_SECTION_SIZE = new (std::nothrow) QAction(QIcon(sectionSizeIconPath), "Set default section size", this);
  CHECK_NULLPTR_RETURN_VOID(_SET_DEFAULT_SECTION_SIZE);
  _SET_MAX_SECTION_SIZE = new (std::nothrow) QAction("Set max section size", this);
  CHECK_NULLPTR_RETURN_VOID(_SET_MAX_SECTION_SIZE);

  _STRETCH_DETAIL_SECTION = new (std::nothrow) QAction("Stretch last column", this);
  CHECK_NULLPTR_RETURN_VOID(_STRETCH_DETAIL_SECTION);
  _STRETCH_DETAIL_SECTION->setCheckable(true);
  _STRETCH_DETAIL_SECTION->setChecked(Configuration().value(m_stretchLastSectionKey, false).toBool());

  {
    _RESIZE_MODE_INTERACTIVE = new (std::nothrow) QAction{QIcon{":img/RESIZE_MODE"}, "Interactive", this};
    CHECK_NULLPTR_RETURN_VOID(_RESIZE_MODE_INTERACTIVE);
    _RESIZE_MODE_INTERACTIVE->setCheckable(true);

    _RESIZE_MODE_STRETCH = new (std::nothrow) QAction{QIcon{":img/RESIZE_MODE_STRETCH"}, "Stretch", this};
    CHECK_NULLPTR_RETURN_VOID(_RESIZE_MODE_STRETCH);
    _RESIZE_MODE_STRETCH->setCheckable(true);

    _RESIZE_MODE_FIXED = new (std::nothrow) QAction{QIcon{":img/RESIZE_MODE_FIXED"}, "Fixed", this};
    CHECK_NULLPTR_RETURN_VOID(_RESIZE_MODE_FIXED);
    _RESIZE_MODE_FIXED->setCheckable(true);

    _RESIZE_MODE_RESIZE_TO_CONTENTS = new (std::nothrow) QAction{QIcon{":img/RESIZE_MODE_RESIZE_TO_CONTENTS"}, "Resize to contents", this};
    CHECK_NULLPTR_RETURN_VOID(_RESIZE_MODE_RESIZE_TO_CONTENTS);
    _RESIZE_MODE_RESIZE_TO_CONTENTS->setCheckable(true);

    mResizeModeIntAction.init(
        {
            {_RESIZE_MODE_INTERACTIVE, QHeaderView::ResizeMode::Interactive}, //
            {_RESIZE_MODE_STRETCH, QHeaderView::ResizeMode::Stretch},         //
            {_RESIZE_MODE_FIXED, QHeaderView::ResizeMode::Fixed},             //
            {_RESIZE_MODE_RESIZE_TO_CONTENTS, QHeaderView::ResizeMode::ResizeToContents},
        }, //
        QHeaderView::ResizeMode::Interactive,
        QActionGroup::ExclusionPolicy::Exclusive);

    int defResizeMode = Configuration().value(m_resizeModeKey, QHeaderView::ResizeMode::Interactive).toInt();
    QAction *checkedResizeModeAct = mResizeModeIntAction.setCheckedIfActionExist(defResizeMode);

    m_menu = new (std::nothrow) QMenu{m_name + " Header Menu", this};
    CHECK_NULLPTR_RETURN_VOID(m_menu);
    m_menu->setToolTipsVisible(true);
    m_menu->addAction(_SET_DEFAULT_SECTION_SIZE);
    m_menu->addAction(_SET_MAX_SECTION_SIZE);
    m_menu->addAction(_STRETCH_DETAIL_SECTION);
    QMenu *sectionResizeModeMenu = m_menu->addMenu(QIcon{":img/RESIZE_MODE"}, "Section resize mode");
    CHECK_NULLPTR_RETURN_VOID(sectionResizeModeMenu);
    sectionResizeModeMenu->addActions(mResizeModeIntAction.getActionEnumAscendingList());

    setStretchLastSection(_STRETCH_DETAIL_SECTION->isChecked());
    setDefaultSectionSize(GetDefaultSectionSizeFirstTime());
    setMaximumSectionSize(GetMaxSectionSizeFirstTime());
    setDefaultAlignment(Qt::AlignLeft);
    onSetSectionResizeMode(checkedResizeModeAct);
  }

  connect(mResizeModeIntAction.getActionGroup(), &QActionGroup::triggered, this, &MenuInHeader::onSetSectionResizeMode);
  connect(_STRETCH_DETAIL_SECTION, &QAction::toggled, this, &MenuInHeader::setStretchLastSection);
  connect(_SET_DEFAULT_SECTION_SIZE, &QAction::triggered, this, &MenuInHeader::onSetDefaultSectionSize);
  connect(_SET_MAX_SECTION_SIZE, &QAction::triggered, this, &MenuInHeader::onSetMaxSectionSize);
}

MenuInHeader::~MenuInHeader() {
  Configuration().setValue(m_defaultSectionSizeKey, defaultSectionSize());
  Configuration().setValue(m_maxSectionSizeKey, maximumSectionSize());
  Configuration().setValue(m_stretchLastSectionKey, stretchLastSection());
  Configuration().setValue(m_resizeModeKey, sectionResizeMode(0));
  Configuration().setValue(m_headerStateKey, saveState());
}

void MenuInHeader::AddActionToMenu(QAction *act) {
  if (act == nullptr) {
    m_menu->addSeparator();
    return;
  }
  m_menu->addAction(act);
}

void MenuInHeader::RestoreHeaderState() {
  restoreState(Configuration().value(m_headerStateKey, QByteArray{}).toByteArray());
}

void MenuInHeader::contextMenuEvent(QContextMenuEvent *e) {
  CHECK_NULLPTR_RETURN_VOID(e);
  CHECK_NULLPTR_RETURN_VOID(m_menu);
  if (m_menu != nullptr) {
    QPoint pnt = e->globalPos();
#ifndef RUNNING_UNIT_TESTS
    m_menu->popup(pnt);
#endif
  }
  setClickedSection(logicalIndexAt(e->pos()));
  e->accept();
}

void MenuInHeader::onSetSectionResizeMode(const QAction *pResizeToContent) {
  CHECK_NULLPTR_RETURN_VOID(pResizeToContent);
  QHeaderView::ResizeMode lastMode = sectionResizeMode(0);
  const QHeaderView::ResizeMode newMode = mResizeModeIntAction.act2Enum(pResizeToContent);
  if (newMode == lastMode) {
    return;
  }
  if (lastMode == QHeaderView::ResizeMode::ResizeToContents) {
    RestoreHeaderState();
  }
  setSectionResizeMode(newMode);
}

bool MenuInHeader::onSetDefaultSectionSize() {
  bool setOk{false};
  const int beforeDefSize = defaultSectionSize();
  int afterDefSize{beforeDefSize};
  const int minSize = minimumSectionSize(), maxSize = maximumSectionSize();
  constexpr int sizeStep = 10;
  const QString titleMsg{"Set Default Section Size"};
  const QString labelMsg{QString::asprintf("Current: %d px | Range: %d-%d px", beforeDefSize, minSize, maxSize)};
#ifdef RUNNING_UNIT_TESTS
  setOk = UserSpecifiedIntValueMock::mockBoolOk();
  afterDefSize = UserSpecifiedIntValueMock::mockIntValue();
#else
  afterDefSize = QInputDialog::getInt(this, titleMsg, labelMsg, beforeDefSize, minSize, maxSize, sizeStep, &setOk);
#endif
  if (!setOk) {
    LOG_W("User canceled default section size setting");
    return false;
  }
  if (afterDefSize == beforeDefSize) {
    LOG_D("Default section size unchanged: %d px", beforeDefSize);
    return true;
  }
  setDefaultSectionSize(afterDefSize);
  return true;
}

bool MenuInHeader::onSetMaxSectionSize() {
  bool setOk{false};
  const int beforeDefSize = maximumSectionSize();
  int afterDefSize{beforeDefSize};
  const int minSize = minimumSectionSize(), maxSize = 9999;
  constexpr int sizeStep = 10;
  const QString titleMsg{"Set Maximum Section Size"};
  const QString labelMsg{QString::asprintf("Current: %d px | Range: %d-%d px", beforeDefSize, minSize, maxSize)};
#ifdef RUNNING_UNIT_TESTS
  setOk = UserSpecifiedIntValueMock::mockBoolOk();
  afterDefSize = UserSpecifiedIntValueMock::mockIntValue();
#else
  afterDefSize = QInputDialog::getInt(this, titleMsg, labelMsg, beforeDefSize, minSize, maxSize, sizeStep, &setOk);
#endif
  if (!setOk) {
    LOG_W("User canceled maximum section size setting");
    return false;
  }
  if (afterDefSize == beforeDefSize) {
    LOG_D("Maximum section size unchanged: %d px", beforeDefSize);
    return true;
  }
  setMaximumSectionSize(afterDefSize);
  return true;
}
