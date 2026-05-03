#include "HarTableView.h"
#include "CompoVisKey.h"
#include "Configuration.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "PathTool.h"

#include <QFileDialog>
#include <QLabel>

HarTableView::HarTableView(QWidget* parent)
  : CustomTableView{"HAR_TABLEVIEW", parent} {
  mHarModel = new (std::nothrow) HarModel{this}; // .har file model
  CHECK_NULLPTR_RETURN_VOID(mHarModel);

  mSortFilterProxy = new (std::nothrow) QSortFilterProxyModel{this}; // for sort
  CHECK_NULLPTR_RETURN_VOID(mSortFilterProxy);

  mSortFilterProxy->setSourceModel(mHarModel);
  setModel(mSortFilterProxy);
  registerProxyModel(mSortFilterProxy);

  setEditTriggers(QAbstractItemView::NoEditTriggers);
  InitTableView();

  mEXPORT_TO = new (std::nothrow) QAction{QIcon{":img/EXPORT_TO"}, tr("Export Selections to"), this};
  CHECK_NULLPTR_RETURN_VOID(mEXPORT_TO)
  mEXPORT_TO->setToolTip(QString("<b>%1</b><br/> Export the selection items to local path.").arg(mEXPORT_TO->text()));

  PushFrontExclusiveActions({mEXPORT_TO});

  subscribe();
}

QString HarTableView::GetWinTitleStr(const QString& harFile) const {
  QString title{"Har Viewer"};
  if (!harFile.isEmpty()) {
    title += " | ";
    title += harFile;
  }
  return title;
}

int HarTableView::operator()(const QString& harAbsPath) {
  mHarAbsPath = harAbsPath;
  emit windowTitleChanged(GetWinTitleStr(mHarAbsPath));
  return mHarModel->setRootPath(harAbsPath);
}

void HarTableView::setFilter(const QString& filter) {
  mSortFilterProxy->setFilterFixedString(filter);
}

void HarTableView::subscribe() {
  connect(mEXPORT_TO, &QAction::triggered, this, &HarTableView::SaveSelectionFilesTo);
  connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &HarTableView::PreviewImage);
}

bool HarTableView::PreviewImage(const QModelIndex& current) {
  if (!current.isValid()) {
    return false;
  }
  const QModelIndex srcIndex = mSortFilterProxy->mapToSource(current);
  if (!srcIndex.isValid()) {
    return false;
  }

  static const QSet<QString> IMAGE_PREVIEW_SUPPORTED{"image/jpg", "image/svg", "image/png", ".jpeg", ".jpg", ".png", ".webp", ".gif", "tif", "tiff"};
  const int srcRow = srcIndex.row();
  const HAR_FILE_ITEM& entryItem = mHarModel->GetHarEntryItem(srcRow);
  if (!IMAGE_PREVIEW_SUPPORTED.contains(entryItem.type)) {
    return false;
  }
  QString noDotFormatStr = PathTool::GetFormatInHar(entryItem.type);
  emit pixmapByteArrayChanged(entryItem.content, noDotFormatStr);
  return true;
}

int HarTableView::SaveSelectionFilesTo() const {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("[skip] Nothing were selected to export", "return");
    return 0;
  }

  const QString dstFolder{QFileDialog::getExistingDirectory(nullptr, "Export selection(s) to", mHarAbsPath)};
  if (!QFileInfo{dstFolder}.isDir()) {
    LOG_W("dst folder[%s] not existed", qPrintable(dstFolder));
    return -1;
  }

  const QList<int> selectedRows{selectedRowsSourceInt()};
  const int exportCount = mHarModel->SaveToLocal(dstFolder, selectedRows);
  if (exportCount != selectedRows.size()) {
    LOG_WARN_P("[Partially failed] Item(s) exported", "Only %d/%d item(s) to path:\n%s", exportCount, selectedRows.size(), qPrintable(dstFolder));
    return exportCount;
  }
  LOG_OK_P("[Ok] Item(s) exported", "all %d/%d item(s) to path:\n%s", exportCount, selectedRows.size(), qPrintable(dstFolder));
  return exportCount;
}
