#ifndef NAVIGATIONEXTOOLBAR_H
#define NAVIGATIONEXTOOLBAR_H

#include "PublicVariable.h"
#include "ReorderableToolBar.h"

class NavigationExToolBar : public ReorderableToolBar {
 public:
  explicit NavigationExToolBar(const QString& title, QWidget* parent = nullptr);

  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;

  void ReadSettings();
  void Subscribe();

  void SaveName2PathLink();
  bool UnpinThis();
  int UnpinAll();

  void contextMenuEvent(QContextMenuEvent* event) override;

  void AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath);
  static void BindIntoNewPathNavi(T_IntoNewPath IntoNewPath) { m_IntoNewPathNavi = IntoNewPath; }
  static bool onPathActionTriggeredNavi(const QAction* pAct);

 private:
  static T_IntoNewPath m_IntoNewPathNavi;

  QPoint mDragStartPosition;
  QAction *UNPIN_THIS{nullptr}, *UNPIN_ALL{nullptr};
  QAction *mRightClickAtAction{nullptr};
  QMenu* mMenu{nullptr};

  static constexpr char EXTRA_NAVI_DICT[] {"ExtraNavigationDict"};
  static constexpr char EXTRA_NAVI_DICT_KEY[] {"folderName"};
  static constexpr char EXTRA_NAVI_DICT_VALUE[] {"absPath"};
};
#endif  // NAVIGATIONEXTOOLBAR_H
