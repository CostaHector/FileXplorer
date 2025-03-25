#include "RedundantImageFinder.h"

#include <QAbstractTableModel>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QListView>
#include <QTableView>
#include <QToolBar>

#include "Actions/FileBasicOperationsActions.h"
#include "Component/NotificatorFrame.h"
#include "PublicVariable.h"
#include "Tools/MD5Calculator.h"
#include "Tools/PathTool.h"
#include "Tools/QAbstractTableModelPub.h"
#include "UndoRedo.h"
#include "public/DisplayEnhancement.h"
#include "qdesktopservices.h"
#include "View/CustomTableView.h"

#include <QDataStream>

class RedundantImageFinder;

class RedundantImageModel : public QAbstractTableModelPub {
 public:
  friend class RedundantImageFinder;
  explicit RedundantImageModel(QObject* parent = nullptr) : QAbstractTableModelPub{parent} {}
  auto rowCount(const QModelIndex& parent = {}) const -> int override { return m_paf != nullptr ? m_paf->size() : 0; }
  auto columnCount(const QModelIndex& parent = {}) const -> int override { return HORIZONTAL_HEADER.size(); }
  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override {
    if (m_paf == nullptr or not index.isValid()) {
      return QVariant();
    }
    switch (role) {
      case Qt::DisplayRole: {
        switch (index.column()) {
          case 0:
            return PATHTOOL::fileName(m_paf->operator[](index.row()).filePath);
          case 1:
            return FILE_PROPERTY_DSP::sizeToHumanReadFriendly(m_paf->operator[](index.row()).size);
          case 2:
            return m_paf->operator[](index.row()).md5;
          default:
            return QVariant();
        }
        break;
      }
      case Qt::DecorationRole: {
        if (index.column() == HORIZONTAL_HEADER.size() - 1) {
          QPixmap pm{m_paf->operator[](index.row()).filePath};
          return pm.scaledToWidth(128);
        }
        break;
      }
      default:
        break;
    }
    return QVariant();
  }
  auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }else if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Horizontal) {
        return HORIZONTAL_HEADER[section];
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  QString filePath(const QModelIndex& index) const {
    if (m_paf == nullptr or not index.isValid())
      return "";
    const int r = index.row();
    if (not(0 <= r and r < rowCount())) {
      qWarning("r[%d] out of range[0, %d)", r, rowCount());
      return "";
    }
    return m_paf->operator[](r).filePath;
  }

  void setRootPath(const REDUNDANT_IMG_BUNCH* p_af) {
    int beforeRow = rowCount();
    int afterRow = p_af != nullptr ? p_af->size() : 0;
    qDebug("setRootPath. RowCountChanged: %d->%d", beforeRow, afterRow);

    RowsCountStartChange(beforeRow, afterRow);
    m_paf = p_af;
    RowsCountEndChange(beforeRow, afterRow);
  }

 private:
  const REDUNDANT_IMG_BUNCH* m_paf{nullptr};
  static const QStringList HORIZONTAL_HEADER;
};
const QStringList RedundantImageModel::HORIZONTAL_HEADER{"Name", "Size(B)", "MD5", "Preview"};

bool RedundantImageFinder::ALSO_RECYCLE_EMPTY_IMAGE = true;
QSet<qint64> RedundantImageFinder::m_commonFileSizeSet;
QSet<QString> RedundantImageFinder::m_commonFileHash;

