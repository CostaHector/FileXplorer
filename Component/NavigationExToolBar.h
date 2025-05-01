#ifndef NAVIGATIONEXTOOLBAR_H
#define NAVIGATIONEXTOOLBAR_H

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QToolBar>

class NavigationExToolBar : public QToolBar {
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

 private:
  QPoint mRightClickAtPnt;
  QAction *UNPIN_THIS, *UNPIN_ALL;
  QMenu* mMenu;
};

#endif  // NAVIGATIONEXTOOLBAR_H
