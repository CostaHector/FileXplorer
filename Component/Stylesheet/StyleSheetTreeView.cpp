#include "StyleSheetTreeView.h"
#include "NotificatorMacro.h"
#include "StyleSheetGetter.h"
#include <QColorDialog>
#include <QFontDialog>

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

  mSetFontGeneral = new QAction{QIcon{":img/FONT_SIZE"}, tr("Set General Font"), this};
  mSetFontGeneral->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                      "Configure text font family/size/weight/style")
                                  .arg(mSetFontGeneral->text(), mSetFontGeneral->shortcut().toString()));
  mBatchSetColor = new QAction{QIcon{":/styles/COLOR_SELECT"}, tr("Batch Set Color"), this};
  mSeeChanges = new QAction{QIcon{":img/SAVED"}, tr("View Changes"), this};
  mClearModifiedValues = new QAction{QIcon{":/styles/CLEAR_MODIFIED_VALUES"}, tr("Clear Modified Values"), this};
  mRestoreToDefault = new QAction{QIcon{":/styles/RESTORE_TO_DEFAULT"}, tr("Restore to Default"), this};
  mRestoreToBackup = new QAction{QIcon{":/styles/RESTORE_TO_BACKUP"}, tr("Restore to Backup"), this};
  mLivePreviewSwitch = new QAction{QIcon{":/styles/INSTANT_APPLY"}, tr("Live Preview"), this};
  mLivePreviewSwitch->setToolTip(tr("See changes immediately for real-time preview when enable. Otherwise manually click `View Changes` needed. Note: this may cause screen flickering when enabled."));
  mLivePreviewSwitch->setCheckable(true);
  mLivePreviewSwitch->setChecked(false);
  QList<QAction*> exclusiveActs{mSetFontGeneral, mBatchSetColor, mSeeChanges, mClearModifiedValues, mRestoreToDefault, mRestoreToBackup, mLivePreviewSwitch};
  PushFrontExclusiveActions(exclusiveActs);

  InitTreeView();
  subscribe();
}

void StyleSheetTreeView::initExclusivePreferenceSetting() {
  CustomTreeView::m_defaultExpandAll = true;
}

void StyleSheetTreeView::subscribe() {
  connect(mSetFontGeneral, &QAction::triggered, this, &StyleSheetTreeView::onSetFontGeneral);
  connect(mBatchSetColor, &QAction::triggered, this, &StyleSheetTreeView::onBatchSetColor);
  connect(mSeeChanges, &QAction::triggered, this, &StyleSheetTreeView::onSeeChanges);
  connect(mClearModifiedValues, &QAction::triggered, this, &StyleSheetTreeView::onClearModifiedValues);
  connect(mRestoreToDefault, &QAction::triggered, this, &StyleSheetTreeView::onRestoreToDefault);
  connect(mRestoreToBackup, &QAction::triggered, this, &StyleSheetTreeView::onRestoreToBackup);
  connect(mLivePreviewSwitch, &QAction::toggled, mStyleModel, &StyleSheetTreeModel::onLivePreviewSwitchChanged);
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

std::pair<bool, QFont> GetFontWithInitial(const QFont& initialFont, QWidget* parent, const QString& title) {
  bool bOk{false};
  const QFont& newFont = QFontDialog::getFont(&bOk, initialFont, parent, title);
  return {bOk, newFont};
}

int StyleSheetTreeView::onSetFontGeneral() {
  const QFont& beforeFont = FontCfg::ReadGeneralFont();

  bool bOk{false};
  QFont newFont;
  std::tie(bOk, newFont) = GetFontWithInitial(beforeFont, nullptr, mSetFontGeneral->text());
  if (!bOk) {
    LOG_INFO_NP("Font setting cancelled", "User cancelled font selection");
    return -1;
  }
  const int attChangedCnt = mStyleModel->SetFontGeneral(newFont);
  const QString fontDetail{FontCfg::Font2String(newFont)};
  if (attChangedCnt <= 0) {
    LOG_INFO_P("Font unchanged", "No attributes modified. Current font: [%s]", qPrintable(fontDetail));
    return 0;
  }
  LOG_OK_P("Font updated", "%d attribute(s) changed. New font: [%s]", attChangedCnt, qPrintable(fontDetail));
  return attChangedCnt;
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
    LOG_INFO_NP("No row selected", "No selection to process");
    return 0;
  }
  const QVariantHash cfg = mStyleModel->CollectItemsNeedSeeChange(selectedRowsSource());
  bool bChangeExist{!cfg.isEmpty()};
  if (!bChangeExist) {
    LOG_INFO_NP("View changes", "No changes to display");
    return 0;
  }
  int settingItemsUpdatedCnt = StyleSheetGetter::GetInst().UpdateCurValue(cfg);
  if (settingItemsUpdatedCnt == 0) {
    LOG_INFO_P("Settings unchanged", "%d configuration value(s) modified, but all match current settings", cfg.size());
    return 0;
  }
  LOG_OK_P("Changes applied successfully", "%d of %d value(s) updated", settingItemsUpdatedCnt, cfg.size());
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
