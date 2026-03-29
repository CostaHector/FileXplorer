#include "NavigationToolBar.h"
#include "PopupWidgetManager.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include "SizeChangeAnimation.h"
#include "ImageTool.h"
#include <QApplication>
#include <QDir>
#include <QStyle>
#include <QTimer>

constexpr int NavigationToolBar::MAXIMUM_WIDTH_WHEN_NOT_EXPAND, NavigationToolBar::MAXIMUM_WIDTH_WHEN_EXPAND;

NavigationToolBar::NavigationToolBar(const QString& title, QWidget* parent) //
  : QToolBar{title, parent}                                                 //
{
  setObjectName(title);

  const bool bExpandSideBar{
      Configuration().value(MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.v).toBool()};
  setOrientation(Qt::Vertical);
  setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
  setIconSize(QSize{IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24});

  // 0. expand and minimum
  {
    EXPAND_SIDEBAR = new (std::nothrow) QAction{QIcon(":img/EXPAND_SIDEBAR"), tr("Expand sidebar"), this};
    CHECK_NULLPTR_RETURN_VOID(EXPAND_SIDEBAR);
    EXPAND_SIDEBAR->setCheckable(true);
    EXPAND_SIDEBAR->setChecked(bExpandSideBar);
    addAction(EXPAND_SIDEBAR);
    addSeparator();
  }

  // 1. devices and drives
  {
    DEVICES_AND_DRIVES = new (std::nothrow) QAction{QIcon(":img/DISKS"), tr("Devices and Drives"), this};
    CHECK_NULLPTR_RETURN_VOID(DEVICES_AND_DRIVES);
    DEVICES_AND_DRIVES->setCheckable(true);
    addAction(DEVICES_AND_DRIVES);
    addSeparator();
  }

  {
    // 2. all home links
    auto* pDesktop = addAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DesktopIcon), tr("Desktop"));
    auto* pDocument = addAction(QIcon(":img/FOLDER_OF_DOCUMENTS"), tr("Documents"));
    auto* pDownloads = addAction(QIcon(":img/FOLDER_OF_DOWNLOADS"), tr("Downloads"));
    auto* pPictures = addAction(QIcon(":img/FOLDER_OF_PICTURES"), tr("Pictures"));
    auto* pVideos = addAction(QIcon(":img/FOLDER_OF_VIDEOS"), tr("Videos"));
    auto* pFavorite = addAction(QIcon(":img/FOLDER_OF_FAVORITE"), tr("Favorites"));
    CHECK_NULLPTR_RETURN_VOID(pDesktop)
    CHECK_NULLPTR_RETURN_VOID(pDocument)
    CHECK_NULLPTR_RETURN_VOID(pDownloads)
    CHECK_NULLPTR_RETURN_VOID(pPictures)
    CHECK_NULLPTR_RETURN_VOID(pVideos)
    CHECK_NULLPTR_RETURN_VOID(pFavorite)
    pDesktop->setData(SystemPath::HOME_PATH() + "/Desktop");
    pDocument->setData(SystemPath::HOME_PATH() + "/Documents");
    pDownloads->setData(SystemPath::HOME_PATH() + "/Downloads");
    pPictures->setData(SystemPath::HOME_PATH() + "/Pictures");
    pVideos->setData(SystemPath::HOME_PATH() + "/Videos");
    pFavorite->setData(SystemPath::HOME_PATH() + "/Documents");
    pDesktop->setToolTip(QString{"<b>%1</b><br/> %2"}.arg(pDesktop->text(), pDesktop->data().toString()));
    pDocument->setToolTip(QString{"<b>%1</b><br/> %2"}.arg(pDocument->text(), pDocument->data().toString()));
    pDownloads->setToolTip(QString{"<b>%1</b><br/> %2"}.arg(pDownloads->text(), pDownloads->data().toString()));
    pPictures->setToolTip(QString{"<b>%1</b><br/> %2"}.arg(pPictures->text(), pPictures->data().toString()));
    pVideos->setToolTip(QString{"<b>%1</b><br/> %2"}.arg(pVideos->text(), pVideos->data().toString()));
    pFavorite->setToolTip(QString{"<b>%1</b><br/> %2"}.arg(pFavorite->text(), pFavorite->data().toString()));
    addSeparator();

    m_pathActionGroups = new (std::nothrow) QActionGroup{this};
    CHECK_NULLPTR_RETURN_VOID(m_pathActionGroups);
    m_pathActionGroups->addAction(pDesktop);
    m_pathActionGroups->addAction(pDesktop);
    m_pathActionGroups->addAction(pDownloads);
    m_pathActionGroups->addAction(pPictures);
    m_pathActionGroups->addAction(pVideos);
    m_pathActionGroups->addAction(pFavorite);

    // 3. all volumes
    foreach (const QFileInfo& fi, QDir::drives()) {
      const QString diskAbsPath{fi.absoluteFilePath()};
      auto* pVolume = addAction(diskAbsPath);
      CHECK_NULLPTR_RETURN_VOID(pVolume)
      pVolume->setData(diskAbsPath);
      pVolume->setToolTip(QString{"<b>Disk: %1</b>"}.arg(diskAbsPath));
      m_pathActionGroups->addAction(pVolume);
    }
    addSeparator();
  }

  // 4. all collections
  m_extraAppendTB = new (std::nothrow) NavigationExToolBar{"ExtraNavigation", this};
  CHECK_NULLPTR_RETURN_VOID(m_extraAppendTB);
  addWidget(m_extraAppendTB);

  SetLayoutAlightment(layout(), Qt::AlignmentFlag::AlignLeft);
#ifdef RUNNING_UNIT_TESTS
  updateToolbarButtonStyle(EXPAND_SIDEBAR->isChecked(), false);
#else
  QTimer::singleShot(0, this, [this]() { updateToolbarButtonStyle(EXPAND_SIDEBAR->isChecked(), false); });
#endif
  subscribe();
}

NavigationToolBar::~NavigationToolBar() { //
  Configuration().setValue(MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.name, EXPAND_SIDEBAR->isChecked());
}

void NavigationToolBar::subscribe() {
  mDevDriveTV = new (std::nothrow) PopupWidgetManager<DevicesDrivesTV>{DEVICES_AND_DRIVES, this, "DevicesDrivesTVGeometry"};
  CHECK_NULLPTR_RETURN_VOID(mDevDriveTV);
  connect(m_pathActionGroups, &QActionGroup::triggered, this, &NavigationExToolBar::onPathActionTriggeredNavi);
  connect(EXPAND_SIDEBAR, &QAction::toggled, this, &NavigationToolBar::onExpandSidebar);
}

void NavigationToolBar::onExpandSidebar(bool bExpand) {
  updateToolbarButtonStyle(bExpand, true);
}

void NavigationToolBar::updateToolbarButtonStyle(bool bExpand, bool bAnimationEnabled) {
#ifdef RUNNING_UNIT_TESTS // no need animation in testcase
  bAnimationEnabled = false;
#endif
  SizeChangeAnimation ani{orientation(), MAXIMUM_WIDTH_WHEN_EXPAND, MAXIMUM_WIDTH_WHEN_NOT_EXPAND, bAnimationEnabled};
  if (bExpand) { // 扩展场景, 需要事先设置Icon旁文字, 这样计算的宽度是最终宽度
    ani.registerCallbackBeforeStart([this] { setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon); });
  } else { // 收缩场景, 先收缩到一个很小值, 再设置仅文字
    ani.registerCallbackWhenFinished([this] { setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly); });
  }
  ani(this, width(), bExpand);
}
