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
  QMenu* _FLOW_ORIENTATION{nullptr};
  QAction* _FLOW_ORIENTATION_LEFT_TO_RIGHT{nullptr};
  QAction* _FLOW_ORIENTATION_TOP_TO_BOTTOM{nullptr};
  EnumIntAction<QListView::Flow> mflowIntAction;

private:
  QMenu* m_menu {nullptr};

  inline bool isNameExists(const QString& name) const { return LISTS_SET.contains(name); }
  static QSet<QString> LISTS_SET;
  int mCurIconSizeIndex = 1;
  static constexpr QSize ICON_SIZE_CANDIDATES[]//
      {
          QSize(25, 16),
          QSize(40, 25),
          QSize(65, 40),
          QSize(105, 65),
          QSize(170, 105),
          QSize(275, 170),
          QSize(323, 200),
          QSize(445, 275),
          QSize(485, 300),
          QSize(566, 350),
          QSize(648, 400),
          QSize(720, 445),
          QSize(809, 500),
          QSize(970, 600),
          QSize(1165, 720),
          QSize(1885, 1165),
      };
  static constexpr int ICON_SIZE_CANDIDATES_N = sizeof(ICON_SIZE_CANDIDATES) / sizeof(*ICON_SIZE_CANDIDATES);
};

#endif  // CUSTOMLISTVIEW_H
