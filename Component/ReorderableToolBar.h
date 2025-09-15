#ifndef REORDERABLETOOLBAR_H
#define REORDERABLETOOLBAR_H

#include <QToolBar>
#include <QBoxLayout>
class ReorderableToolBar : public QToolBar {
  Q_OBJECT
public:
  explicit ReorderableToolBar(const QString &title, QWidget *parent = nullptr);
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
#ifdef RUNNING_UNIT_TESTS
  QObject* mSourceObject{nullptr};
#endif
};

#endif // REORDERABLETOOLBAR_H