RedundantImageFinder::RedundantImageFinder(QWidget* parent)
    : QMainWindow{parent},
      m_imgModel{new RedundantImageModel{this}},
      m_table{new CustomTableView{"RedundantImageTable", this}},
      m_toolBar{new QToolBar{"RedundantImageFinderToolbar", this}} {
  RECYLE_NOW->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_Delete));
  RECYLE_NOW->setToolTip(
      QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the Recyle Bin.").arg(RECYLE_NOW->text(), RECYLE_NOW->shortcut().toString()));

  RECYCLE_EMPTY_IMAGE->setCheckable(true);
  RECYCLE_EMPTY_IMAGE->setChecked(ALSO_RECYCLE_EMPTY_IMAGE);
  RECYCLE_EMPTY_IMAGE->setToolTip(QString("<b>%1 (%2)</b><br/> Empty images alse be regard as redundant image.")
                                      .arg(RECYCLE_EMPTY_IMAGE->text(), RECYCLE_EMPTY_IMAGE->shortcut().toString()));

  OPEN_REDUNDANT_IMAGES_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/> Open redundant images learned from folder.")
                                               .arg(OPEN_REDUNDANT_IMAGES_FOLDER->text(), OPEN_REDUNDANT_IMAGES_FOLDER->shortcut().toString()));

  m_toolBar->addAction(RECYLE_NOW);
  m_toolBar->addSeparator();
  m_toolBar->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());
  m_toolBar->addSeparator();
  m_toolBar->addAction(RECYCLE_EMPTY_IMAGE);
  m_toolBar->addSeparator();
  m_toolBar->addAction(OPEN_REDUNDANT_IMAGES_FOLDER);

  m_toolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  addToolBar(m_toolBar);

  m_table->setModel(m_imgModel);
  m_table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

  m_table->horizontalHeader()->setStretchLastSection(true);
  m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
  m_table->horizontalHeader()->setSectionResizeMode(RedundantImageModel::HORIZONTAL_HEADER.indexOf("MD5"), QHeaderView::ResizeMode::Stretch);
  m_table->horizontalHeader()->setSectionResizeMode(RedundantImageModel::HORIZONTAL_HEADER.indexOf("Name"), QHeaderView::ResizeMode::ResizeToContents);

  m_table->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
  m_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  m_table->setTextElideMode(Qt::TextElideMode::ElideLeft);
  m_table->setShowGrid(false);

  setCentralWidget(m_table);

  subscribe();

  m_imgModel->setRootPath(&m_imgsBunch);

  setWindowIcon(QIcon(":img/REDUNDANT_IMAGE_FINDER"));
  setWindowTitle("Redundant Images Finder");
  setMinimumSize(1024, 768);

  LearnCommonImageCharacteristic(GetRedunPath());
}

RedundantImageFinder::~RedundantImageFinder() {
  if (m_libFi.isOpen()) {
    m_libFi.close();
  }
}

void RedundantImageFinder::ChangeWindowTitle(const QString& rootPath) {
  setWindowTitle(QString("Redundant Images Finder | %1 | %2 item(s)").arg(rootPath).arg(m_imgsBunch.size()));
}

QString RedundantImageFinder::GetRedunPath() const {
#ifdef _WIN32
  return PreferenceSettings().value(MemoryKey::WIN32_RUND_IMG_PATH.name).toString();
#else
  return PreferenceSettings().value(MemoryKey::LINUX_RUND_IMG_PATH.name).toString();
#endif
}

void RedundantImageFinder::subscribe() {
  connect(RECYLE_NOW, &QAction::triggered, this, &RedundantImageFinder::RecycleSelection);
  connect(RECYCLE_EMPTY_IMAGE, &QAction::triggered, this, [](bool recycleEmptyImage) -> void { ALSO_RECYCLE_EMPTY_IMAGE = recycleEmptyImage; });
  connect(OPEN_REDUNDANT_IMAGES_FOLDER, &QAction::triggered, this, [this]() { QDesktopServices::openUrl(QUrl::fromLocalFile(GetRedunPath())); });
}

