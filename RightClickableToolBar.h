#ifndef RIGHTCLICKABLETOOLBAR_H
#define RIGHTCLICKABLETOOLBAR_H

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QToolBar>

#include <QFileInfo>
#include "PublicVariable.h"

#include <QApplication>
#include <QLayout>
#include <QMenu>
#include <QStyle>

class RightClickableToolBar : public QToolBar {
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

  bool subscribe();

  bool bindIntoNewPath(T_IntoNewPath IntoNewPath = nullptr) { m_IntoNewPath = IntoNewPath; }

  QActionGroup* extraAG;
  QPoint rightClickedPos;

  QAction *UNPIN, *UNPIN_ALL;
  QAction* SHOW_TOOL_BUTTON_TEXT;
  QAction* SHOW_TOOL_BUTTON_ICON;
  QAction* SHOW_TOOL_BUTTON_TEXT_BESIDE_ICON;
  QActionGroup* textIconActionGroup;

  T_IntoNewPath m_IntoNewPath;
  QMenu* menuQWidget;
};

#endif  // RIGHTCLICKABLETOOLBAR_H
