#include "NavigationToolBar.h"
#include "Actions/ActionWithPath.h"
#include "public/OnCheckedPopupOrHideAWidget.h"
#include "public/PublicMacro.h"
#include "View/DevicesDriveTableView.h"
#include <QApplication>
#include <QDir>
#include <QStyle>

NavigationToolBar::NavigationToolBar(const QString& title, bool isShow_)  //
    : QToolBar{title} {
  setObjectName(title);
  // 1. devices and drives
  DEVICES_AND_DRIVES = new (std::nothrow) QAction{QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DriveHDIcon), "Devices and Drives", this};
  CHECK_NULLPTR_RETURN_VOID(DEVICES_AND_DRIVES);
  addAction(DEVICES_AND_DRIVES);
  DEVICES_AND_DRIVES->setCheckable(true);
  addSeparator();
  // 2. all home links
  static const QString TEMPLATE{QDir::homePath() + "/%1"};
  addAction(new (std::nothrow) ActionWithPath{TEMPLATE.arg("Desktop"), QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DesktopIcon), "Desktop", this});
  addAction(new (std::nothrow) ActionWithPath{TEMPLATE.arg("Documents"), QIcon(":img/FOLDER_OF_DOCUMENTS"), "Documents", this});
  addAction(new (std::nothrow) ActionWithPath{TEMPLATE.arg("Downloads"), QIcon(":img/FOLDER_OF_DOWNLOADS"), "Downloads", this});
  addAction(new (std::nothrow) ActionWithPath{TEMPLATE.arg("Pictures"), QIcon(":img/FOLDER_OF_PICTURES"), "Pictures", this});
  addAction(new (std::nothrow) ActionWithPath{TEMPLATE.arg("Videos"), QIcon(":img/FOLDER_OF_VIDEOS"), "Videos", this});
  addSeparator();
  // 3. all volumes
  addAction(new (std::nothrow) ActionWithPath{"", QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_ComputerIcon), "Computer", this});
  foreach (const QFileInfo& fi, QDir::drives()) {
    addAction(new (std::nothrow) ActionWithPath{fi.absoluteFilePath(), fi.absoluteFilePath(), this});
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
  connect(DEVICES_AND_DRIVES, &QAction::triggered, this, [this](const bool checked) {  //
    mDevDriveTV = PopupHideWidget<DevicesDriveTableView>(mDevDriveTV, checked, nullptr);
  });
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
