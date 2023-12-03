#include "DatabaseTableView.h"

#include "Tools/PlayVideo.h"
#include "View/ViewHelper.h"

#include "Component/DatabaseToolBar.h"
#include "PublicTool.h"
#include "PublicVariable.h"

#include <QDesktopServices>
#include <QHeaderView>
#include <QProcess>
#include <QStorageInfo>

DatabaseTableView::DatabaseTableView() : m_dbModel(nullptr), m_vidsDBMenu(new DBRightClickMenu("Database Right click menu", this)) {
  setContextMenuPolicy(Qt::CustomContextMenu);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

  QSqlDatabase con = GetSqlVidsDB();

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

  {
    const QList<QAction*>& DB_FUNCTIONS_ACTIONS = g_dbAct().DB_FUNCTIONS->actions();
    QAction* COUNT = DB_FUNCTIONS_ACTIONS[0];
    QAction* AVG = DB_FUNCTIONS_ACTIONS[1];
    QAction* SUM = DB_FUNCTIONS_ACTIONS[2];
    connect(COUNT, &QAction::triggered, this, &DatabaseTableView::onCountRow);
  }
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
#ifdef _WIN32
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

  subscribe();
}

bool DatabasePanel::onInitDataBase() {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  qDebug("Database create succeed");
  return true;
}

void DatabasePanel::onInitATable() {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return;
  }

  if (con.tables().contains(TABLE_NAME)) {
    qDebug("Table[%s] already exists in database[%s]", TABLE_NAME.toStdString().c_str(), con.databaseName().toStdString().c_str());
    return;
  }

  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  const QString& createTableSQL = QString(
                                      "CREATE TABLE IF NOT EXISTS `%1`("
                                      "   `%2` TEXT NOT NULL,"
                                      "   `%3` INT,"
                                      "   `%4` TEXT NOT NULL,"
                                      "   `%5` TEXT,"
                                      "   `%6` TEXT,"
                                      "   `%7` TEXT,"
                                      "   `%8` INT,"
                                      "   `%9` TEXT,"
                                      "   `%10` TEXT NOT NULL,"
                                      "   `%11` TEXT,"
                                      "    PRIMARY KEY (%9, %10, "
                                      "                %2, %4,"
                                      "                %6, %7)"
                                      "    );")
                                      .arg(TABLE_NAME)
                                      .arg(DB_HEADER_KEY::Name)
                                      .arg(DB_HEADER_KEY::Size)
                                      .arg(DB_HEADER_KEY::Type)
                                      .arg(DB_HEADER_KEY::DateModified)
                                      .arg(DB_HEADER_KEY::Performers)
                                      .arg(DB_HEADER_KEY::Tags)
                                      .arg(DB_HEADER_KEY::Rate)
                                      .arg(DB_HEADER_KEY::Driver)
                                      .arg(DB_HEADER_KEY::Prepath)
                                      .arg(DB_HEADER_KEY::Extra);
  QSqlQuery createTableQuery(con);
  const auto ret = createTableQuery.exec(createTableSQL);
  if (not ret) {
    qDebug("Create table[%s] failed.", TABLE_NAME.toStdString().c_str());
    return;
  }

  m_dbView->m_dbModel->setTable(TABLE_NAME);
  m_dbView->m_dbModel->submitAll();
  qDebug("Table create succeed");
}

bool DatabasePanel::onDropATable() {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }

  if (not con.tables().contains(TABLE_NAME)) {
    qDebug("Table[%s] not exists", TABLE_NAME.toStdString().c_str());
    return true;
  }

  QSqlQuery dropQry(con);
  const auto dropTableRet = dropQry.exec(QString("DROP TABLE `%1`;").arg(TABLE_NAME));
  if (not dropTableRet) {
    qDebug("Drop Table[%s] failed. %s", TABLE_NAME.toStdString().c_str(), con.lastError().databaseText().toStdString().c_str());
  }
  dropQry.finish();
  m_dbView->m_dbModel->submitAll();
  return dropTableRet;
}