void RedundantImageFinder::RecycleSelection() {
  const QModelIndexList& sel = m_table->selectionModel()->selectedRows();
  const int SELECTED_CNT = sel.size();
  if (SELECTED_CNT <= 0) {
    return;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE recycleCmds;
  recycleCmds.reserve(SELECTED_CNT);
  for (const auto& srcInd : sel) {
    recycleCmds.append(ACMD{MOVETOTRASH, {"", m_imgModel->filePath(srcInd)}});
  }
  auto isRenameAllSucceed = g_undoRedo.Do(recycleCmds);
  qDebug("Recycle %d item(s) %d.", SELECTED_CNT, isRenameAllSucceed);
  if (isRenameAllSucceed) {
    Notificator::goodNews("Recyle redundant images succeed", QString::number(SELECTED_CNT));
  } else {
    Notificator::badNews("Recyle redundant images failed", QString::number(SELECTED_CNT));
  }
  UpdateDisplayWhenRecycled();
}

void RedundantImageFinder::UpdateDisplayWhenRecycled() {
  decltype(m_imgsBunch) redundantImgs;
  for (const auto& info : m_imgsBunch) {
    if (not QFile::exists(info.filePath))
      continue;
    redundantImgs.append(info);
  }

  int beforeRowCnt = m_imgsBunch.size();
  int afterRowCnt = redundantImgs.size();
  m_imgModel->RowsCountStartChange(beforeRowCnt, afterRowCnt);
  m_imgsBunch.swap(redundantImgs);
  m_imgModel->RowsCountEndChange(beforeRowCnt, afterRowCnt);
}

void RedundantImageFinder::ReadLocalCharacteristicLib(const QString& libPath) {
  m_libFi.setFileName(libPath);
  if ((not m_libFi.isOpen()) and (not m_libFi.open(QFile::OpenModeFlag::ReadOnly))) {
    qWarning("Cannot open file[%s] for read", qPrintable(libPath));
    return;
  }

  QDataStream ds;
  ds.setDevice(&m_libFi);
  qint64 size;
  QString md5;
  while (not ds.atEnd()) {
    ds >> size >> md5;
    m_commonFileSizeSet.insert(size);
    m_commonFileHash.insert(md5);
  }
}

void RedundantImageFinder::LearnCommonImageCharacteristic(const QString& folderPath) {
  QDirIterator it(folderPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    QFileInfo imgFi(it.filePath());
    const QString fileAbsPath = imgFi.absoluteFilePath();
    const qint64 sz = imgFi.size();
    m_commonFileSizeSet.insert(sz);
    const QString& md5 = MD5Calculator::GetFileMD5(fileAbsPath);
    m_commonFileHash.insert(md5);
  }
  qDebug("redundant image info size[%d] and hash[%d]", m_commonFileSizeSet.size(), m_commonFileHash.size());
}

void RedundantImageFinder::operator()(const QString& folderPath) {
  QDirIterator it(folderPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  REDUNDANT_IMG_BUNCH redundantImgs;
  while (it.hasNext()) {
    it.next();
    QFileInfo imgFi(it.filePath());
    const QString fileAbsPath = imgFi.absoluteFilePath();
    const qint64 sz = imgFi.size();
    if (sz == 0) {
      if (ALSO_RECYCLE_EMPTY_IMAGE) {
        redundantImgs.append(REDUNDANT_IMG_INFO{fileAbsPath, 0, ""});
      }
      continue;
    }
    if (not m_commonFileSizeSet.contains(sz)) {
      continue;
    }
    const QString& md5 = MD5Calculator::GetFileMD5(fileAbsPath);
    if (not m_commonFileHash.contains(md5)) {
      continue;
    }
    redundantImgs.append(REDUNDANT_IMG_INFO{fileAbsPath, sz, md5});
  }

  int beforeRowCnt = m_imgsBunch.size();
  int afterRowCnt = redundantImgs.size();
  m_imgModel->RowsCountStartChange(beforeRowCnt, afterRowCnt);
  m_imgsBunch.swap(redundantImgs);
  m_imgModel->RowsCountEndChange(beforeRowCnt, afterRowCnt);
  ChangeWindowTitle(folderPath);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  RedundantImageFinder rif;
  rif("E:/torrents/gay/imagesnotallowed");
  rif.show();
  return a.exec();
}

#endif
