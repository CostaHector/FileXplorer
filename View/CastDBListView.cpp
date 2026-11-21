#include "CastDBListView.h"
#include "CastCardDelegate.h"
#include "PublicCheckMacro.h"
#include "StyleSheet.h"
#include "Configuration.h"
#include "SizeTool.h"

CastDBListView::CastDBListView(QWidget *parent) : CustomListView{"CAST_DB_LISTVIEW", parent}  {}

CastDBListView::CastDBListView(CastDbModel* castDbModel_, QWidget* parent) : CastDBListView{parent}  {
  CHECK_NULLPTR_RETURN_VOID(castDbModel_)
  setModel(castDbModel_);

  // IconMode 默认 Movement::Free, 用户能把卡片拖走, 布局会散掉
  setMovement(QListView::Movement::Static);
  setSpacing(0);              // 留白交给 delegate 的 margin; gridSize 生效时 spacing 本来也无效
  setMouseTracking(true);     // 让 State_MouseOver 生效, 悬浮边框才有用

  m_castCardDel = new (std::nothrow) CastCardDelegate{this};
  CHECK_NULLPTR_RETURN_VOID(m_castCardDel)
  setItemDelegate(m_castCardDel);

  InitListView();

  // 显式初始化卡片尺寸, 不依赖 InitListView() 内部 setIconSize() 是否恰好触发了 iconSizeChanged
  onIconSizeChanged(iconSize()); // init icon size

  connect(this, &QListView::iconSizeChanged, this, &CastDBListView::onIconSizeChanged);

  setWindowIcon(QIcon{":img/SHOW_CASTS_IN_CARDS_LIST"});
  setWindowTitle(tr("Card View"));
}

void CastDBListView::initExclusivePreferenceSetting() {
  CustomListView::m_defaultFlowLeft2Right = true;
  CustomListView::m_defaultViewModeIcon = true;
  CustomListView::m_defaultWrapping = true;
  CustomListView::m_uniformItemSize = true;
  CustomListView::m_resizeModeAdjustFixed = true;
}

void CastDBListView::onIconSizeChanged(const QSize& iconSize) {
  CHECK_NULLPTR_RETURN_VOID(m_castCardDel)
  if (iconSize == m_castCardDel->cardSize()) {
    return;
  }
  m_castCardDel->setCardSize(iconSize);
  // gridSize 一变, QListView 就会做一次全量重新布局。 不要"对每个 index emit sizeHintChanged"
  setGridSize(iconSize);
}

void CastDBListView::showEvent(QShowEvent* event) {
  if (!m_geometryRestoredOnce) {
    ReadSettings();
  }
  StyleSheet::UpdateTitleBar(this);
  CustomListView::showEvent(event);
}

void CastDBListView::closeEvent(QCloseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event)
  Configuration().setValue("Geometry/CastDBListView", saveGeometry());
  CustomListView::closeEvent(event);
}

void CastDBListView::ReadSettings() {
  m_geometryRestoredOnce = true;
  if (Configuration().contains("Geometry/CastDBListView")) {
    restoreGeometry(Configuration().value("Geometry/CastDBListView").toByteArray());
  } else {
    setGeometry(SizeTool::DEFAULT_GEOMETRY);
  }
}
