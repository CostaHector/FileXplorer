#ifndef RIGHTCLICKABLETOOLBAR_H
#define RIGHTCLICKABLETOOLBAR_H

#include <QToolBar>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragMoveEvent>

#include <QFileInfo>
#include "PublicVariable.h"

#include <QMenu>
#include <QLayout>
#include <QApplication>
#include <QStyle>

class RightClickableToolBar : public QToolBar
{
public:
    RightClickableToolBar(const QString& title);
    void dragEnterEvent(QDragEnterEvent* event) override;

    void dropEvent(QDropEvent* event) override;

    void dragMoveEvent(QDragMoveEvent* event) override;

    void _save();

    void readSettings();

    void _unpin();

    void _unpinAll();

    void _switchTextBesideIcon(const QAction* act);

    using QToolBar::contextMenuEvent;
    void CustomContextMenuEvent(const QPoint& pnt);

    void alighLeft();

    void AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath);

    bool subscribe(T_IntoNewPath IntoNewPath = nullptr);

    QActionGroup* extraAG;
    QMap<QString, QString> extraShownText2Path;
    QPoint rightClickedPos;
    QAction* SHOW_TOOL_BUTTON_TEXT;
    QAction* SHOW_TOOL_BUTTON_ICON;
    QAction* SHOW_TOOL_BUTTON_TEXT_BESIDE_ICON;
    QActionGroup* textIconActionGroup;
    QMenu* menuQWidget;
};

#endif // RIGHTCLICKABLETOOLBAR_H
