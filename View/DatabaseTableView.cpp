#include "DatabaseTableView.h"

#include "Tools/PlayVideo.h"
#include "View/ViewHelper.h"

#include "Component/DatabaseToolBar.h"
#include "PublicTool.h"
#include "PublicVariable.h"

#include <QDesktopServices>
#include <QHeaderView>
#include <QProcess>

DatabaseTableView::DatabaseTableView() : m_dbModel(nullptr), menu(new DBRightClickMenu("Database Right click menu", this)) {
  setContextMenuPolicy(Qt::CustomContextMenu);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

  QSqlDatabase con = GetSqlDB();

  m_dbModel = new MyQSqlTableModel(this, con);
  if (con.tables().contains(TABLE_NAME)) {
    m_dbModel->setTable(TABLE_NAME);
  }
  m_dbModel->setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
  m_dbModel->select();

  this->setModel(m_dbModel);

  this->InitViewSettings();
  subscribe();
}

auto DatabaseTableView::InitViewSettings() -> void {
  setShowGrid(false);
  setAlternatingRowColors(true);
  setSortingEnabled(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  verticalHeader()->setVisible(false);
  verticalHeader()->setDefaultSectionSize(ROW_SECTION_HEIGHT);
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
  horizontalHeader()->setStretchLastSection(false);
  horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  DatabaseTableView::SetViewColumnWidth();
  DatabaseTableView::UpdateItemViewFontSize();
}

auto DatabaseTableView::SetViewColumnWidth() -> void {
  const auto columnWidth = PreferenceSettings().value(MemoryKey::NAME_COLUMN_WIDTH.name, MemoryKey::NAME_COLUMN_WIDTH.v).toInt();
  qDebug("%d:%d", DB_HEADER_KEY::DB_NAME_INDEX, columnWidth);
  setColumnWidth(DB_HEADER_KEY::DB_NAME_INDEX, columnWidth);
}

auto DatabaseTableView::UpdateItemViewFontSize() -> void {
  View::UpdateItemViewFontSizeCore(this);
}

void DatabaseTableView::subscribe() {
  {
    const QList<QAction*>& DB_RIGHT_CLICK_MENU_AG = g_dbAct().DB_RIGHT_CLICK_MENU_AG->actions();
    auto* OPEN_RUN = DB_RIGHT_CLICK_MENU_AG[0];
    auto* PLAY_VIDEOS = DB_RIGHT_CLICK_MENU_AG[1];
    auto* _REVEAL_IN_EXPLORER = DB_RIGHT_CLICK_MENU_AG[2];
    connect(OPEN_RUN, &QAction::triggered, this, [this]() { on_cellDoubleClicked(currentIndex()); });
    connect(PLAY_VIDEOS, &QAction::triggered, this, &DatabaseTableView::on_PlayVideo);
    connect(_REVEAL_IN_EXPLORER, &QAction::triggered, this, &DatabaseTableView::on_revealInExplorer);
  }

  connect(this, &QTableView::doubleClicked, this, &DatabaseTableView::on_cellDoubleClicked);
  connect(this, &QTableView::customContextMenuRequested, this, &DatabaseTableView::on_ShowContextMenu);
}

auto DatabaseTableView::on_cellDoubleClicked(QModelIndex clickedIndex) -> bool {
  if (not clickedIndex.isValid()) {
    return false;
  }
  if (not m_dbModel) {
    return false;
  }

  QFileInfo fi = m_dbModel->fileInfo(clickedIndex);
  qDebug("Enter(%d, %d) [%s]", clickedIndex.row(), clickedIndex.column(), fi.fileName().toStdString().c_str());
  if (not fi.exists()) {
    qDebug("[path inexists] %s", fi.absoluteFilePath().toStdString().c_str());
    return false;
  }
  if (fi.isSymLink()) {
#ifdef _WIN32
    QString tarPath = fi.symLinkTarget();
#else  // ref: https://doc.qt.io/qt-6/qfileinfo.html#isSymLink
    QString tarPath(fi.absoluteFilePath());
#endif
    fi = QFileInfo(tarPath);
    if (not fi.exists()) {
      qDebug("[link inexists] %s", fi.absoluteFilePath().toStdString().c_str());
      return false;
    }
  }
  QString path(fi.absoluteFilePath());
  return QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
}

auto DatabaseTableView::on_revealInExplorer() const -> bool {
  // hasSelection: reveal with selection
  // noSelection: folder -> open, file -> open its dir
  QModelIndex curIndex = selectionModel()->currentIndex();
  QStringList args;
  if (not curIndex.isValid()) {
    QString reveal_path = m_dbModel->rootPath();
    args = QStringList{QDir::toNativeSeparators(reveal_path)};
  } else {
    QFileInfo fi(m_dbModel->fileInfo(curIndex));
    if (not fi.exists()) {
      qDebug("Path[%s] not exists", fi.absoluteFilePath().toStdString().c_str());
      return false;
    }
    QString reveal_path(fi.absoluteFilePath());
    args = QStringList{"/e,", "/select,", QDir::toNativeSeparators(reveal_path)};
  }

  QProcess process;
#ifdef WIN32
  process.setProgram("explorer");
  process.setArguments(args);
#else
  process.setProgram(r "xdg-open");
  if (not QFileInfo(revealPath).isDir()) {
#is not dir = > reveal its dirname
    revealPath = QFileInfo(revealPath).absolutePath();
  }
  process.setArguments({revealPath});
#endif
  process.startDetached();  // Start the process in detached mode instead of start
  return true;
}

auto DatabaseTableView::on_PlayVideo() const -> bool {
  // select an item or select nothing
  const int selectedCnt = selectedIndexes().size();
  QString playPath;
  if (selectedCnt == 0) {
    if (m_dbModel->rootDirectory().isRoot()) {
      qDebug("root path is so large range. skip");
      return true;
    }
    playPath = m_dbModel->rootPath();
  } else if (selectedCnt == 1) {
    QModelIndex curIndex = selectionModel()->currentIndex();
    QFileInfo selectedFi = m_dbModel->fileInfo(curIndex);
    if (selectedFi.isDir()) {
      if (QDir(selectedFi.absoluteFilePath()).isRoot()) {
        qDebug("root path is so large range. skip");
        return true;
      }
    }
    playPath = selectedFi.absoluteFilePath();
  } else {
    qDebug("Select nothing MyQFileSystemModel or JUST Select 1");
    return true;
  }

  auto ret = on_ShiftEnterPlayVideo(playPath);
  qDebug(ret ? "Playing ..." : "Nothing to play");
  return ret;
}

DatabasePanel::DatabasePanel(QWidget* parent)
    : QWidget(parent), m_searchLE(new QLineEdit), m_searchCB(new QComboBox), m_dbView(new DatabaseTableView) {
  m_searchLE->setClearButtonEnabled(true);
  m_searchLE->addAction(QIcon(":/themes/SEARCH"), QLineEdit::LeadingPosition);
  m_searchCB->setLineEdit(m_searchLE);

  m_searchCB->addItem(QString("%1 not like \"_a%B\"").arg(DB_HEADER_KEY::Name));
  m_searchCB->addItem(QString("%1 in (\"ts\", \"avi\")").arg(DB_HEADER_KEY::Type));
  m_searchCB->addItem(QString("%1 between 0 AND 1000000").arg(DB_HEADER_KEY::Size));
  m_searchCB->addItem(QString("%1 = \"E:/\"").arg(DB_HEADER_KEY::Driver));

  QVBoxLayout* panelLo = new QVBoxLayout;
  panelLo->setContentsMargins(0, 0, 0, 0);
  panelLo->setSpacing(0);
  panelLo->addWidget(m_searchCB);
  panelLo->addWidget(m_dbView);
  setLayout(panelLo);
}

#include <QMainWindow>
class MoviesDatabase : public QMainWindow {
 public:
  DatabaseToolBar* m_databaseTB;
  DatabasePanel* m_dbPanel;
  explicit MoviesDatabase(QWidget* parent = nullptr)
      : QMainWindow(parent), m_databaseTB(new DatabaseToolBar("Movies Database Toolbar", this)), m_dbPanel(new DatabasePanel) {
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_databaseTB);
    this->setCentralWidget(m_dbPanel);
    this->setWindowTitle("QTableView Example");
    this->setWindowIcon(QIcon(":/themes/SHOW_DATABASE"));
  }
  QSize sizeHint() const override { return QSize(1400, 768); }
};

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
#include "Tools/SubscribeDatabase.h"

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  //  QWidget widget;
  //  QMessageBox::warning(&widget, "FILE_INFO_DATABASE path", SystemPath::FILE_INFO_DATABASE);
  //  widget.show();
  MoviesDatabase win;
  win.show();
  auto* eventImplementer = new SubscribeDatabase(win.m_dbPanel->m_dbView, win.m_dbPanel->m_dbView->m_dbModel, win.m_dbPanel->m_searchLE);
  return a.exec();
}
#endif
