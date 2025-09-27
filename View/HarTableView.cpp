#include "HarTableView.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "NotificatorMacro.h"
#include <QFileDialog>
#include <QLabel>

HarTableView::HarTableView(QWidget* parent)
  : CustomTableView{"HAR_TABLEVIEW", parent} {
  mHarModel = new (std::nothrow) HarModel{this};                      // .har file model
  CHECK_NULLPTR_RETURN_VOID(mHarModel)
  mSortProxyModel = new (std::nothrow) QSortFilterProxyModel{this};  // for sort
  CHECK_NULLPTR_RETURN_VOID(mSortProxyModel)
  mSortProxyModel->setSourceModel(mHarModel);
  setModel(mSortProxyModel);

  setEditTriggers(QAbstractItemView::NoEditTriggers);
  InitTableView();

  mEXPORT_TO = new (std::nothrow) QAction{QIcon{":img/EXPORT_TO"}, "Export Selections to", this};
  CHECK_NULLPTR_RETURN_VOID(mEXPORT_TO)
  mEXPORT_TO->setToolTip(QString("<b>%1</b><br/> Export the selection items to local path.").arg(mEXPORT_TO->text()));

  mShowImagePreview = Configuration().value(MemoryKey::SHOW_HAR_IMAGE_PREVIEW.name, MemoryKey::SHOW_HAR_IMAGE_PREVIEW.v).toBool();
  mQUICK_PREVIEW = new (std::nothrow) QAction{QIcon{":img/IMAGE"}, "Enable Images Preview", this};
  CHECK_NULLPTR_RETURN_VOID(mQUICK_PREVIEW)
  mQUICK_PREVIEW->setCheckable(true);
  mQUICK_PREVIEW->setChecked(mShowImagePreview);

  mMenu = new (std::nothrow) QMenu{"Har Operation Menu", this};
  CHECK_NULLPTR_RETURN_VOID(mMenu)
  mMenu->addAction(mEXPORT_TO);
  mMenu->addAction(mQUICK_PREVIEW);
  mMenu->setToolTipsVisible(true);
  BindMenu(mMenu);

  subscribe();
  updateWindowsSize();
  setWindowTitle(GetWinTitleStr());
  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose, true);
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
  setWindowTitle(GetWinTitleStr(mHarAbsPath));
  return mHarModel->setRootPath(harAbsPath);
}

void HarTableView::subscribe() {
  connect(mEXPORT_TO, &QAction::triggered, this, &HarTableView::SaveSelectionFilesTo);
  connect(mQUICK_PREVIEW, &QAction::triggered, this, [this](const bool bChecked) { mShowImagePreview = bChecked; });
  connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &HarTableView::PreviewImage);
}

int HarTableView::SaveSelectionFilesTo() const {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("[skip] Nothing were selected to export", "return");
    return 0;
  }
  QList<int> selectedRows;
  for (const auto& rowIndex : selectionModel()->selectedRows()) {
    const auto& srcIndex = mSortProxyModel->mapToSource(rowIndex);
    selectedRows.append(srcIndex.row());
  }
  const QFileInfo harFi{mHarAbsPath};
  const QString& dstFolder = QFileDialog::getExistingDirectory(nullptr, "Export selection(s) to", harFi.absolutePath());
  if (!QFileInfo{dstFolder}.isDir()) {
    LOG_W("dst folder[%s] not existed", qPrintable(dstFolder));
    return -1;
  }
  int exportCount = mHarModel->SaveToLocal(dstFolder, selectedRows);
  if (exportCount != selectedRows.size()) {
    LOG_WARN_P("[Partially failed] Item(s) exported", "Only %d/%d item(s) to path:\n%s", exportCount, selectedRows.size(), qPrintable(dstFolder));
    return exportCount;
  }
  LOG_OK_P("[Ok] Item(s) exported", "all %d/%d item(s) to path:\n%s", exportCount, selectedRows.size(), qPrintable(dstFolder));
  return exportCount;
}

bool HarTableView::PreviewImage(const QModelIndex &current, const QModelIndex &/*previous*/) {
  if (!mShowImagePreview) {
    return false;
  }
  const QModelIndex srcIndex = mSortProxyModel->mapToSource(current);
  const int srcRow = srcIndex.row();
  const auto& entryItem = mHarModel->GetHarEntryItem(srcRow);
  static const QSet<QString> IMAGE_PREVIEW_SUPPORTED {".jpeg", ".jpg", ".png", ".webp", ".gif", "tif", "tiff"};
  if (!IMAGE_PREVIEW_SUPPORTED.contains(entryItem.type)) {
    LOG_D("file type[%s] cannot preview", qPrintable(entryItem.type));
    return true;
  }

  QImage image;
  const QString& fileName = entryItem.name;
  if (!image.loadFromData(entryItem.content, fileName.toStdString().c_str())) {
    LOG_W("image[%s] load from data failed", qPrintable(fileName));
    return false;
  }
  if (mPreviewLabel == nullptr) {
    mPreviewLabel = new QLabel{this};
    mPreviewLabel->setWindowFlags(Qt::Window);
    mPreviewLabel->setAttribute(Qt::WA_DeleteOnClose);
    mPreviewLabel->move(geometry().x() + width(), geometry().y());
    mPreviewLabel->setMinimumSize(600, 400);
  }
  mPreviewLabel->setPixmap(QPixmap::fromImage(image));
  mPreviewLabel->setWindowTitle(QString("%1 | %2 Byte(s)").arg(fileName).arg(entryItem.content.size()));
  mPreviewLabel->show();
  mPreviewLabel->raise();
  return true;
}

void HarTableView::updateWindowsSize() {
  if (Configuration().contains("HAR_TABLEVIEW_GEOMETRY")) {
    restoreGeometry(Configuration().value("HAR_TABLEVIEW_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

void HarTableView::showEvent(QShowEvent *event) {
  CustomTableView::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void HarTableView::closeEvent(QCloseEvent* event) {
  Configuration().setValue("HAR_TABLEVIEW_GEOMETRY", saveGeometry());
  Configuration().setValue(MemoryKey::SHOW_HAR_IMAGE_PREVIEW.name, mShowImagePreview);
  CustomTableView::closeEvent(event);
}
