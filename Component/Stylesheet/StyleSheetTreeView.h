#ifndef STYLESHEETTREEVIEW_H
#define STYLESHEETTREEVIEW_H

#include "CustomTreeView.h"
#include "StyleSheetTreeModel.h"
#include "TreeFilterProxyModel.h"
#include "StyleSheetEditDelegate.h"

extern template class TreeFilterProxyModel<StyleTreeNode>;

class StyleSheetTreeView : public CustomTreeView {
  Q_OBJECT
public:
  explicit StyleSheetTreeView(QWidget* parent = nullptr);
  ~StyleSheetTreeView() = default;
  void setFilter(const QString& filter);
  QString curFilter() const;

private:
  void initExclusivePreferenceSetting() override;

  void subscribe();
  int onClearModifiedValues();
  int onRestoreToDefault();
  int onRestoreToBackup();
  int onBatchSetColor();
  int onApplyChanges();

  void onRequestApplyChanges(const QString& cfgKey, const QVariant& value);

  QAction* mBatchSetColor{nullptr};
  QAction* mApplyChanges{nullptr};
  QAction* mClearModifiedValues{nullptr};
  QAction* mRestoreToDefault{nullptr};
  QAction* mRestoreToBackup{nullptr};
  QAction* mApplyInstantly{nullptr};

  StyleSheetTreeModel* mStyleModel{nullptr};
  TreeFilterProxyModel<StyleTreeNode>* mStyleFilterProxyModel{nullptr};
  StyleSheetEditDelegate* mStyleSheetEditDelegate{nullptr};
};

#endif // STYLESHEETTREEVIEW_H
