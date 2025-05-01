#include "NavigationToolBar.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QStyle>
#include "Actions/ActionWithPath.h"
#include "public/PublicMacro.h"

NavigationToolBar::NavigationToolBar(const QString& title, bool isShow_)  //
    : QToolBar{title} {
  setObjectName(title);

  static const QString TEMPLATE{QDir::homePath() + "/%1"};
  addAction(new (std::nothrow) ActionWithPath{TEMPLATE.arg("Desktop"), QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DesktopIcon), "Desktop", this});
  addAction(new (std::nothrow) ActionWithPath{TEMPLATE.arg("Documents"), QIcon(":img/FOLDER_OF_DOCUMENTS"), "Documents", this});
  addAction(new (std::nothrow) ActionWithPath{TEMPLATE.arg("Downloads"), QIcon(":img/FOLDER_OF_DOWNLOADS"), "Downloads", this});
  addAction(new (std::nothrow) ActionWithPath{TEMPLATE.arg("Pictures"), QIcon(":img/FOLDER_OF_PICTURES"), "Pictures", this});
  addAction(new (std::nothrow) ActionWithPath{TEMPLATE.arg("Videos"), QIcon(":img/FOLDER_OF_VIDEOS"), "Videos", this});
  addAction(new (std::nothrow) ActionWithPath{"", QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_ComputerIcon), "Computer", this});
  addSeparator();
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
}

// #define __MAIN__EQ__NAME__ 1
#ifdef __MAIN__EQ__NAME__
#include <QApplication>
#include <QDir>
#include <QFileSystemModel>
#include <QMainWindow>
#include <QPushButton>
#include <QTableView>

int main(int argc, char* argv[]) {
  QString rootPath = QFileInfo(__FILE__).absolutePath();
  QString testDir = QDir(rootPath).absoluteFilePath("test");

  QApplication a(argc, argv);

  QTableView* tb = new QTableView;
  QFileSystemModel* fsm = new QFileSystemModel;
  tb->setModel(fsm);
  tb->setRootIndex(fsm->setRootPath(testDir));

  NavigationToolBar* naviTB = new NavigationToolBar();
  naviTB->subscribe([](const QString& path, bool isNewPath, bool isForce) -> bool {
    qDebug("%s,%d,%d", qPrintable(path), isNewPath, isForce);
    return true;
  });
  auto* appendPins = new QPushButton("append pins");
  QPushButton::connect(appendPins, &QPushButton::clicked, nullptr, [naviTB]() { naviTB->AppendExtraActions({{"1", "path/to/1"}}); });

  QToolBar* topRibbons = new QToolBar("TopRibbons");
  topRibbons->addWidget(appendPins);

  QMainWindow mw;
  mw.setCentralWidget(tb);
  mw.addToolBar(Qt::ToolBarArea::LeftToolBarArea, naviTB);
  mw.addToolBar(Qt::ToolBarArea::TopToolBarArea, topRibbons);

  mw.show();
  return a.exec();
}
#endif
