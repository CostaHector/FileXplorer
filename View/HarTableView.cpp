#include "HarTableView.h"
#include "PublicVariable.h"
#include <QFileDialog>
#include <QLabel>

HarTableView::HarTableView(QWidget* parent)
    : CustomTableView{"HAR_TABLEVIEW", parent},
      mHarModel{new HarModel{this}},                      // .har file model
      mSortProxyModel{new QSortFilterProxyModel{this}} {  // for sort
  mSortProxyModel->setSourceModel(mHarModel);
  setModel(mSortProxyModel);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  InitTableView();

  mEXPORT_TO = new QAction{QIcon{":img/EXPORT_TO"}, "Export to", this};
  mEXPORT_TO->setToolTip(QString("<b>%1</b><br/> Export the selection items to local path.").arg(mEXPORT_TO->text()));

  mShowImagePreview = PreferenceSettings().value(MemoryKey::SHOW_HAR_IMAGE_PREVIEW.name, MemoryKey::SHOW_HAR_IMAGE_PREVIEW.v).toBool();
  mQUICK_PREVIEW = new QAction{"Quick Preview", this};
  mQUICK_PREVIEW->setCheckable(true);
  mQUICK_PREVIEW->setChecked(mShowImagePreview);

  mMenu = new QMenu{"Har Operation Menu", this};
  mMenu->addAction(mEXPORT_TO);
  mMenu->addAction(mQUICK_PREVIEW);
  mMenu->setToolTipsVisible(true);
  BindMenu(mMenu);

  subscribe();
  updateWindowsSize();
  setWindowTitle(GetWinTitleStr());
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
  return mHarModel->SetHarFileAbsPath(harAbsPath);
}

void HarTableView::subscribe() {
  connect(mEXPORT_TO, &QAction::triggered, this, &HarTableView::SaveSelectionFilesTo);
  connect(mQUICK_PREVIEW, &QAction::triggered, this, [this](const bool bChecked) {
    mShowImagePreview = bChecked;
    PreferenceSettings().setValue(MemoryKey::SHOW_HAR_IMAGE_PREVIEW.name, bChecked);
  });
  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &HarTableView::PreviewImage);
}

int HarTableView::SaveSelectionFilesTo() const {
  if (!selectionModel()->hasSelection()) {
    qDebug("nothing were selected. skip export.");
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
    qWarning("dst folder[%s] not existed", qPrintable(dstFolder));
    return -1;
  }
  int exportCount = mHarModel->SaveToLocal(dstFolder, selectedRows);
  qDebug("%d file(s) were exported", exportCount);
  return exportCount;
}

bool HarTableView::PreviewImage() const {
  if (!mShowImagePreview) {
    return false;
  }
  const QModelIndex srcIndex = mSortProxyModel->mapToSource(currentIndex());
  const int srcRow = srcIndex.row();
  const auto& entryItem = mHarModel->GetHarEntryItem(srcRow);
  if (!IMAGE_PREVIEW_SUPPORTED.contains(entryItem.type)) {
    qDebug("file type[%s] cannot preview", qPrintable(entryItem.type));
    return true;
  }

  QImage image;
  const QString& fileName = entryItem.name;
  if (!image.loadFromData(entryItem.content, fileName.toStdString().c_str())) {
    qWarning("image[%s] load from data failed", qPrintable(fileName));
    return false;
  }
  static QLabel* myLabel = nullptr;
  if (myLabel == nullptr) {
    myLabel = new QLabel{nullptr};
    myLabel->move(geometry().x() + width(), geometry().y());
  }
  myLabel->setPixmap(QPixmap::fromImage(image));
  myLabel->setWindowTitle(QString("%1 | %2 Byte(s)").arg(fileName).arg(entryItem.content.size()));
  myLabel->show();
  myLabel->raise();
  return true;
}

void HarTableView::updateWindowsSize() {
  if (PreferenceSettings().contains("HAR_TABLEVIEW_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("HAR_TABLEVIEW_GEOMETRY").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 1024, 768));
  }
}

void HarTableView::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("HAR_TABLEVIEW_GEOMETRY", saveGeometry());
  CustomTableView::closeEvent(event);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  auto* m_model = new HarModel;
  HarTableView win{m_model, nullptr};
  win("E:/har261/RAM261.har");
  win.show();
  return a.exec();
}
#endif
