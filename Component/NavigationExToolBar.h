#ifndef NAVIGATIONEXTOOLBAR_H
#define NAVIGATIONEXTOOLBAR_H

#include "public/PublicVariable.h"
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QToolBar>

class NavigationExToolBar : public QToolBar {
  Q_OBJECT
 public:
  explicit NavigationExToolBar(const QString& title, QWidget* parent = nullptr);
  void addDraggableAction(QAction* act);

  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;

  void ReadSettings();
  void Subscribe();

  void SaveName2PathLink();
  void UnpinThis();
  void UnpinAll();

  void CustomContextMenuEvent(const QPoint& pnt);

  void AlighLeft();
  void AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath);
  static void BindIntoNewPath(T_IntoNewPath IntoNewPath) { m_IntoNewPath = IntoNewPath; }
  static bool onPathActionTriggered(const QAction* pAct);

 signals:
  void widgetMoved(int fromIndex, int destIndex);

 private:
  bool MoveToolbuttonInToolBar(int fromIndex, int destIndex);
  static T_IntoNewPath m_IntoNewPath;

  QPoint mRightClickAtPnt;
  QPoint mDragStartPosition;
  QAction *UNPIN_THIS{nullptr}, *UNPIN_ALL{nullptr};
  QMenu* mMenu{nullptr};
};

#endif  // NAVIGATIONEXTOOLBAR_H
