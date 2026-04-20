#include "StyleSheetTreeView.h"
#include "NotificatorMacro.h"
#include "StyleSheetGetter.h"
#include <QColorDialog>

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

  mBatchSetColor = new QAction{QIcon{":/styles/COLOR_SELECT"}, tr("Batch Set Color"), this};
  mSeeChanges = new QAction{QIcon{":img/SAVED"}, tr("See Changes"), this};
  mClearModifiedValues = new QAction{QIcon{":/styles/CLEAR_MODIFIED_VALUES"}, tr("Clear Modified Values"), this};
  mRestoreToDefault = new QAction{QIcon{":/styles/RESTORE_TO_DEFAULT"}, tr("Restore to Default"), this};
  mRestoreToBackup = new QAction{QIcon{":/styles/RESTORE_TO_BACKUP"}, tr("Restore to Backup"), this};
  mSeeChangesInstantly = new QAction{QIcon{":/styles/INSTANT_APPLY"}, tr("Instant See"), this};
  mSeeChangesInstantly->setToolTip("See changes immediately for real-time preview. Note: this may cause screen flickering.");
  mSeeChangesInstantly->setCheckable(true);
  mSeeChangesInstantly->setChecked(false);
  QList<QAction*> exclusiveActs{mBatchSetColor, mSeeChanges, mClearModifiedValues, mRestoreToDefault, mRestoreToBackup, mSeeChangesInstantly};
  PushFrontExclusiveActions(exclusiveActs);

  InitTreeView();
  subscribe();
}

void StyleSheetTreeView::initExclusivePreferenceSetting() {
  CustomTreeView::m_defaultExpandAll = true;
}

void StyleSheetTreeView::subscribe() {
  connect(mBatchSetColor, &QAction::triggered, this, &StyleSheetTreeView::onBatchSetColor);
  connect(mSeeChanges, &QAction::triggered, this, &StyleSheetTreeView::onSeeChanges);
  connect(mClearModifiedValues, &QAction::triggered, this, &StyleSheetTreeView::onClearModifiedValues);
  connect(mRestoreToDefault, &QAction::triggered, this, &StyleSheetTreeView::onRestoreToDefault);
  connect(mRestoreToBackup, &QAction::triggered, this, &StyleSheetTreeView::onRestoreToBackup);
  connect(mSeeChangesInstantly, &QAction::toggled, mStyleModel, &StyleSheetTreeModel::onInstantSeeSwitchChanged);
  connect(mStyleModel, &StyleSheetTreeModel::requestSeeChanges, this, &StyleSheetTreeView::onRequestSeeChanges);
}

void StyleSheetTreeView::setFilter(const QString& filter) {
  mStyleFilterProxyModel->setFilterString(filter);
}

QString StyleSheetTreeView::curFilter() const {
  return mStyleFilterProxyModel->curFilter();
}

int StyleSheetTreeView::onClearModifiedValues() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("No row selected", "No modified value cleared");
    return 0;
  }
  int rowsAffected = mStyleModel->ClearNewValues(selectedRowsSource());
  LOG_OK_P("Modified values cleared", "%d row(s) reset to default", rowsAffected);
  return rowsAffected;
}

int StyleSheetTreeView::onRestoreToDefault() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("No row selected", "No changes to restore");
    return 0;
  }
  int rowsAffected = mStyleModel->RecoverNewValuesToDefault(selectedRowsSource());
  if (rowsAffected <= 0) {
    LOG_INFO_NP("Restore to default", "No changes to restore");
    return rowsAffected;
  }
  LOG_OK_P("Values restored", "%d row(s) restored to default", rowsAffected);
  return rowsAffected;
}

int StyleSheetTreeView::onRestoreToBackup() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("No row selected", "No changes to restore");
    return 0;
  }
  int rowsAffected = mStyleModel->RecoverNewValuesToBackup(selectedRowsSource());
  if (rowsAffected <= 0) {
    LOG_INFO_NP("Restore to backup", "No changes to restore");
    return rowsAffected;
  }
  LOG_OK_P("Values restored", "%d row(s) restored to backup", rowsAffected);
  return rowsAffected;
}

int StyleSheetTreeView::onBatchSetColor() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("No row selected", "No color value set");
    return 0;
  }
  const QModelIndexList indexes = selectedRowsSource();
  const QString& colorSelectTitle{QString{"Select color for 1 rows"}.arg(indexes.size())};
  QColor newColor = QColorDialog::getColor(Qt::GlobalColor::white, this, colorSelectTitle);
  if (!newColor.isValid()) {
    return -1;
  }
  const QString& newColorStr{newColor.name(QColor::HexArgb)};
  const int rowsAffected = mStyleModel->SetNewColors(indexes, newColorStr);
  LOG_OK_P("Set New Color", "%d/%d rows changed to %s", rowsAffected, indexes.size(), qPrintable(newColorStr));
  return rowsAffected;
}

int StyleSheetTreeView::onSeeChanges() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("No row selected", "No changes to apply");
    return 0;
  }
  const QVariantHash cfg = mStyleModel->CollectItemsNeedApplyChange(selectedRowsSource());
  bool bChangeExist{!cfg.isEmpty()};
  if (!bChangeExist) {
    LOG_INFO_NP("See changes", "No changes to apply");
    return 0;
  }
  int settingItemsUpdatedCnt = StyleSheetGetter::GetInst().UpdateCurValue(cfg);
  if (settingItemsUpdatedCnt == 0) {
    LOG_INFO_P("No changes needed", "%d configuration value(s) modified but all match existing settings", cfg.size());
    return 0;
  }
  LOG_OK_P("Changes shown successfully", "%d of %d value(s) updated", settingItemsUpdatedCnt, cfg.size());
  emit reqSeeChanges();
  return settingItemsUpdatedCnt;
}

void StyleSheetTreeView::onRequestSeeChanges(const QString& cfgKey, const QVariant& value) {
  int settingItemsUpdatedCnt = StyleSheetGetter::GetInst().UpdateCurValue({{cfgKey, value}});
  if (settingItemsUpdatedCnt == 0) {
    return;
  }
  emit reqSeeChanges();
}
