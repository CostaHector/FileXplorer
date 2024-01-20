#include "PerformersManagerWidget.h"

#include "Actions/PerformersManagerActions.h"
#include "Component/RatingSqlTableModel.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "Tools/DBTableMoviesHelper.h"
#include "Tools/JsonFileHelper.h"
#include "Tools/PerformerJsonFileHelper.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QDockWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

PerformersManagerWidget::PerformersManagerWidget(QWidget* parent)
    : QMainWindow{parent},
      m_performersListView(new QTableView),
      m_introductionTextEdit(new PerformersPreviewTextBrowser),
      m_performerCentralWidget(new QWidget),
      m_perfsDBModel(nullptr),
      m_performerTableMenu(new QMenu(this)),
      m_verticalHeaderMenu(new QMenu(this)),
      m_horizontalHeaderMenu(new QMenu(this)),
      m_imageHostPath(
          PreferenceSettings().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v).toString()),
      m_performerImageHeight(
          PreferenceSettings().value(MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.name, MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.v).toInt()),
      m_defaultTableRowCount(PreferenceSettings()
                                 .value(MemoryKey::PERFORMER_TABLE_DEFAULT_SECTION_SIZE.name, MemoryKey::PERFORMER_TABLE_DEFAULT_SECTION_SIZE.v)
                                 .toInt()),
      m_columnsShowSwitch(
          PreferenceSettings().value(MemoryKey::PERFORMER_COLUMN_SHOW_SWITCH.name, MemoryKey::PERFORMER_COLUMN_SHOW_SWITCH.v).toString()) {
  setCentralWidget(m_performersListView);

  auto* performerPreviewDoct = new QDockWidget("Overview", this);
  performerPreviewDoct->setWidget(m_introductionTextEdit);
  addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, performerPreviewDoct);

  setMenuBar(g_performersManagerActions().m_menuBar);
  m_performerTableMenu->addAction(g_performersManagerActions().REFRESH_SELECTED_RECORDS_VIDS);
  m_performerTableMenu->addSeparator();
  m_performerTableMenu->addAction(g_performersManagerActions().OPEN_RECORD_IN_FILE_SYSTEM);
  m_performerTableMenu->addSeparator();
  m_performerTableMenu->addAction(g_performersManagerActions().DUMP_SELECTED_RECORDS_INTO_PJSON_FILE);
  m_performerTableMenu->setToolTipsVisible(true);
  m_verticalHeaderMenu->addActions(g_performersManagerActions().VERTICAL_HEADER_AGS->actions());
  m_verticalHeaderMenu->setToolTipsVisible(true);
  m_horizontalHeaderMenu->addActions(g_performersManagerActions().HORIZONTAL_HEADER_AGS->actions());
  m_horizontalHeaderMenu->setToolTipsVisible(true);

  QSqlDatabase con = GetSqlDB();
  m_perfsDBModel = new RatingSqlTableModel(this, con);
  if (con.tables().contains(DB_TABLE::PERFORMERS)) {
    m_perfsDBModel->setTable(DB_TABLE::PERFORMERS);
  }
  m_perfsDBModel->setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);

  m_performersListView->setModel(m_perfsDBModel);
  m_performersListView->setAlternatingRowColors(true);
  m_performersListView->setSortingEnabled(true);
  m_performersListView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  m_performersListView->setDragDropMode(QAbstractItemView::NoDragDrop);
  m_performersListView->setEditTriggers(QAbstractItemView::EditKeyPressed);

  m_performersListView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_performersListView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
  m_performersListView->horizontalHeader()->setStretchLastSection(g_performersManagerActions().STRETCH_DETAIL_SECTION->isChecked());
  m_performersListView->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
  m_performersListView->verticalHeader()->setDefaultSectionSize(m_defaultTableRowCount);

  ShowOrHideColumnCore();
  subscribe();
  setWindowTitle("Performers Manager Widget");
  setWindowIcon(QIcon(":/themes/PERFORMERS_MANAGER"));
  updateWindowsSize();
}

