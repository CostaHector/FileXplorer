#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include <QAction>
#include <QActionGroup>
#include <QList>
#include <QToolBar>
#include "RightClickableToolBar.h"

class NavigationToolBar : public QToolBar {
 public:
  NavigationToolBar(const QString& title = "NavigationToolBar", bool isShow_ = true);
  void InitFixedActions();
  void AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath);
 protected:
  RightClickableToolBar* m_extraAppendTB;
};

#endif  // NAVIGATIONTOOLBAR_H
