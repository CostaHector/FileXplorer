#ifndef CUSTOMTREEVIEW_H
#define CUSTOMTREEVIEW_H

#include <QTreeView>
#include "AddableMenu.h"

class DoubleRowHeader;
class ScrollBarPolicyMenu;
class ViewItemDelegate;
class QSortFilterProxyModel;

class CustomTreeView : public QTreeView {
  Q_OBJECT
 public:
  explicit CustomTreeView(const QString& instName, QWidget* parent = nullptr);
  ~CustomTreeView();

  const QString& GetName() const { return m_name; }

  void PushFrontExclusiveActions(const QList<QAction*>& acts);
  void InitTreeView();

  int rowHeight() const;
  bool setRowHeight(int newRowHeight);

 signals:
  void searchSqlStatementChanged(const QString& sqlStatement);

 protected:
  void contextMenuEvent(QContextMenuEvent* event) override;
  virtual void initExclusivePreferenceSetting() {}
  bool m_defaultExpandAll{false};

  void registerProxyModel(QSortFilterProxyModel* proxyModelInDerived);
  QModelIndexList selectedRowsSource() const;

 private:
  void AddItselfAction2Menu();
  void SubscribeHeaderActions();
  bool ShowOrHideColumnCore();

  const QString m_name;
  const QString m_showHorizontalHeaderKey;
  const QString m_autoScrollKey;
  const QString m_alternatingRowColorsKey;

  QAction *mExpandAll{nullptr}, *mCollapseAll{nullptr};
  QAction* mAnimatedEnableAct{nullptr};
  QAction* mRootDecorationEnabled{nullptr};

  QAction* _SHOW_ALL_HORIZONTAL_COLUMNS{nullptr};
  QAction* _SHOW_HORIZONTAL_HEADER{nullptr};
  QAction* _AUTO_SCROLL{nullptr};
  QAction* _ALTERNATING_ROW_COLORS{nullptr};
  ScrollBarPolicyMenu *m_horScrollBarPolicyMenu{nullptr}, *m_verScrollBarPolicyMenu{nullptr};

  DoubleRowHeader* m_horHeader{nullptr};
  AddableMenu* m_menu{nullptr};
  ViewItemDelegate* m_itemDelegate{nullptr};

  QSortFilterProxyModel* _proxyModel{nullptr};

  inline bool isNameExists(const QString& name) const { return mTreeInstSet.contains(name); }
  static QSet<QString> mTreeInstSet;
};

#endif  // CUSTOMTREEVIEW_H