void PerformersManagerWidget::subscribe() {
  connect(g_performersManagerActions().OPEN_WITH_LOCAL_APP, &QAction::triggered, this, [this]() {
    if (not QFile::exists(SystemPath::PEFORMERS_DATABASE)) {
      QMessageBox::information(this, "open failed", QString("[%1] not exists. \nCreate it first").arg(SystemPath::PEFORMERS_DATABASE));
      return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(SystemPath::PEFORMERS_DATABASE));
  });

  connect(g_performersManagerActions().INIT_DATABASE, &QAction::triggered, this, &PerformersManagerWidget::onInitDataBase);
  connect(g_performersManagerActions().INIT_TABLE, &QAction::triggered, this, &PerformersManagerWidget::onInitATable);
  connect(g_performersManagerActions().INSERT_INTO_TABLE, &QAction::triggered, this, &PerformersManagerWidget::onInsertIntoTable);
  connect(g_performersManagerActions().DROP_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DROP_OR_DELETE::DROP); });
  connect(g_performersManagerActions().DELETE_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DROP_OR_DELETE::DELETE); });

  connect(g_performersManagerActions().SUBMIT, &QAction::triggered, this, &PerformersManagerWidget::onSubmit);

  connect(g_performersManagerActions().LOCATE_IMAGEHOST, &QAction::triggered, this, &PerformersManagerWidget::onLocateImageHost);

  connect(g_performersManagerActions().CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT, &QAction::triggered, this,
          &PerformersManagerWidget::onChangePerformerImageHeight);

  connect(g_performersManagerActions().COLUMNS_VISIBILITY, &QAction::triggered, this, &PerformersManagerWidget::onShowHideColumn);

  connect(g_performersManagerActions().LOAD_FROM_FILE_SYSTEM_STRUCTURE, &QAction::triggered, this,
          &PerformersManagerWidget::onLoadFromFileSystemStructure);
  connect(g_performersManagerActions().LOAD_FROM_PERFORMERS_LIST, &QAction::triggered, this, &PerformersManagerWidget::onLoadFromPerformersList);
  connect(g_performersManagerActions().LOAD_FROM_PJSON_PATH, &QAction::triggered, this, &PerformersManagerWidget::onLoadFromPJsonDirectory);

  connect(g_performersManagerActions().DUMP_ALL_RECORDS_INTO_PJSON_FILE, &QAction::triggered, this, &PerformersManagerWidget::onDumpAllIntoPJsonFile);
  connect(g_performersManagerActions().DUMP_SELECTED_RECORDS_INTO_PJSON_FILE, &QAction::triggered, this,
          &PerformersManagerWidget::onDumpIntoPJsonFile);

  connect(g_performersManagerActions().OPEN_RECORD_IN_FILE_SYSTEM, &QAction::triggered, this, &PerformersManagerWidget::onOpenRecordInFileSystem);

  connect(m_performersListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PerformersManagerWidget::on_selectionChanged);

  connect(m_performersListView, &QListView::customContextMenuRequested, this, [this](const QPoint pnt) {
    m_performerTableMenu->popup(m_performersListView->mapToGlobal(pnt));  // or QCursor::pos()
  });

  connect(m_performersListView->verticalHeader(), &QHeaderView::customContextMenuRequested, this,
          [this](const QPoint pnt) { m_verticalHeaderMenu->popup(m_performersListView->mapToGlobal(pnt)); });

  connect(m_performersListView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, [this](const QPoint pnt) {
    m_horizontalHeaderSectionClicked = m_performersListView->horizontalHeader()->logicalIndexAt(pnt);
    m_horizontalHeaderMenu->popup(m_performersListView->mapToGlobal(pnt));
  });

  connect(g_performersManagerActions().DELETE_RECORDS, &QAction::triggered, this, &PerformersManagerWidget::onDeleteRecords);

  connect(g_performersManagerActions().HIDE_THIS_COLUMN, &QAction::triggered, this, &PerformersManagerWidget::onHideThisColumn);
  connect(g_performersManagerActions().SHOW_ALL_COLUMNS, &QAction::triggered, this, &PerformersManagerWidget::onShowAllColumn);

  connect(g_performersManagerActions().STRETCH_DETAIL_SECTION, &QAction::triggered, this, &PerformersManagerWidget::onStretchLastSection);
  connect(g_performersManagerActions().RESIZE_ROWS_TO_CONTENT, &QAction::triggered, this, &PerformersManagerWidget::onResizeRowToContents);
  connect(g_performersManagerActions().RESIZE_ROWS_DEFAULT_SECTION_SIZE, &QAction::triggered, this,
          &PerformersManagerWidget::onResizeRowDefaultSectionSize);

  connect(g_performersManagerActions().REFRESH_SELECTED_RECORDS_VIDS, &QAction::triggered, this, &PerformersManagerWidget::onForceRefreshRecordsVids);
  connect(g_performersManagerActions().REFRESH_ALL_RECORDS_VIDS, &QAction::triggered, this, &PerformersManagerWidget::onForceRefreshAllRecordsVids);
}

