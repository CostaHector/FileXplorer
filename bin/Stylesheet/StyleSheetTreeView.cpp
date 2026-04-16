#include "StyleSheetTreeView.h"
#include "NotificatorMacro.h"
#include "StyleSheetGetter.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "PreferenceActions.h"

StyleSheetTreeView::StyleSheetTreeView(QWidget* parent) //
  : CustomTreeView{"StyleSheetTreeView", parent} {
  setIndentation(12);
  mStyleModel = new StyleSheetTreeModel{this};

  mStyleFilterProxyModel = new TreeFilterProxyModel<StyleTreeNode>{this};
  mStyleFilterProxyModel->setSourceModel(mStyleModel);
  setModel(mStyleFilterProxyModel);
  registerProxyModel(mStyleFilterProxyModel);

  mStyleSheetEditDelegate = new StyleSheetEditDelegate{this};
  setItemDelegateForColumn(StyleItemData::EDITABLE_COLUMN, mStyleSheetEditDelegate);

  setEditTriggers(QAbstractItemView::EditKeyPressed);
  setDragDropMode(QAbstractItemView::NoDragDrop);
  setSortingEnabled(true);

  mClearModifiedValues = new QAction{tr("Clear Modified Values"), this};
  mRestoreFromBackup = new QAction{tr("Restore from Backup"), this};
  mApplyChanges = new QAction{tr("Apply Changes"), this};
  mApplyInstantly = new QAction{tr("Instant Apply"), this};
  mApplyInstantly->setToolTip("Apply changes immediately for real-time preview. Note: this may cause screen flickering.");
  mApplyInstantly->setCheckable(true);
  mApplyInstantly->setChecked(false);
  QList<QAction*> exclusiveActs{mClearModifiedValues, mRestoreFromBackup, mApplyChanges, mApplyInstantly};
  PushFrontExclusiveActions(exclusiveActs);

  InitTreeView();
  subscribe();
}

void StyleSheetTreeView::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void StyleSheetTreeView::hideEvent(QHideEvent* event) {
  g_PreferenceActions().STYLESHEET_MGR->setChecked(false);
  QWidget::hideEvent(event);
}

void StyleSheetTreeView::initExclusivePreferenceSetting() {
  CustomTreeView::m_defaultExpandAll = true;
}

void StyleSheetTreeView::subscribe() {
  connect(mClearModifiedValues, &QAction::triggered, this, &StyleSheetTreeView::onClearModifiedValues);
  connect(mRestoreFromBackup, &QAction::triggered, this, &StyleSheetTreeView::onRestoreFromBackup);
  connect(mApplyChanges, &QAction::triggered, this, &StyleSheetTreeView::onApplyChanges);
  connect(mApplyInstantly, &QAction::toggled, mStyleModel, &StyleSheetTreeModel::onInstantApplyChanged);
  connect(mStyleModel, &StyleSheetTreeModel::requestApplyChanges, this, &StyleSheetTreeView::onRequestApplyChanges);
}

void StyleSheetTreeView::setFilter(const QString& filter) {
  mStyleFilterProxyModel->setFilterString(filter);
}

void StyleSheetTreeView::onClearModifiedValues() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("No row selected", "No modified value cleared");
    return;
  }
  int rowsAffected = mStyleModel->ClearNewValues(selectedRowsSource());
  LOG_OK_P("Modified values cleared", "%d row(s) reset to default", rowsAffected);
}

void StyleSheetTreeView::onRestoreFromBackup() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("No row selected", "No changes to restore");
    return;
  }
  int rowsAffected = mStyleModel->recoverNewValuesToBackup(selectedRowsSource());
  if (rowsAffected <= 0) {
    LOG_INFO_NP("Restore from backup", "No changes to restore");
    return;
  }
  LOG_OK_P("Values restored", "%d row(s) restored from backup", rowsAffected);
}

void StyleSheetTreeView::onApplyChanges() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("No row selected", "No changes to apply");
    return;
  }
  const QVariantHash cfg = mStyleModel->ApplyNewValues(selectedRowsSource());
  bool bChangeExist{!cfg.isEmpty()};
  if (!bChangeExist) {
    LOG_INFO_NP("Apply changes", "No changes to apply");
    return;
  }
  LOG_OK_P("Changes applied", "%d value(s) updated", cfg.size());
  for (auto it = cfg.cbegin(); it != cfg.cend(); ++it) {
    Configuration().setValue(it.key(), it.value());
  }
  g_PreferenceActions().initStyleSheet(false);
}

void StyleSheetTreeView::onRequestApplyChanges(const QString& cfgKey, const QVariant& value) {
  Configuration().setValue(cfgKey, value);
  g_PreferenceActions().initStyleSheet(false);
}
