﻿#include "NavigationToolBar.h"
#include "Actions/DevicesDrivesActions.h"
#include "public/OnCheckedPopupOrHideAWidget.h"
#include "public/PublicMacro.h"
#include "View/DevicesDrivesTV.h"
#include <QApplication>
#include <QDir>
#include <QStyle>

NavigationToolBar::NavigationToolBar(const QString& title, bool isShow_)  //
    : QToolBar{title}                                                     //
{
  setObjectName(title);
  // 1. devices and drives
  addAction(DevicesDrivesActions::Inst().DEVICES_AND_DRIVES);
  addSeparator();
  // 2. all home links
  auto* desktop = new (std::nothrow) QAction{QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DesktopIcon), "Desktop", this};
  CHECK_NULLPTR_RETURN_VOID(desktop)
  desktop->setToolTip(QDir::homePath() + "/Desktop");
  addAction(desktop);
  auto* pDocument = new (std::nothrow) QAction{QIcon(":img/FOLDER_OF_DOCUMENTS"), "Documents", this};
  CHECK_NULLPTR_RETURN_VOID(pDocument)
  pDocument->setToolTip(QDir::homePath() + "/Documents");
  addAction(pDocument);
  auto* pDownloads = new (std::nothrow) QAction{QIcon(":img/FOLDER_OF_DOWNLOADS"), "Downloads", this};
  CHECK_NULLPTR_RETURN_VOID(pDownloads)
  pDownloads->setToolTip(QDir::homePath() + "/Downloads");
  addAction(pDownloads);
  auto* pPictures = new (std::nothrow) QAction{QIcon(":img/FOLDER_OF_PICTURES"), "Pictures", this};
  CHECK_NULLPTR_RETURN_VOID(pPictures)
  pPictures->setToolTip(QDir::homePath() + "/Pictures");
  addAction(pPictures);
  auto* pVideos = new (std::nothrow) QAction{QIcon(":img/FOLDER_OF_VIDEOS"), "Videos", this};
  CHECK_NULLPTR_RETURN_VOID(pVideos)
  pVideos->setToolTip(QDir::homePath() + "/Videos");
  addAction(pVideos);
  auto* pFavorite = new (std::nothrow) QAction{QIcon(":img/FOLDER_OF_FAVORITE"), "Favorites", this};
  CHECK_NULLPTR_RETURN_VOID(pFavorite)
  pFavorite->setToolTip(QDir::homePath() + "/Documents");
  addAction(pFavorite);
  addSeparator();
  // 3. all volumes
  foreach (const QFileInfo& fi, QDir::drives()) {
    auto* pVolume = new (std::nothrow) QAction{fi.absoluteFilePath(), this};
    CHECK_NULLPTR_RETURN_VOID(pVolume)
    pVolume->setToolTip(fi.absoluteFilePath());
    addAction(pVolume);
  }
  addSeparator();
  // 4. all collections
  m_extraAppendTB = new (std::nothrow) NavigationExToolBar{"ExtraNavigation"};
  CHECK_NULLPTR_RETURN_VOID(m_extraAppendTB);
  addWidget(m_extraAppendTB);

  setOrientation(Qt::Vertical);
  setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  setMaximumWidth(40);

  if (!isShow_) {
    hide();
  }
  subscribe();
}

void NavigationToolBar::subscribe() {
  connect(DevicesDrivesActions::Inst().DEVICES_AND_DRIVES, &QAction::triggered, this, [this](const bool checked) -> void {  //
    mDevDriveTV = PopupHideWidget<DevicesDrivesTV>(mDevDriveTV, checked, nullptr);
  });
  connect(this, &QToolBar::actionTriggered, &NavigationExToolBar::onPathActionTriggered);
}

// #define __MAIN__EQ__NAME__ 1
#ifdef __MAIN__EQ__NAME__
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  NavigationToolBar naviTB{"nvTB", true};
  naviTB.show();
  // try drag a folder into NavigationExToolBar, it should create a link
  return a.exec();
}
#endif
