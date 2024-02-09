#include "NavigationToolBar.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QStyle>

NavigationToolBar::NavigationToolBar(const QString& title, bool isShow_)
    : QToolBar(title),
      m_fixedAG(new QActionGroup(this)),
      m_labelsLst{
          NavigationLabel{QDir::homePath(), new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirHomeIcon), "Home")},
          NavigationLabel{QString("%1/%2").arg(QDir::homePath(), "Desktop"),
                          new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DesktopIcon), "Desktop")},
          NavigationLabel{QString("%1/%2").arg(QDir::homePath(), "Documents"), new QAction(QIcon(":/themes/FOLDER_OF_DOCUMENTS"), "Documents")},
          NavigationLabel{QString("%1/%2").arg(QDir::homePath(), "Downloads"), new QAction(QIcon(":/themes/FOLDER_OF_DOWNLOADS"), "Downloads")},
          NavigationLabel{QString("%1/%2").arg(QDir::homePath(), "Pictures"), new QAction(QIcon(":/themes/FOLDER_OF_PICTURES"), "Pictures")},
          NavigationLabel{QString("%1/%2").arg(QDir::homePath(), "Videos"), new QAction(QIcon(":/themes/FOLDER_OF_VIDEOS"), "Videos")},
          NavigationLabel{"", new QAction(QIcon(":/themes/FOLDER_OF_FAVORITE"), "Starred")},
          NavigationLabel{"", new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_ComputerIcon), "Computer")}},
      m_extraAppendTB(new RightClickableToolBar("ExtraNavigation")) {
  setObjectName(title);
  GetFixedActions();
  addActions(m_fixedAG->actions());
  addSeparator();
  addWidget(m_extraAppendTB);

  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  setOrientation(Qt::Vertical);

  if (not isShow_) {
    hide();
  }
}

void NavigationToolBar::GetFixedActions() {
  for (NavigationLabel& lb : m_labelsLst) {
    lb.action->setCheckable(false);
    lb.action->setToolTip(lb.path);
    m_fixedAG->addAction(lb.action);
  }
}

bool NavigationToolBar::subscribe(T_IntoNewPath IntoNewPath) {
  connect(this, &QToolBar::actionTriggered, this, [this, IntoNewPath](const QAction* act) {
    if (!IntoNewPath) {
      qDebug("RightClickableToolBar, IntoNewPath is nullptr");
      return;
    }
    IntoNewPath(act->toolTip(), true, true);
  });
  m_extraAppendTB->bindIntoNewPath(IntoNewPath);
  return true;
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
