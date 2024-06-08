#include "Archiver.h"
#include "PublicVariable.h"
#include "Tools/ArchiveFiles.h"
#include "View/CustomListView.h"
#include "public/DisplayEnhancement.h"

#include <QLabel>
#include <QSplitter>
#include <QAbstractTableModel>

class ArchiverModel : public QAbstractTableModel {
 public:
  explicit ArchiverModel(QObject* parent = nullptr) : QAbstractTableModel{parent} {}
  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override { return m_paf != nullptr ? m_paf->size() : 0; }
  auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override { return m_paf != nullptr ? 1 : 0; }
  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override {
    if (m_paf == nullptr or not index.isValid()) {
      return QVariant();
    }
    switch (role) {
      case Qt::DisplayRole:
        return m_paf->key(index.row());
      default:
        return QVariant();
    }
  }

  auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant override {
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
    qDebug(" setRootPath %d->%d", beforeRow, afterRow);
    // equal
    if (beforeRow == afterRow) {
      return;
    }
    // isInc
    if (beforeRow < afterRow) {
      beginInsertRows(QModelIndex(), 0, afterRow - 1);
      m_paf = p_af;
      endInsertRows();
      return;
    }
    // isDec
    beginRemoveRows(QModelIndex(), afterRow, beforeRow - 1);
    m_paf = p_af;
    endRemoveRows();
  }

 private:
  const ArchiveFiles* m_paf{nullptr};
  static const QStringList ARCHIVE_HORIZONTAL_HEADER;
};
const QStringList ArchiverModel::ARCHIVE_HORIZONTAL_HEADER{"Name", "Compressed", "Original", "Type"};

Archiver::Archiver(QWidget* parent)
    : QMainWindow{parent},
      m_splitter{new QSplitter{this}},
      m_itemsList{new CustomListView{"ArchiverItemsList", this}},
      m_archiverModel{new ArchiverModel{this}},
      m_thumbnailViewer{new QLabel{"Preview here", this}},
      m_af{"", ArchiveFiles::ONLY_IMAGE},
      m_tempAf{"", ArchiveFiles::ONLY_IMAGE} {
  m_itemsList->setModel(m_archiverModel);

  m_splitter->addWidget(m_itemsList);
  m_splitter->addWidget(m_thumbnailViewer);
  setCentralWidget(m_splitter);

  subscribe();

  UpdateWindowsSize();
  setWindowTitle("QZ Archive");
  setWindowIcon(QIcon(":/themes/COMPRESS_ITEM"));
}

void Archiver::ChangeWindowTitle(const QString& name, const int& Bytes) {
  setWindowTitle(QString("QZ Archive | %1 | %2").arg(name).arg(FILE_PROPERTY_DSP::sizeToHumanReadFriendly(Bytes)));
}

void Archiver::UpdateWindowsSize() {
  if (PreferenceSettings().contains("ArchiverGeometry")) {
    restoreGeometry(PreferenceSettings().value("ArchiverGeometry").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 1024, 768));
  }
  m_splitter->restoreState(PreferenceSettings().value("ArchiverSplitterState", QByteArray()).toByteArray());
}

void Archiver::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("ArchiverGeometry", saveGeometry());
  qDebug("Archiver geometry was resize to (%d, %d, %d, %d)", geometry().x(), geometry().y(), geometry().width(), geometry().height());
  PreferenceSettings().setValue("ArchiverSplitterState", m_splitter->saveState());
  QMainWindow::closeEvent(event);
}

void Archiver::subscribe() {
  connect(m_itemsList->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &Archiver::onNewRow);
}

bool Archiver::onNewRow(const QModelIndex& current, const QModelIndex& previous) {
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

  m_af.ResetPath(qzPath);
  m_archiverModel->setRootPath(nullptr);
  m_archiverModel->setRootPath(&m_af);
  return true;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  Archiver afPreview;
  afPreview.show();
  afPreview.operator()("E:/Brazzers/profiles/profiles.qz");
  return a.exec();
}
#endif
