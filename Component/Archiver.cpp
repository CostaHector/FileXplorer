#include "Archiver.h"
#include "public/PublicVariable.h"
#include "Tools/ArchiveFiles.h"
#include "Model/QAbstractTableModelPub.h"
#include "View/CustomTableView.h"
#include "public/DisplayEnhancement.h"
#include "public/StyleSheet.h"

#include <QAbstractTableModel>
#include <QLabel>
#include <QSplitter>

class ArchiverModel : public QAbstractTableModelPub {
 public:
  explicit ArchiverModel(QObject* parent = nullptr) : QAbstractTableModelPub{parent} {}
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return m_paf != nullptr ? m_paf->size() : 0; }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return ARCHIVE_HORIZONTAL_HEADER.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    if (m_paf == nullptr or not index.isValid()) {
      return QVariant();
    }
    switch (role) {
      case Qt::DisplayRole: {
        switch (index.column()) {
          case 0:
            return m_paf->key(index.row());
          case 1:
            return FILE_PROPERTY_DSP::sizeToHumanReadFriendly(m_paf->beforeSize(index.row()));
          case 2:
            return FILE_PROPERTY_DSP::sizeToHumanReadFriendly(m_paf->afterSize(index.row()));
          default:
            return QVariant();
        }
      }
      default:
        return QVariant();
    }
    return QVariant();
  }
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Horizontal) {
        return ARCHIVE_HORIZONTAL_HEADER[section];
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  void setRootPath(const ArchiveFiles* p_af) {
    int beforeRow = rowCount();
    int afterRow = p_af != nullptr ? p_af->size() : 0;
    qDebug("setRootPath. RowCountChanged: %d->%d", beforeRow, afterRow);

    RowsCountBeginChange(beforeRow, afterRow);
    m_paf = p_af;
    RowsCountEndChange();
  }

 private:
  const ArchiveFiles* m_paf{nullptr};
  static const QStringList ARCHIVE_HORIZONTAL_HEADER;
};
const QStringList ArchiverModel::ARCHIVE_HORIZONTAL_HEADER{"Name", "Compressed(B)", "Original(B)"};

Archiver::Archiver(QWidget* parent)
    : QMainWindow{parent},
      m_splitter{new QSplitter{this}},
      m_itemsTable{new CustomTableView{"ArchiverItemsTable", this}},
      m_archiverModel{new ArchiverModel{this}},
      m_thumbnailViewer{new QLabel{"Preview here", this}},
      m_af{"", ArchiveFiles::ONLY_IMAGE} {
  m_itemsTable->setModel(m_archiverModel);

  m_splitter->addWidget(m_itemsTable);
  m_splitter->addWidget(m_thumbnailViewer);
  setCentralWidget(m_splitter);

  subscribe();

  m_itemsTable->InitTableView();

  m_archiverModel->setRootPath(&m_af);

  UpdateWindowsSize();
  setWindowTitle("QZ Archive");
  setWindowIcon(QIcon(":img/COMPRESS_ITEM"));
}

void Archiver::ChangeWindowTitle(const QString& name, const int& Bytes) {
  setWindowTitle(QString("QZ Archive | %1 | %2").arg(name).arg(FILE_PROPERTY_DSP::sizeToHumanReadFriendly(Bytes)));
}

void Archiver::UpdateWindowsSize() {
  if (PreferenceSettings().contains("ArchiverGeometry")) {
    restoreGeometry(PreferenceSettings().value("ArchiverGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  m_splitter->restoreState(PreferenceSettings().value("ArchiverSplitterState", QByteArray()).toByteArray());
}

void Archiver::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void Archiver::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("ArchiverGeometry", saveGeometry());
  qDebug("Archiver geometry was resize to (%d, %d, %d, %d)", geometry().x(), geometry().y(), geometry().width(), geometry().height());
  PreferenceSettings().setValue("ArchiverSplitterState", m_splitter->saveState());
  QMainWindow::closeEvent(event);
}

void Archiver::subscribe() {
  connect(m_itemsTable->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &Archiver::onNewRow);
}

bool Archiver::onNewRow(const QModelIndex& current, const QModelIndex& /* previous */) {
  if (not current.isValid()) {
    m_thumbnailViewer->clear();
    return false;
  }
  int newRow = current.row();
  const QString& name = m_af.key(newRow);
  const QByteArray& img = m_af.value(newRow);

  QPixmap pm;
  pm.loadFromData(img);
  m_thumbnailViewer->setPixmap(pm.scaledToWidth(m_thumbnailViewer->width()));

  ChangeWindowTitle(name, img.size());
  return true;
}

bool Archiver::operator()(const QString& qzPath) {
  if (not ArchiveFiles::isQZFile(qzPath)) {
    qWarning("Path[%s] is not a qz file", qPrintable(qzPath));
    setWindowTitle(QString("ArchiveFilesPreview | [%1] not a qz file").arg(qzPath));
    return false;
  }

  decltype(m_af) temp{qzPath, ArchiveFiles::ONLY_IMAGE};
  int beforeRowCount = m_af.size();
  int afterRowCount = temp.size();
  m_archiverModel->RowsCountBeginChange(beforeRowCount, afterRowCount);
  m_af.swap(temp);
  m_archiverModel->RowsCountEndChange();
  return true;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  Archiver afPreview;
  afPreview.show();
  afPreview.operator()("E:/Brazzers/Page400/Page400/Page400.qz");
  return a.exec();
}
#endif
