#ifndef REORDERABLETOOLBAR_H
#define REORDERABLETOOLBAR_H

#include <QToolBar>
#include <QBoxLayout>
class ReorderableToolBar : public QToolBar {
  Q_OBJECT
 public:
  explicit ReorderableToolBar(const QString &title, QWidget *parent = nullptr);
  explicit ReorderableToolBar(QWidget *parent = nullptr);
  void addDraggableAction(QAction* act);
  QActionGroup* mCollectPathAgs{nullptr};

 signals:
  void widgetMoved(int fromIndex, int destIndex);
 protected:
  void actionEvent(QActionEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;

 private:
  bool MoveToolbuttonInToolBar(int fromIndex, int destIndex);
};

#include <QSplitter>
#include <QBoxLayout>
bool MoveWidgetAtFromIndexInFrontOfDestIndex(int fromIndex, int destIndex, QSplitter& splitter);
bool MoveWidgetAtFromIndexInFrontOfDestIndex(int fromIndex, int destIndex, QBoxLayout& layout);

#endif // REORDERABLETOOLBAR_H
