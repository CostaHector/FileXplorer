#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include <QAction>
#include <QActionGroup>
#include <QList>
#include <QToolBar>
#include "RightClickableToolBar.h"

struct NavigationLabel {
 public:
  QString name;
  QString path;
  QAction* action;
};

class NavigationToolBar : public QToolBar {
 public:
  NavigationToolBar(const QString& title = "NavigationToolBar", bool isShow_ = true);
  void GetFixedActions();
  bool subscribe(T_IntoNewPath IntoNewPath);
  void AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath);

 protected:
  QActionGroup* fixedAG;
  QMap<QString, QString> shownText2Path;
  QList<NavigationLabel> labelsLst;
  RightClickableToolBar* extraAppendTB;
};

#endif  // NAVIGATIONTOOLBAR_H
