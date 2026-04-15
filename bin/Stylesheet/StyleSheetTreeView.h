#ifndef STYLESHEETTREEVIEW_H
#define STYLESHEETTREEVIEW_H

#include "CustomTreeView.h"
#include "StyleSheetTreeModel.h"
#include "StyleSheetEditDelegate.h"

class StyleSheetTreeView : public CustomTreeView {
  Q_OBJECT
public:
  explicit StyleSheetTreeView(QWidget* parent = nullptr);
  ~StyleSheetTreeView() = default;

protected:
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;

private:
  void initExclusivePreferenceSetting() override;

  void subscribe();
  void onClearModifiedValues();
  void onRestoreFromBackup();
  void onApplyChanges();

  void onRequestApplyChanges(const QString& cfgKey, const QVariant& value);

  QAction* mClearModifiedValues{nullptr};
  QAction* mRestoreFromBackup{nullptr};
  QAction* mApplyChanges{nullptr};
  QAction* mApplyInstantly{nullptr};

  StyleSheetTreeModel* mStyleModel{nullptr};
  StyleSheetEditDelegate* mStyleSheetEditDelegate{nullptr};
};

#endif // STYLESHEETTREEVIEW_H