bool DatabasePanel::onDeleteFromTable(const QString& clause) {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }

  QStringList deleteDriverChoicePool;
  for (const QAction* act : g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions()) {
    deleteDriverChoicePool.append(QString("\"%1\"=\"%2\"").arg(DB_HEADER_KEY::Driver, act->text()));
  }
  deleteDriverChoicePool.append(QString("\"%1\"=\"\"").arg(DB_HEADER_KEY::Prepath));

  if (deleteDriverChoicePool.isEmpty()) {
    qDebug("Cancel [No need to delete rows from drive]");
    return true;
  }
  QString whereClause = clause;
  if (clause.isEmpty()) {
    bool okClicked = false;
    whereClause =
        QInputDialog::getItem(m_dbView, "Where clause", "DELETE FROM \"%1\" WHERE \t\t\t\t\t\t\t\t", deleteDriverChoicePool, 0, true, &okClicked);
    if (not okClicked or whereClause.isEmpty()) {
      qDebug("Cancel");
      return false;
    }
  }
  const QString& deleteCmd = QString("DELETE FROM \"%1\" WHERE %2").arg(TABLE_NAME, whereClause);

  QSqlQuery seleteQry(con);
  const auto deleteRes = seleteQry.exec(deleteCmd);
  qDebug("delete from result %d", deleteRes);
  m_dbView->m_dbModel->submitAll();
  return deleteRes;
}

bool DatabasePanel::on_DeleteByDrive() {
  const QList<QAction*>& DB_CONTROL_ACTIONS = g_dbAct().DB_CONTROL_ACTIONS->actions();
  QAction* DELETE_FROM_TABLE = DB_CONTROL_ACTIONS[1];

  QSet<QString> driversSet;

  for (const auto rowIndex : m_dbView->selectionModel()->selectedRows()) {
    const QString& curDriver = m_dbView->m_dbModel->driver(rowIndex);  // TODO:driver
    if (driversSet.contains(curDriver)) {
      continue;
    }
    driversSet.insert(curDriver);

    const QString& whereClause = QString("\"%1\"=\"%2\"").arg(DB_HEADER_KEY::Driver, curDriver);
    const auto ret = onDeleteFromTable(whereClause);
    if (not ret) {
      qDebug("Error when %s", whereClause.toStdString().c_str());
      return false;
    }
  }
  return true;
}

bool DatabasePanel::on_DeleteByPrepath() {
  const QList<QAction*>& DB_CONTROL_ACTIONS = g_dbAct().DB_CONTROL_ACTIONS->actions();
  QAction* DELETE_FROM_TABLE = DB_CONTROL_ACTIONS[1];

  QSet<QString> prepathSet;
  for (const auto rowIndex : m_dbView->selectionModel()->selectedRows()) {
    const QString& prepath = m_dbView->m_dbModel->absolutePath(rowIndex);
    if (prepathSet.contains(prepath)) {
      continue;
    }
    prepathSet.insert(prepath);
    const QString& whereClause = QString("\"%1\"=\"%2\"").arg(DB_HEADER_KEY::Prepath, prepath);
    const auto ret = onDeleteFromTable(whereClause);
    if (not ret) {
      qDebug("Error when %s", whereClause.toStdString().c_str());
      return false;
    }
  }
  return true;
}