auto PerformersManagerWidget::closeEvent(QCloseEvent* event) -> void {
  PreferenceSettings().setValue("PerformersManagerWidgetGeometry", saveGeometry());
  PreferenceSettings().setValue("PerformersManagerWidgetDockerWidth", m_performersListView->width());
  PreferenceSettings().setValue("PerformersManagerWidgetDockerHeight", m_performersListView->height());
  QMainWindow::closeEvent(event);
}

void PerformersManagerWidget::updateWindowsSize() {
  if (PreferenceSettings().contains("PerformersManagerWidgetGeometry")) {
    restoreGeometry(PreferenceSettings().value("PerformersManagerWidgetGeometry").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 1024, 768));
  }
}

QSqlDatabase PerformersManagerWidget::GetSqlDB() const {
  QSqlDatabase con;
  if (QSqlDatabase::connectionNames().contains("perfs_connection")) {
    con = QSqlDatabase::database("perfs_connection", false);
  } else {
    con = QSqlDatabase::addDatabase("QSQLITE", "perfs_connection");
  }
  con.setDatabaseName(SystemPath::PEFORMERS_DATABASE);
  if (not con.open()) {
    qDebug("%s", con.lastError().text().toStdString().c_str());
  }
  return con;
}

bool PerformersManagerWidget::onInitDataBase() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  qDebug("Database create succeed");
  return true;
}

void PerformersManagerWidget::onInitATable() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return;
  }

  if (con.tables().contains(DB_TABLE::PERFORMERS)) {
    qDebug("Table[%s] already exists in database[%s]", qPrintable(DB_TABLE::PERFORMERS), con.databaseName().toStdString().c_str());
    return;
  }

  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  const QString& createTableSQL = PerformerJsonFileHelper::CreatePerformerTableSQL(DB_TABLE::PERFORMERS);
  QSqlQuery createTableQuery(con);
  const auto ret = createTableQuery.exec(createTableSQL);
  if (not ret) {
    qDebug("Create table[%s] failed.", qPrintable(DB_TABLE::PERFORMERS));
    return;
  }
  m_perfsDBModel->setTable(DB_TABLE::PERFORMERS);
  m_perfsDBModel->submitAll();
  qDebug("Table create succeed");
}

bool PerformersManagerWidget::onInsertIntoTable() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  if (not con.tables().contains(DB_TABLE::PERFORMERS)) {
    const QString& tablesNotExistsMsg = QString("Cannot insert, table[%1] not exist.").arg(DB_TABLE::PERFORMERS);
    qDebug("Table [%s] not exists", qPrintable(tablesNotExistsMsg));
    QMessageBox::warning(this, "Abort", tablesNotExistsMsg);
    return false;
  }
  const QString& name = QInputDialog::getText(this, "Insert a performer record", "Input name here (CANNOT be empty)");
  if (name.isEmpty()) {
    QMessageBox::warning(this, "Name is Empty", "Name cannot be empty");
    return false;
  }
  const QString& insertCmd = QString("INSERT INTO `%1` (%2) VALUES(\"%3\");").arg(DB_TABLE::PERFORMERS, PERFORMER_DB_HEADER_KEY::Name, name);
  QSqlQuery insertTableQuery(con);
  const bool insertResult = insertTableQuery.exec(insertCmd);
  if (not insertResult) {
    const QString& errorMsg = insertTableQuery.lastError().text();
    qDebug("[Command Failed] [%s]. %s", qPrintable(insertCmd), qPrintable(errorMsg));
    QMessageBox::warning(this, "Command Failed", insertCmd + '\n' + errorMsg);
    con.rollback();
    return false;
  }
  insertTableQuery.finish();
  m_perfsDBModel->submitAll();
  return true;
}

