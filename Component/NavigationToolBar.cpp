#include "NavigationToolBar.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QStyle>
#include "Actions/ActionWithPath.h"

NavigationToolBar::NavigationToolBar(const QString& title, bool isShow_)
    : QToolBar(title), m_extraAppendTB(new NavigationExToolBar("ExtraNavigation")) {
  setObjectName(title);
  InitFixedActions();
  addSeparator();
  addWidget(m_extraAppendTB);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  setOrientation(Qt::Vertical);

  if (not isShow_) {
    hide();
  }
}

void NavigationToolBar::InitFixedActions() {
  this->addAction(new ActionWithPath(QString("%1/%2").arg(QDir::homePath(), "Desktop"),
                                        QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DesktopIcon), "Desktop"));
  this->addAction(new ActionWithPath(QString("%1/%2").arg(QDir::homePath(), "Documents"), QIcon(":img/FOLDER_OF_DOCUMENTS"), "Documents"));
  this->addAction(new ActionWithPath(QString("%1/%2").arg(QDir::homePath(), "Downloads"), QIcon(":img/FOLDER_OF_DOWNLOADS"), "Downloads"));
  this->addAction(new ActionWithPath(QString("%1/%2").arg(QDir::homePath(), "Pictures"), QIcon(":img/FOLDER_OF_PICTURES"), "Pictures"));
  this->addAction(new ActionWithPath(QString("%1/%2").arg(QDir::homePath(), "Videos"), QIcon(":img/FOLDER_OF_VIDEOS"), "    Videos"));
  this->addAction(new ActionWithPath("", QIcon(":img/FOLDER_OF_FAVORITE"), "Starred"));
  this->addAction(new ActionWithPath("", QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_ComputerIcon), "Computer"));
}

void NavigationToolBar::AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath) {
  if (folderName2AbsPath.isEmpty()) {
    return;
  }
  m_extraAppendTB->AppendExtraActions(folderName2AbsPath);
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
