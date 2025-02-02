#include "HarTableView.h"
#include <QFileDialog>
#include <QLabel>

HarTableView::HarTableView(QWidget* parent)
    : CustomTableView{"HAR_TABLEVIEW", parent},
      mHarModel{new HarModel{this}},                      // .har file model
      mSortProxyModel{new QSortFilterProxyModel{this}} {  // for sort
  mSortProxyModel->setSourceModel(mHarModel);
  setModel(mSortProxyModel);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setWindowTitle(GetWinTitleStr());
  InitTableView();
  setSortingEnabled(true);

  mEXPORT_TO = new QAction{"Export to", this};
  mMenu = new QMenu{"Har Operation Menu", this};
  mMenu->addAction(mEXPORT_TO);
  BindMenu(mMenu);

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
  setWindowTitle(GetWinTitleStr(mHarAbsPath));
  return mHarModel->SetHarFileAbsPath(harAbsPath);
}

void HarTableView::subscribe() {
  connect(mEXPORT_TO, &QAction::triggered, this, &HarTableView::SaveSelectionFilesTo);
  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &HarTableView::PreviewImage);
}

int HarTableView::SaveSelectionFilesTo() {
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
  const QString& dstFolder = QFileDialog::getExistingDirectory(this, "Export selection(s) to", harFi.absolutePath());
  if (!QFileInfo{dstFolder}.isDir()) {
    qWarning("dst folder[%s] not existed", qPrintable(dstFolder));
    return -1;
  }
  int exportCount = mHarModel->SaveToLocal(dstFolder, selectedRows);
  qDebug("%d file(s) exported", exportCount);
  return exportCount;
}

bool HarTableView::PreviewImage() const {
  const QModelIndex srcIndex = mSortProxyModel->mapToSource(currentIndex());
  const int srcRow = srcIndex.row();
  QString fileName;
  const auto& imgBytes = mHarModel->GetImageByteArray(srcRow, fileName);
  QImage image;
  if (image.loadFromData(imgBytes, fileName.toStdString().c_str())) {
    static QLabel* myLabel = nullptr;
    if (myLabel == nullptr) {
      myLabel = new QLabel{nullptr};
    }
    myLabel->setPixmap(QPixmap::fromImage(image));
    myLabel->setWindowTitle(fileName);
    myLabel->show();
    myLabel->raise();
  }
  return true;
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
