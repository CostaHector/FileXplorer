#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include <QToolBar>
#include <QActionGroup>
#include <QAction>
#include <QList>
#include "RightClickableToolBar.h"


struct NavigationLable{
public:
    QString name;
    QString path;
    QAction* action;
};

class NavigationToolBar : public QToolBar
{
public:
    NavigationToolBar(const QString& title = "NavigationToolBar", bool isShow_ = true);
    void GetFixedActions();
    bool subscribe(T_IntoNewPath IntoNewPath);
    void AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath);

protected:
    QActionGroup* fixedAG;
    QMap<QString, QString> shownText2Path;
    QList<NavigationLable> labelsLst;
    RightClickableToolBar* extraAppendTB;
};

#endif // NAVIGATIONTOOLBAR_H
