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
  void setFilter(const QString& filter);
  QString curFilter() const;
signals:
  void reqSeeChanges();

private:
  void initExclusivePreferenceSetting() override;

  void subscribe();
  int onClearModifiedValues();
  int onRestoreToDefault();
  int onRestoreToBackup();
  int onSetFontGeneral();
  int onBatchSetColor();
  int onSeeChanges();

  void onRequestSeeChanges(const QString& cfgKey, const QVariant& value);

  QAction* mSetFontGeneral{nullptr};
  QAction* mBatchSetColor{nullptr};
  QAction* mSeeChanges{nullptr};
  QAction* mClearModifiedValues{nullptr};
  QAction* mRestoreToDefault{nullptr};
  QAction* mRestoreToBackup{nullptr};
  QAction* mLivePreviewSwitch{nullptr};

  StyleSheetTreeModel* mStyleModel{nullptr};
  TreeFilterProxyModel<StyleTreeNode>* mStyleFilterProxyModel{nullptr};
  StyleSheetEditDelegate* mStyleSheetEditDelegate{nullptr};
};

#endif // STYLESHEETTREEVIEW_H
