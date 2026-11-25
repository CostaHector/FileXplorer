#include "CastDBViewSplitter.h"
#include "PublicCheckMacro.h"
#include "SystemPath.h"
#include "CastDBActions.h"
#include "NotificatorMacro.h"
#include "InputDialogHelper.h"
#include "DatabaseSearchToolBar.h"
#include "PublicVariable.h"
#include "PathKey.h"
#include "Configuration.h"
#include "CastKey.h"
#include <QSqlError>

CastDBViewSplitter::CastDBViewSplitter(const QString& castDbFullPath, const QString& castDbConnName, const QString& imgHostFullPath,
                                       CastDatabaseSearchToolBar* castDbSearchBar_, QWidget* parent)
  : QSplitter{parent}, _castDbSearchBar{castDbSearchBar_} {
  CHECK_NULLPTR_RETURN_VOID(castDbSearchBar_);

  setChildrenCollapsible(false);
  setHandleWidth(4);

  m_castDb = new (std::nothrow) CastBaseDb{castDbFullPath, castDbConnName, this};
  CHECK_NULLPTR_RETURN_VOID(m_castDb);

  m_castDbModel = new (std::nothrow) CastDbModel{imgHostFullPath, this, m_castDb->GetDb()};
  CHECK_NULLPTR_RETURN_VOID(m_castDbModel)
  m_sharedSelectionModel = new (std::nothrow) QItemSelectionModel{m_castDbModel, this};
  CHECK_NULLPTR_RETURN_VOID(m_sharedSelectionModel)
  m_castTableView = new (std::nothrow) CastDBTableView{m_castDbModel, m_sharedSelectionModel, this};
  CHECK_NULLPTR_RETURN_VOID(m_castTableView)
  addWidget(m_castTableView);

  if (CastDBActions::GetInst().m_showCastsCardsListAct->isChecked()) {
    onShowCastListView(true);
  }
  subscribe();
}

CastDBViewSplitter::CastDBViewSplitter(CastDatabaseSearchToolBar* castDbSearchBar_, QWidget* parent)
  : CastDBViewSplitter{SystemPath::PEFORMERS_DATABASE(), SystemPath::CastDBConnection(), getConfig(PathKey::PERFORMER_IMAGEHOST_LOCATE).toString(),
                       castDbSearchBar_, parent}
{}

CastDBViewSplitter::~CastDBViewSplitter() {
  Configuration().setValue("CastDBViewSplitter/LRSplitterState", saveState());
}

void CastDBViewSplitter::subscribe() {
  connect(m_sharedSelectionModel, &QItemSelectionModel::currentRowChanged, this, &CastDBViewSplitter::onCurrentRowChanged);

  const auto& castInst = CastDBActions::GetInst();
  connect(castInst.m_showCastsTableAct, &QAction::toggled, this, &CastDBViewSplitter::onShowCastTableView);
  connect(castInst.m_showCastsCardsListAct, &QAction::toggled, this, &CastDBViewSplitter::onShowCastListView);

  connect(castInst.INIT_DATABASE, &QAction::triggered, m_castDb, &DbManager::CreateDatabase);
  connect(castInst.OPEN_DB_WITH_LOCAL_APP, &QAction::triggered, m_castDb, &DbManager::onShowInFileSystemView);
  connect(_castDbSearchBar, &CastDatabaseSearchToolBar::whereClauseChanged, m_castDbModel, &CastDbModel::SetFilterAndSelect);
  connect(m_castTableView, &CustomTableView::searchSqlStatementChanged, this, [this](const QString& s) {
    _castDbSearchBar->SetWhereClause(s);
    m_castDbModel->SetFilterAndSelect(s);
  });
  connect(castInst.INIT_TABLE, &QAction::triggered, this, &CastDBViewSplitter::onInitATable);
  connect(castInst.DROP_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DROP); });
  connect(castInst.DELETE_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DELETE); });

  connect(castInst.APPEND_FROM_FILE_SYSTEM_STRUCTURE, &QAction::triggered, this, &CastDBViewSplitter::onLoadFromFileSystemStructure);
  connect(castInst.APPEND_FROM_PSON_FILES, &QAction::triggered, this, &CastDBViewSplitter::onLoadFromPsonDirectory);
}

void CastDBViewSplitter::onShowCastTableView(bool bShow) {
  m_castTableView->setHidden(!bShow);
}

void CastDBViewSplitter::onShowCastListView(bool bShow) {
  setConfig(CastKey::SHOW_CASTS_IN_CARD_VIEW, bShow);
  if (m_castListView == nullptr) {
    if (bShow) {
      m_castListView = new (std::nothrow) CastDBListView{m_castDbModel, m_sharedSelectionModel, this};
      CHECK_NULLPTR_RETURN_VOID(m_castListView)
      addWidget(m_castListView);
      const QByteArray stateBa = Configuration().value("CastDBViewSplitter/LRSplitterState").toByteArray();
      if (!stateBa.isEmpty()) {
        restoreState(stateBa);
      } else if (width() > 0) {
        // 首次出现时平分可视区域, 否则新窗格初始宽度为 0, 看起来像没显示出来。
        // 尚未布局时 width() 可能为 0, 此时交给 setSizes 的默认等分处理。
        const int w = width();
        const int half{w / 2};
        setSizes({half, w - half});
      }
    }
    return;
  }
  m_castListView->setHidden(!bShow);
}

