#include "NavigationToolBar.h"
#include "PopupWidgetManager.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include <QApplication>
#include <QDir>
#include <QStyle>

constexpr int NavigationToolBar::MAXIMUM_WIDTH_WHEN_NOT_EXPAND, NavigationToolBar::MAXIMUM_WIDTH_WHEN_EXPAND;

NavigationToolBar::NavigationToolBar(const QString& title, QWidget* parent)  //
    : QToolBar{title, parent}                                                //
{
  setObjectName(title);

  const bool bExpandSideBar{
      Configuration().value(MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.v).toBool()};
  setOrientation(Qt::Vertical);
  setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
  updateToolbarButtonStyle(bExpandSideBar);

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
      auto* pVolume = addAction(fi.absoluteFilePath());
      CHECK_NULLPTR_RETURN_VOID(pVolume)
      pVolume->setData(fi.absoluteFilePath());
      m_pathActionGroups->addAction(pVolume);
    }
    addSeparator();
  }

  // 4. all collections
  m_extraAppendTB = new (std::nothrow) NavigationExToolBar{"ExtraNavigation", this};
  CHECK_NULLPTR_RETURN_VOID(m_extraAppendTB);
  addWidget(m_extraAppendTB);

  SetLayoutAlightment(layout(), Qt::AlignmentFlag::AlignLeft);

  subscribe();
}

NavigationToolBar::~NavigationToolBar() {  //
  Configuration().setValue(MemoryKey::EXPAND_QUICK_NAVIGATION_TOOL_BAR.name, EXPAND_SIDEBAR->isChecked());
}

void NavigationToolBar::subscribe() {
  mDevDriveTV = new (std::nothrow) PopupWidgetManager<DevicesDrivesTV>{DEVICES_AND_DRIVES, this, "DevicesDrivesTVGeometry"};
  CHECK_NULLPTR_RETURN_VOID(mDevDriveTV);
  connect(m_pathActionGroups, &QActionGroup::triggered, this, &NavigationExToolBar::onPathActionTriggeredNavi);
  connect(EXPAND_SIDEBAR, &QAction::toggled, this, &NavigationToolBar::onExpandSidebar);
}

void NavigationToolBar::onExpandSidebar(bool bExpand) {
  updateToolbarButtonStyle(bExpand);
}

void NavigationToolBar::updateToolbarButtonStyle(bool bExpand) {
  setToolButtonStyle(bExpand ? Qt::ToolButtonStyle::ToolButtonTextBesideIcon : Qt::ToolButtonStyle::ToolButtonIconOnly);
  if (bExpand) {
    setMaximumWidth(MAXIMUM_WIDTH_WHEN_EXPAND);
  } else {
    setMaximumWidth(MAXIMUM_WIDTH_WHEN_NOT_EXPAND);
  }
}
