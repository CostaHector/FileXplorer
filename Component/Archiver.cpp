#include "Archiver.h"
#include "MemoryKey.h"
#include "DataFormatter.h"
#include "StyleSheet.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include <QToolBar>

Archiver::Archiver(QWidget* parent) : QMainWindow{parent} {
  m_splitter = new (std::nothrow) QSplitter{this};
  CHECK_NULLPTR_RETURN_VOID(m_splitter);
  m_itemsTable = new (std::nothrow) CustomTableView{"ArchiverItemsTable", this};
  CHECK_NULLPTR_RETURN_VOID(m_itemsTable);
  m_archiverModel = new (std::nothrow) ArchiverModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_archiverModel);
  m_thumbnailViewer = new (std::nothrow) ThumbnailImageViewer{"AchiveImagePreview", this};
  CHECK_NULLPTR_RETURN_VOID(m_thumbnailViewer);

  m_itemsTable->setModel(m_archiverModel);
  m_splitter->addWidget(m_itemsTable);
  m_splitter->addWidget(m_thumbnailViewer);
  setCentralWidget(m_splitter);

  m_ImageSizeHint = new (std::nothrow) QLabel{this};
  CHECK_NULLPTR_RETURN_VOID(m_ImageSizeHint);

  m_ImageSizeScale = new (std::nothrow) QSlider{Qt::Orientation::Horizontal, this};
  CHECK_NULLPTR_RETURN_VOID(m_ImageSizeScale);
  m_ImageSizeScale->setRange(0, IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);
  m_ImageSizeScale->setValue(m_thumbnailViewer->GetCurImageSizeScale());

  QToolBar* tb = new (std::nothrow) QToolBar("Image adjuster toolbar", this);
  CHECK_NULLPTR_RETURN_VOID(tb);
  tb->addWidget(m_ImageSizeScale);
  tb->addWidget(m_ImageSizeHint);
  addToolBar(Qt::ToolBarArea::TopToolBarArea, tb);

  m_itemsTable->InitTableView();
  UpdateWindowsSize();
  setWindowTitle("QZ Archive");
  setWindowIcon(QIcon(":img/COMPRESS_ITEM"));

  subscribe();
}

bool Archiver::operator()(const QString& qzPath) {
  if (!ArchiveFilesReader::isQZFile(qzPath)) {
    LOG_W("Path[%s] is not a qz file", qPrintable(qzPath));
    setWindowTitle(QString("ArchiveFilesPreview | [%1] not a qz file").arg(qzPath));
    return false;
  }

  m_archiverModel->setRootPath(qzPath);
  return true;
}

void Archiver::ChangeWindowTitle(const QString& name, const int& Bytes) {
  setWindowTitle(QString("QZ Archive | %1 | %2").arg(name).arg(DataFormatter::formatFileSizeGMKB(Bytes)));
}

void Archiver::UpdateWindowsSize() {
  m_splitter->restoreState(Configuration().value("ArchiverSplitterState", QByteArray()).toByteArray());
}

void Archiver::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void Archiver::closeEvent(QCloseEvent* event) {
  Configuration().setValue("ArchiverSplitterState", m_splitter->saveState());
  QMainWindow::closeEvent(event);
}

void Archiver::subscribe() {
  connect(m_itemsTable->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &Archiver::onSelectNewItemRow);
  connect(m_ImageSizeScale, &QSlider::valueChanged, this, &Archiver::onSilderChangedUpdateImageScaledIndex);
  connect(m_thumbnailViewer, &ThumbnailImageViewer::onImageScaledIndexChanged, this, &Archiver::setSliderValueAndLabelDisplayText);
}

bool Archiver::onSelectNewItemRow(const QModelIndex& current, const QModelIndex& /* previous */) {
  if (!current.isValid()) {
    m_thumbnailViewer->clearPixmap();
    return false;
  }
  int newRow = current.row();
  const QString& name = m_archiverModel->GetRelativeName(newRow);
  const QByteArray& dataByteArray = m_archiverModel->GetByteArrayData(newRow);
  ChangeWindowTitle(name, dataByteArray.size());
  if (TYPE_FILTER::IMAGE_TYPE_SET.contains(PathTool::GetAsteriskDotFileExtension(name))) {
    m_thumbnailViewer->setPixmapByByteArrayData(dataByteArray); // only update for images
  }
  return true;
}

void Archiver::setSliderValueAndLabelDisplayText(int scaleIndex) {
  m_ImageSizeScale->setValue(scaleIndex);
  m_ImageSizeHint->setText(IMAGE_SIZE::HumanReadFriendlySize(scaleIndex));
}

void Archiver::onSilderChangedUpdateImageScaledIndex(int scaleIndex) {
  if (!m_thumbnailViewer->setIconSizeScaledIndex(scaleIndex)) { // to label updata size index
    LOG_D("Invalid image scaled index[%d]", scaleIndex);
    return;
  }
  setSliderValueAndLabelDisplayText(scaleIndex); // to qslider and label display
  m_thumbnailViewer->refreshPixmapSize(); // to label let it refresh
}