bool PerformersManagerWidget::onDropDeleteTable(const DROP_OR_DELETE dropOrDelete) {
  QString sqlCmd;
  switch (dropOrDelete) {
    case DROP_OR_DELETE::DROP:
      sqlCmd = QString("DROP TABLE `%1`;").arg(DB_TABLE::PERFORMERS);
      break;
    case DROP_OR_DELETE::DELETE:;
      sqlCmd = QString("DELETE FROM `%1`;").arg(DB_TABLE::PERFORMERS);
      break;
    default:
      qDebug("invalid choice");
      return false;
  }
  auto retBtn = QMessageBox::question(this, "Confirm drop/delete? (not recoverable)", sqlCmd);
  if (retBtn != QMessageBox::StandardButton::Yes) {
    qDebug("cancel");
    return true;
  }
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  if (not con.tables().contains(DB_TABLE::PERFORMERS)) {
    qDebug("Table[%s] not exists", qPrintable(DB_TABLE::PERFORMERS));
    return true;
  }

  QSqlQuery dropQry(con);
  const auto dropTableRet = dropQry.exec(sqlCmd);
  if (not dropTableRet) {
    qDebug("[Drop Table Failed] [%s]. Reason: %s", qPrintable(sqlCmd), dropQry.lastError().text().toStdString().c_str());
  }
  dropQry.finish();
  m_perfsDBModel->submitAll();
  QMessageBox::information(this, QString("Drop/Delete result: %1").arg(dropTableRet), sqlCmd);
  return dropTableRet;
}