void CastDBViewSplitter::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous) {
  if (!current.isValid()) {
    return;
  }
  if (m_castListView != nullptr) {
    m_castListView->scrollTo(current, QAbstractItemView::EnsureVisible);
  }
  if (m_castTableView != nullptr) {
    m_castTableView->scrollTo(current, QAbstractItemView::EnsureVisible);
  }
}

bool CastDBViewSplitter::onInitATable() {
  if (m_castDbModel->isDirty()) {
    LOG_W("Reject select to avoid data loss");
    return false;
  }

  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  if (!m_castDb->CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE)) {
    LOG_W("Table[%s] create failed.", qPrintable(DB_TABLE::PERFORMERS));
    return false;
  }
  m_castDbModel->setTable(DB_TABLE::PERFORMERS);
  if (!m_castDbModel->select()) {
    // select() 可能因各种原因失败（如表被意外删除）, 需要处理失败情况，例如记录日志或重置模型
    LOG_W("Failed to refresh model[%s] after Init a table: %s", qPrintable(DB_TABLE::PERFORMERS), qPrintable(m_castDbModel->lastError().text()));
    return false;
  }
  LOG_D("Table[%s] create succeed", qPrintable(DB_TABLE::PERFORMERS));
  return true;
}

bool CastDBViewSplitter::onDropDeleteTable(const DbManagerHelper::DropOrDeleteE dropOrDelete) {
  const QString cfmTitleText{QString::asprintf("Confirm %s?", DbManagerHelper::c_str(dropOrDelete))};
  const QString hintText{QString::asprintf("Operation[%s] on Table[%s] is not recoverable", DbManagerHelper::c_str(dropOrDelete), qPrintable(DB_TABLE::PERFORMERS))};
  const QString informativeText{"Drop(0)/Delete(1) [" + DB_TABLE::PERFORMERS + "] operation not recoverable"};

  if (!InputDialogHelper::YesOrCancelBox(QMessageBox::Icon::Warning, QIcon{}, cfmTitleText, hintText, informativeText, "")) {
    LOG_D("User cancel drop/delete table[%s]", qPrintable(DB_TABLE::PERFORMERS));
    return false;
  }
  int rmvedTableCnt = m_castDb->RmvTable(DB_TABLE::PERFORMERS, dropOrDelete);
  LOG_OE_P(rmvedTableCnt >= 0, "Drop/Delete", "Table[%s] %s count:%d", qPrintable(DB_TABLE::PERFORMERS), DbManagerHelper::c_str(dropOrDelete), rmvedTableCnt);
  if (rmvedTableCnt > 0) {
    if (dropOrDelete == DbManagerHelper::DropOrDeleteE::DELETE) {
      m_castDbModel->select();
    } else {
      m_castDbModel->setTable("");
    }
  }
  return rmvedTableCnt >= 0;
}

int CastDBViewSplitter::onLoadFromFileSystemStructure() {
  if (m_castDbModel->isDirty()) {
    LOG_ERR_NP("[Abort] Table dirty", "submit before load from file-system structure");
    return 0;
  }
  QString imageHostFullPath = m_castDbModel->rootPath();
  int succeedCnt = m_castDb->ReadFromImageHost(imageHostFullPath);
  LOG_OE_P(succeedCnt >= 0, "Load Performers", "count: %d", succeedCnt);
  if (succeedCnt > 0) {
    if (!m_castDbModel->select()) {
      // select() 可能因各种原因失败（如表被意外删除）, 需要处理失败情况，例如记录日志或重置模型
      LOG_W("Failed to refresh model after external insert: %s", qPrintable(m_castDbModel->lastError().text()));
    }
  }
  return succeedCnt;
}

int CastDBViewSplitter::onLoadFromPsonDirectory() {
  if (m_castDbModel->isDirty()) {
    LOG_ERR_NP("Table dirty", "submit before load pson");
    return 0;
  }
  QMessageBox::StandardButton stdCfmLoadFromPson = QMessageBox::StandardButton::No;
  const QString cfmTitleText{"CONFIRM Load from pson? (OVERRIDE NOT RECOVERABLE)"};
  const QString hintText{"Risk: records in database will be override if differs from local pson file."};
  if (!InputDialogHelper::YesOrCancelBox(QMessageBox::Icon::Warning, QIcon{}, cfmTitleText, hintText, "", "")) {
    LOG_INFO_NP("[Skip] User cancel load records from pson", "return");
    return 0;
  }
  QString imageHostFullPath = m_castDbModel->rootPath();
  int succeedCnt = m_castDb->LoadFromPsonFile(imageHostFullPath);
  LOG_OE_P(succeedCnt >= 0, "Load perfs from pJson", "[%s/*.pson] count:%d", qPrintable(imageHostFullPath), succeedCnt);
  if (succeedCnt > 0) {
    if (!m_castDbModel->select()) {
      // select() 可能因各种原因失败（如表被意外删除）, 需要处理失败情况，例如记录日志或重置模型
      LOG_W("Failed to refresh model after external insert: %s", qPrintable(m_castDbModel->lastError().text()));
    }
  }
  return succeedCnt;
}