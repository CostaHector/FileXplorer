#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QAction>
#include <QListView>
#include <QMenu>
#include "EnumIntAction.h"

extern template struct EnumIntAction<QListView::Flow>;

class CustomListView : public QListView {
  Q_OBJECT
public:
  explicit CustomListView(const QString& name, QWidget* parent = nullptr);
  ~CustomListView();
  void contextMenuEvent(QContextMenuEvent* event) override;
  void wheelEvent(QWheelEvent *event) override;

  void BindMenu(QMenu* menu);
  void onOrientationChange(const QAction* pAct);

  void InitListView();

signals:
  void onIconSizeChanged(QSize newSize);

protected:
  QString m_name;
  QMenu* _FLOW_ORIENTATION_MENU{nullptr};
  QAction* _FLOW_ORIENTATION_LEFT_TO_RIGHT{nullptr};
  QAction* _FLOW_ORIENTATION_TOP_TO_BOTTOM{nullptr};
  EnumIntAction<QListView::Flow> mflowIntAction;

private:
  QMenu* m_menu {nullptr};

  inline bool isNameExists(const QString& name) const { return LISTS_SET.contains(name); }
  static QSet<QString> LISTS_SET;
  int mCurIconSizeIndex = 1;
};

#endif  // CUSTOMLISTVIEW_H