int PerformersManagerWidget::onLoadFromFileSystemStructure() {
  if (not QDir(m_imageHostPath).exists()) {
    QMessageBox::warning(this, "Path[file-system structure] not exist", m_imageHostPath);
    qDebug("file-system structure [%s] not exists", qPrintable(m_imageHostPath));
    return false;
  }
  if (m_perfsDBModel->isDirty()) {
    QMessageBox::warning(this, "Cannot load from file-system structure now", "submit before load from file-system structure");
    qDebug("submit before load from file-system structure");
    return false;
  }
  const QString& insertTemplate = QString("INSERT INTO `%1` (%2,%3,%4) VALUES")
                                      .arg(DB_TABLE::PERFORMERS)
                                      .arg(PERFORMER_DB_HEADER_KEY::Name)
                                      .arg(PERFORMER_DB_HEADER_KEY::Orientation)
                                      .arg(PERFORMER_DB_HEADER_KEY::Imgs) +
                                  QString("(\"%1\", \"%2\", \"%3\") ON CONFLICT(") + PERFORMER_DB_HEADER_KEY::Name + (") DO UPDATE SET ") +
                                  PERFORMER_DB_HEADER_KEY::Orientation + "=\"%2\"," + PERFORMER_DB_HEADER_KEY::Imgs + "=\"%3\"";

  int loadCnt = 0;
  int succeedCnt = 0;
  QSqlDatabase con = GetSqlDB();
  if (con.transaction()) {
    QSqlQuery insertTableQuery(con);

    QMap<QString, QString> name2Ori;
    QMap<QString, QStringList> name2Imgs;
    QDirIterator it(m_imageHostPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
    while (it.hasNext()) {
      it.next();
      const QStringList& imgPath = it.filePath().split('/');
      const QString& imgName = imgPath.back();
      const QString& perfName = imgPath[imgPath.size() - 2];
      const QString& ori = imgPath[imgPath.size() - 3];
      if (name2Imgs.contains(perfName)) {
        name2Imgs[perfName] << imgName;
      } else {
        name2Imgs.insert(perfName, {imgName});
        name2Ori[perfName] = ori;
      }
    }
    for (auto mpIt = name2Ori.cbegin(); mpIt != name2Ori.cend(); ++mpIt) {
      const QString& perf = mpIt.key();
      const QString& ori = mpIt.value();
      const QString& imgs = name2Imgs[perf].join(PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR);  // img seperated by \n
      const QString& currentInsert = insertTemplate.arg(perf, ori, imgs);
      bool ret = insertTableQuery.exec(currentInsert);
      succeedCnt += ret;
      ++loadCnt;
      if (not ret) {
        const QString& errorMsg = insertTableQuery.lastError().text();
        qDebug("[Command Failed] [%s]. Reason: %s", qPrintable(currentInsert), qPrintable(errorMsg));
      }
    }

    if (!con.commit()) {
      qDebug() << "Failed to commit, all will be rollback";
      con.rollback();
    }
    insertTableQuery.finish();
    m_perfsDBModel->submitAll();
  } else {
    qDebug() << "Failed to start transaction mode";
    return 0;
  }
  qDebug("pjson file load succeed %d/%d", succeedCnt, loadCnt);
  return 0;
}

int PerformersManagerWidget::onLoadFromPerformersList() {
  bool ok = false;
  const QString& perfsText =
      QInputDialog::getMultiLineText(this, "Performers List", "Example:\nA(a1, a2)\r\nB(b1, b2))\n...\nseperated by \\r\\n.", "", &ok);
  if (not ok) {
    return 0;
  }
  QMap<QString, QString> perfs;
  for (const QString& line : perfsText.split(PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR)) {
    if (line.isEmpty()) {
      continue;
    }
    QStringList aka = line.split(JSON_RENAME_REGEX::SEPERATOR_COMP);
    QString stdName = aka.front();
    aka.pop_front();
    perfs.insert(stdName, aka.join(","));
  }
  if (perfs.isEmpty()) {
    return 0;
  }

  const QString& insertTemplate =
      QString("INSERT INTO `%1` (%2,%3) VALUES").arg(DB_TABLE::PERFORMERS).arg(PERFORMER_DB_HEADER_KEY::Name).arg(PERFORMER_DB_HEADER_KEY::AKA) +
      QString("(\"%1\", \"%2\") ON CONFLICT(%3) DO UPDATE SET %4 = \"%2\"");

  QSqlDatabase con = GetSqlDB();
  if (con.transaction()) {
    QSqlQuery insertTableQuery(con);
    // update aka by new value if name conflict
    for (auto it = perfs.cbegin(); it != perfs.cend(); ++it) {
      const QString& currentInsert =
          insertTemplate.arg(it.key()).arg(it.value()).arg(PERFORMER_DB_HEADER_KEY::Name).arg(PERFORMER_DB_HEADER_KEY::AKA);
      bool ret = insertTableQuery.exec(currentInsert);
      if (not ret) {
        qDebug("[Failed] %s", qPrintable(currentInsert));
      }
    }
    if (!con.commit()) {
      qDebug() << "Failed to commit";
      con.rollback();
    }
    insertTableQuery.finish();
    m_perfsDBModel->submitAll();
  } else {
    qDebug() << "Failed to start transaction mode";
  }
  return perfs.size();
}

bool PerformersManagerWidget::onLocateImageHost() {
  const QString& locatePath = QFileDialog::getExistingDirectory(this, "Locate imagehost folder", m_imageHostPath);
  if (not QFile::exists(locatePath)) {
    qDebug("locate path not exist");
    return false;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, m_imageHostPath = locatePath);
  g_performersManagerActions().LOCATE_IMAGEHOST->setToolTip(m_imageHostPath);
  return true;
}

bool PerformersManagerWidget::onChangePerformerImageHeight() {
  bool ok = false;
  int height = QInputDialog::getInt(this, "Performer image height(px)", QString("default: %1").arg(m_performerImageHeight), m_performerImageHeight, 0,
                                    INT_MAX, 1, &ok);
  if (not ok) {
    return false;
  }
  m_performerImageHeight = height;
  PreferenceSettings().setValue(MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.name, m_performerImageHeight);
  g_performersManagerActions().CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT->setToolTip(QString::number(height));
  return true;
}

bool PerformersManagerWidget::ShowOrHideColumnCore() {
  if (not m_performersListView) {
    return false;
  }
  const int headColumnCntMin = std::min({m_perfsDBModel->columnCount(), PERFORMER_DB_HEADER_KEY::DB_HEADER.size()});
  for (int c = 0; c < headColumnCntMin; ++c) {
    m_performersListView->setColumnHidden(c, m_columnsShowSwitch[c] == '0');
  }
  PreferenceSettings().setValue(MemoryKey::PERFORMER_COLUMN_SHOW_SWITCH.name, m_columnsShowSwitch);
  return true;
}

bool PerformersManagerWidget::onShowHideColumn() {
  bool ok = false;
  const QString& showHideSwitchArray = QInputDialog::getText(
      this, "Performer table column visibility(0:hide, 1:show)",
      PERFORMER_DB_HEADER_KEY::DB_HEADER.join(',') + "\nExtra element will be dismissed simply", QLineEdit::Normal, m_columnsShowSwitch, &ok);
  if (not ok) {
    return false;
  }
  if (showHideSwitchArray.size() < PERFORMER_DB_HEADER_KEY::DB_HEADER.size()) {
    QMessageBox::warning(this, "[Cancel] Columns count invalid",
                         QString("Condition: length[%1] >= %2.").arg(showHideSwitchArray.size()).arg(PERFORMER_DB_HEADER_KEY::DB_HEADER.size()));
    return false;
  }
  m_columnsShowSwitch = showHideSwitchArray;
  ShowOrHideColumnCore();
  return true;
}

inline bool PerformersManagerWidget::onSubmit() {
  if (not m_perfsDBModel->isDirty()) {
    qDebug("No need to submit");
    return true;
  }
  return m_perfsDBModel->submitAll();
}

bool PerformersManagerWidget::on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
  if (not m_performersListView->currentIndex().isValid()) {
    m_introductionTextEdit->setText("");
    return true;
  }
  const auto& record = m_perfsDBModel->record(m_performersListView->currentIndex().row());
  m_introductionTextEdit->operator()(record, m_imageHostPath, m_performerImageHeight);
  return true;
}

