#include "NavigationToolBar.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QStyle>

NavigationToolBar::NavigationToolBar(const QString& title, bool isShow_)
    : QToolBar(title),
      fixedAG(new QActionGroup(this)),
      labelsLst{
          NavigationLabel{"Home", QDir::homePath(), new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirHomeIcon), "")},
          NavigationLabel{"Desktop", QString("%1/%2").arg(QDir::homePath(), "Desktop"),
                          new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DesktopIcon), "")},
          NavigationLabel{"Documents", QString("%1/%2").arg(QDir::homePath(), "Documents"), new QAction(QIcon(":/themes/FOLDER_OF_DOCUMENTS"), "")},
          NavigationLabel{"Downloads", QString("%1/%2").arg(QDir::homePath(), "Downloads"), new QAction(QIcon(":/themes/FOLDER_OF_DOWNLOADS"), "")},
          NavigationLabel{"Pictures", QString("%1/%2").arg(QDir::homePath(), "Pictures"), new QAction(QIcon(":/themes/FOLDER_OF_PICTURES"), "")},
          NavigationLabel{"Videos", QString("%1/%2").arg(QDir::homePath(), "Videos"), new QAction(QIcon(":/themes/FOLDER_OF_VIDEOS"), "")},
          NavigationLabel{"Starred", "", new QAction(QIcon(":/themes/FOLDER_OF_FAVORITE"), "")},
          NavigationLabel{"Computer", "", new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_ComputerIcon), "")}},
      extraAppendTB(new RightClickableToolBar("ExtraNavigation")) {
  setObjectName(title);
  GetFixedActions();
  addActions(fixedAG->actions());
  addSeparator();
  addWidget(extraAppendTB);

  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  setOrientation(Qt::Vertical);

  if (not isShow_) {
    hide();
  }
}

void NavigationToolBar::GetFixedActions() {
  for (NavigationLabel& lb : labelsLst) {
    lb.action->setText(lb.name);
    lb.action->setCheckable(false);
    fixedAG->addAction(lb.action);
    shownText2Path[lb.name] = lb.path;
  }
}

bool NavigationToolBar::subscribe(T_IntoNewPath IntoNewPath) {
  connect(this, &QToolBar::actionTriggered, this, [this, IntoNewPath](const QAction* act) {
    if (!IntoNewPath) {
      qDebug("RightClickableToolBar, IntoNewPath is nullptr");
      return;
    }
    IntoNewPath(shownText2Path[act->text()], true, true);
  });
  extraAppendTB->subscribe(IntoNewPath);
  return true;
}

void NavigationToolBar::AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath) {
  if (folderName2AbsPath.isEmpty()) {
    return;
  }
  extraAppendTB->AppendExtraActions(folderName2AbsPath);
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
    qDebug("%s,%d,%d", path.toStdString().c_str(), isNewPath, isForce);
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