bool DatabasePanel::onInsertIntoTable() {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  if (not con.tables().contains(TABLE_NAME)) {
    const QString& tablesNotExistsMsg = QString("Cannot insert, table[%1] not exist.").arg(TABLE_NAME);
    qDebug("%s", tablesNotExistsMsg.toStdString().c_str());
    QMessageBox::warning(m_dbView, "Insert abort", tablesNotExistsMsg);
    return false;
  }

  const QString& selectPath = QFileDialog::getExistingDirectory(m_dbView, "Choose a path");
  if (selectPath.isEmpty()) {
    qDebug("Path[%s] is not directory", selectPath.toStdString().c_str());
    return false;
  }

  if (QMessageBox::question(this, QString("Confirm? INSERT INTO %1 () VALUES ();").arg(TABLE_NAME), selectPath) != QMessageBox::StandardButton::Yes) {
    qDebug("User cancel insert", selectPath.toStdString().c_str());
    return true;
  }

  if (not con.transaction()) {
    qDebug() << "Failed to start transaction mode";
    return 0;
  }

  static const QString& insertTemplate = QString("INSERT INTO `%1` (%2) VALUES").arg(TABLE_NAME).arg(DB_HEADER_KEY::DB_HEADER.join(',')) +
                                         QString("(\"%1\", %2, \"%3\", \"%4\", \"%5\", \"%6\", %7, \"%8\", \"%9\", \"%10\");");

  int totalItemCnt = 0;
  int succeedItemCnt = 0;
  QSqlQuery insertTableQuery(con);

  QDirIterator it(selectPath, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  const QString CURRENT_DRIVE_LETTER = QStorageInfo(selectPath).rootPath();
  while (it.hasNext()) {
    it.next();
    const QFileInfo& fi = it.fileInfo();
    const QString& currentInsert = insertTemplate.arg(fi.fileName())
                                       .arg(fi.size())
                                       .arg(fi.suffix())
                                       .arg(fi.lastModified().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                                       .arg("")
                                       .arg("")
                                       .arg(0)
                                       .arg(CURRENT_DRIVE_LETTER)
                                       .arg(fi.absolutePath())
                                       .arg("");
    const bool insertResult = insertTableQuery.exec(currentInsert);
    succeedItemCnt += int(insertResult);
    if (not insertResult) {
      qDebug("Error [%s]: %s", currentInsert.toStdString().c_str(), insertTableQuery.lastError().text().toStdString().c_str());
    }
    ++totalItemCnt;
  }

  if (!con.commit()) {
    qDebug() << "Failed to commit, all will be rollback";
    con.rollback();
  }
  insertTableQuery.finish();
  const QString& msg = QString("%1/%2 item(s) add succeed. %3").arg(succeedItemCnt).arg(totalItemCnt).arg(selectPath);
  qDebug("%s", msg.toStdString().c_str());

  QMessageBox::information(m_dbView, "Insert into movie datebase", msg.toStdString().c_str());
  m_dbView->m_dbModel->submitAll();
  return true;
}

void DatabasePanel::onSelectBatch(const QAction* act) {
  bool changed = false;
  if (act->text() == "all") {
    for (auto* driveAct : g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions()) {
      if (not driveAct->isChecked()) {
        changed = true;
        driveAct->setChecked(true);
      }
    }
  } else if (act->text() == "none") {
    for (auto* driveAct : g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions()) {
      if (driveAct->isChecked()) {
        changed = true;
        driveAct->setChecked(false);
      }
    }
  } else if (act->text() == "online") {
    for (auto* driveAct : g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions()) {
      const bool exiDir = QFile::exists(act->text());
      if (driveAct->isChecked() != exiDir) {
        changed = true;
        driveAct->setChecked(exiDir);
      }
    }
  }
  if (not changed) {
    qDebug("[Normal] No need to change model filter(batch selection checked same).");
    return;
  }
  m_dbView->onSearchDataBase(m_searchLE->text());
}

void DatabasePanel::subscribe() {
  connect(g_dbAct().DRIVE_BATCH_SELECTION_AG, &QActionGroup::triggered, this, &DatabasePanel::onSelectBatch);
  connect(g_dbAct().DRIVE_SEPERATE_SELECTION_AG, &QActionGroup::triggered, this, &DatabasePanel::onSelectSingleDriver);

  connect(m_searchLE, &QLineEdit::returnPressed, this, [this]() {
    const QString& searchPattern = m_searchLE->text();
    m_dbView->onSearchDataBase(searchPattern);
  });

  {
    const QList<QAction*>& DB_CONTROL_ACTIONS = g_dbAct().DB_CONTROL_ACTIONS->actions();

    QAction* INSERT_A_PATH = DB_CONTROL_ACTIONS[0];
    QAction* DELETE_FROM_TABLE = DB_CONTROL_ACTIONS[1];
    QAction* INIT_A_DATABASE = DB_CONTROL_ACTIONS[2];
    QAction* INIT_A_TABLE = DB_CONTROL_ACTIONS[3];
    QAction* DROP_A_DATABASE = DB_CONTROL_ACTIONS[4];
    QAction* DROP_A_TABLE = DB_CONTROL_ACTIONS[5];

    connect(INIT_A_DATABASE, &QAction::triggered, this, &DatabasePanel::onInitDataBase);
    connect(DROP_A_DATABASE, &QAction::triggered, this, [this]() { QMessageBox::warning(m_dbView, "Too danger", "Cancel drop database"); });

    connect(INIT_A_TABLE, &QAction::triggered, this, &DatabasePanel::onInitATable);
    connect(DROP_A_TABLE, &QAction::triggered, this, &DatabasePanel::onDropATable);
    connect(INSERT_A_PATH, &QAction::triggered, this, &DatabasePanel::onInsertIntoTable);
    connect(DELETE_FROM_TABLE, &QAction::triggered, this, [this]() { this->onDeleteFromTable(); });
  }

  {
    const QList<QAction*>& DB_RIGHT_CLICK_MENU_AG = g_dbAct().DB_RIGHT_CLICK_MENU_AG->actions();
    auto* DELETE_BY_DRIVER = DB_RIGHT_CLICK_MENU_AG[3];
    auto* DELETE_BY_PREPATH = DB_RIGHT_CLICK_MENU_AG[4];
    connect(DELETE_BY_DRIVER, &QAction::triggered, this, &DatabasePanel::on_DeleteByDrive);
    connect(DELETE_BY_PREPATH, &QAction::triggered, this, &DatabasePanel::on_DeleteByPrepath);
  }
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

// #define __NAME__EQ__MAIN__ 1
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