int PerformersManagerWidget::onLoadFromPJsonDirectory() {
  if (not QDir(m_imageHostPath).exists()) {
    QMessageBox::warning(this, "Path[pjson load from] not exist", m_imageHostPath);
    qDebug("*.pjson path load from [%s] not exists", qPrintable(m_imageHostPath));
    return false;
  }
  if (m_perfsDBModel->isDirty()) {
    QMessageBox::warning(this, "Cannot load pjson now", "submit before load pjson");
    qDebug("submit before load pjson");
    return false;
  }

  int loadCnt = 0;
  int succeedCnt = 0;
  QSqlDatabase con = GetSqlDB();
  if (con.transaction()) {
    QSqlQuery insertTableQuery(con);
    QDirIterator it(m_imageHostPath, {"*.pjson"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
    while (it.hasNext()) {
      it.next();
      const QVariantHash& pJson = JsonFileHelper::MovieJsonLoader(it.filePath());
      const QString& currentInsert = PerformerJsonFileHelper::PerformerInsertSQL(DB_TABLE::PERFORMERS, pJson);
      bool ret = insertTableQuery.exec(currentInsert);
      succeedCnt += ret;
      ++loadCnt;
      if (not ret) {
        qDebug("[Command Failed] [%s]. Reason: %s", qPrintable(currentInsert), insertTableQuery.lastError().text().toStdString().c_str());
      }
    }
    if (!con.commit()) {
      qDebug() << "Failed to commit, all will be rollback";
      con.rollback();
    }
    insertTableQuery.finish();
    m_perfsDBModel->submitAll();
  } else {
    qDebug() << "Failed to start transaction mode";
    return 0;
  }
  qDebug("pjson file load succeed %d/%d", succeedCnt, loadCnt);
  return 0;
}

int PerformersManagerWidget::onDumpAllIntoPJsonFile() {
  if (not QDir(m_imageHostPath).exists()) {
    qDebug("*.pjson path dump to [%s] not exists", qPrintable(m_imageHostPath));
    QMessageBox::warning(this, "Path[pjson dump to] not exist", m_imageHostPath);
    return 0;
  }
  int dumpCnt = 0;
  int succeedCnt = 0;
  for (int r = 0; r < m_perfsDBModel->rowCount(); ++r) {
    const auto& pJson = PerformerJsonFileHelper::PerformerJsonJoiner(m_perfsDBModel->record(r));
    const QString& pJsonPath = PerformerJsonFileHelper::PJsonPath(m_imageHostPath, pJson);
    succeedCnt += JsonFileHelper::MovieJsonDumper(pJson, pJsonPath);
    ++dumpCnt;
  }
  qDebug("All %d record(s) dump into pjson file. succeed: %d/%d.", dumpCnt, succeedCnt, dumpCnt);
  QMessageBox::information(this, QString("All %1 record(s) dumped result").arg(dumpCnt), QString("succeed:%1/%2").arg(succeedCnt).arg(dumpCnt));
  return succeedCnt;
}

int PerformersManagerWidget::onDumpIntoPJsonFile() {
  if (not QDir(m_imageHostPath).exists()) {
    qDebug("*.pjson path dump to [%s] not exists", qPrintable(m_imageHostPath));
    QMessageBox::warning(this, "Path[pjson dump to] not exist", m_imageHostPath);
    return 0;
  }

  if (not m_performersListView->selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select some row to dump");
    QMessageBox::warning(this, "Nothing was selected", "Select some row to dump");
    return 0;
  }
  int dumpCnt = 0;
  int succeedCnt = 0;
  for (auto indr : m_performersListView->selectionModel()->selectedRows()) {
    const int r = indr.row();
    const auto& pJson = PerformerJsonFileHelper::PerformerJsonJoiner(m_perfsDBModel->record(r));
    const QString& pJsonPath = PerformerJsonFileHelper::PJsonPath(m_imageHostPath, pJson);
    succeedCnt += JsonFileHelper::MovieJsonDumper(pJson, pJsonPath);
    ++dumpCnt;
  }
  qDebug("Selected %d record(s) dump into pjson file. succeed: %d/%d.", dumpCnt, succeedCnt, dumpCnt);
  QMessageBox::information(this, QString("Selected %1 record(s) dumped result").arg(dumpCnt), QString("succeed:%1/%2").arg(succeedCnt).arg(dumpCnt));
  return succeedCnt;
}

int PerformersManagerWidget::onForceRefreshAllRecordsVids() {
  QMessageBox::information(this, QString("Oops function not support now"),
                           QString("But you could selected all record(s) and then force refresh instead."));
  return 0;
}

int PerformersManagerWidget::onForceRefreshRecordsVids() {
  if (not m_performersListView->selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select some records to refresh");
    QMessageBox::warning(this, "Nothing was selected", "Select some row to refresh");
    return 0;
  }
  QSqlDatabase con = ::GetSqlVidsDB();  // videos table
  if (not con.isOpen()) {
    qDebug("con cannot open [%s]", qPrintable(SystemPath::VIDS_DATABASE));
    return 0;
  }

  static auto GetVidsListFromVidsTable = [](const QSqlRecord& record, QSqlQuery& qur) -> QStringList {
    const QString& searchCommand = DBTableMoviesHelper::GetMovieTablePerformerSelectCommand(record);
    bool ret = qur.exec(searchCommand);
    if (not ret) {
      qDebug("Failed when[%s]", qPrintable(searchCommand));
      return {};
    }
    QStringList vidPath;
    while (qur.next()) {
      vidPath << qur.value(DB_HEADER_KEY::ForSearch).toString();
    }
    return vidPath;
  };

  QSqlQuery qur(con);
  int recordsCnt = 0;
  int vidsCnt = 0;
  for (auto indr : m_performersListView->selectionModel()->selectedRows()) {
    const int r = indr.row();
    auto record = m_perfsDBModel->record(r);
    const QStringList& vidsList = GetVidsListFromVidsTable(record, qur);
    record.setValue(PERFORMER_DB_HEADER_KEY::Vids, vidsList.join(PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR));
    m_perfsDBModel->setRecord(r, record);  // update back
    vidsCnt += vidsList.size();
    ++recordsCnt;
  }
  qDebug("Selected %d record(s) updated %d vid(s).", recordsCnt, vidsCnt);
  QMessageBox::information(this, QString("Selected %1 record(s) updated.").arg(recordsCnt), QString("%1 vid(s)").arg(vidsCnt));
  return recordsCnt;
}

bool PerformersManagerWidget::onOpenRecordInFileSystem() const {
  if (not QDir(m_imageHostPath).exists()) {
    qDebug("m_imageHostPath [%s] not exists", qPrintable(m_imageHostPath));
    return false;
  }
  if (not m_performersListView->selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select a row to open pjson folder");
    return false;
  }

  const auto& record = m_perfsDBModel->record(m_performersListView->currentIndex().row());
  QString folderPath = QString("%1/%2/%3")
                           .arg(m_imageHostPath)
                           .arg(record.field(PERFORMER_DB_HEADER_KEY::Orientation).value().toString())
                           .arg(record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString());
  if (m_performersListView->currentIndex().column() == PERFORMER_DB_HEADER_KEY::Detail_INDEX) {
    folderPath += QString("/%1.pjson").arg(record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString());
  }
  if (not QFile::exists(folderPath)) {
    qDebug("Path[%s] not exists", qPrintable(folderPath));
    return false;
  }
  return QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}

bool PerformersManagerWidget::onHideThisColumn() {
  const int c = m_horizontalHeaderSectionClicked;
  if (c < 0 or c >= m_columnsShowSwitch.size()) {
    qDebug("No column selected. Select a column to hide");
    QMessageBox::warning(this, "No column selected", "Select a column to hide");
    return false;
  }
  if (m_columnsShowSwitch[c] == '0') {
    qDebug("Column[%d] already hide. Select another column to hide", c);
    QMessageBox::warning(this, QString("Column[%1] already hide").arg(c), "Select another column to hide");
    return true;
  }
  m_columnsShowSwitch[c] = '0';
  return ShowOrHideColumnCore();
}

bool PerformersManagerWidget::onShowAllColumn() {
  m_columnsShowSwitch.replace('0', '1');
  return ShowOrHideColumnCore();
}

int PerformersManagerWidget::onDeleteRecords() {
  if (not m_performersListView->selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select some row(s) to delete");
    QMessageBox::warning(this, "Nothing was selected", "Select some row(s) to delete");
    return 0;
  }
  int deleteCnt = 0;
  int succeedCnt = 0;
  const auto& itemSelection = m_performersListView->selectionModel()->selection();
  for (auto it = itemSelection.crbegin(); it != itemSelection.crend(); ++it) {
    int startRow = it->top();  // [top, bottom]
    int size = it->bottom() - startRow + 1;
    bool ret = m_perfsDBModel->removeRows(startRow, size);
    qDebug("drop[%d] records [%d, %d]", ret, startRow, it->bottom());
    deleteCnt += size;
    succeedCnt += ((int)ret * size);
  }
  m_perfsDBModel->submitAll();
  qDebug("delete records succeed: %d/%d.", succeedCnt, deleteCnt);
  QMessageBox::information(this, "delete records result", QString("%1/%2 succeed").arg(succeedCnt).arg(deleteCnt));
  return succeedCnt;
}

void PerformersManagerWidget::onStretchLastSection(const bool checked) {
  m_performersListView->horizontalHeader()->setStretchLastSection(checked);
  PreferenceSettings().setValue(MemoryKey::PERFORMER_STRETCH_LAST_SECTION.name, checked);
}

void PerformersManagerWidget::onResizeRowToContents(const bool checked) {
  if (checked) {
    m_performersListView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
  } else {
    m_performersListView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    m_performersListView->verticalHeader()->setDefaultSectionSize(m_defaultTableRowCount);
  }
}

void PerformersManagerWidget::onResizeRowDefaultSectionSize() {
  const int size = QInputDialog::getInt(this, "Resize Row Default Section size >=0 ", QString("default size:%1").arg(m_defaultTableRowCount),
                                        m_defaultTableRowCount);
  if (size < 0) {
    qDebug("Cancel resize");
    return;
  }
  m_defaultTableRowCount = size;
  m_performersListView->verticalHeader()->setDefaultSectionSize(size);
  PreferenceSettings().setValue(MemoryKey::PERFORMER_TABLE_DEFAULT_SECTION_SIZE.name, size);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  PerformersManagerWidget perfManaWid;
  perfManaWid.show();
  a.exec();
  return 0;
}
#endif
