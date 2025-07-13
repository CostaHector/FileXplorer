#ifndef NAVIGATIONEXTOOLBAR_H
#define NAVIGATIONEXTOOLBAR_H

#include "public/PublicVariable.h"
#include "ReorderableToolBar.h"

class NavigationExToolBar : public ReorderableToolBar {
  Q_OBJECT
 public:
  explicit NavigationExToolBar(const QString& title, QWidget* parent = nullptr);

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

 private:
  static T_IntoNewPath m_IntoNewPath;

  QPoint mRightClickAtPnt;
  QPoint mDragStartPosition;
  QAction *UNPIN_THIS{nullptr}, *UNPIN_ALL{nullptr};
  QMenu* mMenu{nullptr};
};

#endif  // NAVIGATIONEXTOOLBAR_H
