#include "ConfigsTable.h"
#include "NotificatorMacro.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "FileLeafAction.h"
#include "PublicTool.h"

#include <QDialogButtonBox>
#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>

ConfigsTable::ConfigsTable(QWidget* parent)
  : QDialog{parent} {
  setFont(StyleSheet::TEXT_EDIT_FONT);

  m_failItemCnt = new (std::nothrow) QLabel{"Configs status here", this};
  CHECK_NULLPTR_RETURN_VOID(m_failItemCnt)
  m_alertModel = new (std::nothrow) ConfigsModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_alertModel)
  m_alertModel->setRootPath("");

  m_alertsTable = new (std::nothrow) CustomTableView{"CONFIGS_TABLE", this};
  CHECK_NULLPTR_RETURN_VOID(m_alertsTable)
  m_alertsTable->setModel(m_alertModel);
  m_alertsTable->setEditTriggers(QAbstractItemView::EditTrigger::EditKeyPressed);

  auto* dlgBtnBox = new (std::nothrow) QDialogButtonBox{QDialogButtonBox::Open | QDialogButtonBox::Ok | QDialogButtonBox::Retry, Qt::Orientation::Horizontal, this};
  CHECK_NULLPTR_RETURN_VOID(dlgBtnBox)

  auto* pOk = dlgBtnBox->button(QDialogButtonBox::StandardButton::Ok);
  pOk->setShortcut(QKeySequence(Qt::Key::Key_F10));
  pOk->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);
  auto* pOpen = dlgBtnBox->button(QDialogButtonBox::StandardButton::Open);
  pOpen->setText("Edit");
  pOpen->setIcon(QIcon(":img/CONFIGURE"));
  pOpen->setToolTip("Edit in config file directly");
  auto* pRetry = dlgBtnBox->button(QDialogButtonBox::StandardButton::Retry);
  pRetry->setText("Recheck");
  pRetry->setIcon(QIcon(":img/RELOAD_FROM_DISK"));

  auto* lo = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(lo)
  lo->addWidget(m_failItemCnt);
  lo->addWidget(m_alertsTable);
  lo->addWidget(dlgBtnBox);
  setLayout(lo);

  connect(m_alertsTable, &QTableView::doubleClicked, this, &ConfigsTable::on_cellDoubleClicked);
  connect(m_alertModel, &QAbstractItemModel::dataChanged, this, &ConfigsTable::RefreshWindowIcon);

  connect(pOk, &QPushButton::clicked, this, &QDialog::accept);
  connect(pOpen, &QPushButton::clicked, this, &ConfigsTable::onEditPreferenceSetting);
  connect(pRetry, &QPushButton::clicked, this, &ConfigsTable::RefreshWindowIcon);

  ReadSettings();

  setWindowFlag(Qt::WindowMaximizeButtonHint);
  setWindowTitle("Configs Table");
  setWindowIcon(QIcon{":img/SETTINGS"});
  RefreshWindowIcon();
}

void ConfigsTable::ReadSettings() {
  m_alertsTable->InitTableView();
}

void ConfigsTable::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void ConfigsTable::hideEvent(QHideEvent* event) {
  g_fileLeafActions()._SETTINGS->setChecked(false);
  QDialog::hideEvent(event);
}

void ConfigsTable::closeEvent(QCloseEvent* event) {
  return QDialog::closeEvent(event);
}

void ConfigsTable::RefreshWindowIcon() {
  const int totalCnt{m_alertModel->rowCount()};
  const int failsCnt{m_alertModel->failCount()};
  QString msg;
  msg += R"(<b>)";
  if (failsCnt != 0) {
    msg += R"(<font color="#FF0000">)";
    msg += QString{"%1 in %2 setting(s) error"}.arg(failsCnt).arg(totalCnt);
    msg += R"(</font>)";
  } else {
    msg += QString("All %1 setting passed").arg(totalCnt);
  }
  msg += R"(</b>)";
  m_failItemCnt->setText(msg);
}

bool ConfigsTable::on_cellDoubleClicked(const QModelIndex& clickedIndex) const {
  const QString& path = m_alertModel->filePath(clickedIndex);
  const QUrl url{QUrl::fromLocalFile(path)};
  if (!url.isLocalFile()) {
    LOG_INFO_P("[Skip] current row is not a path", "row:%d, contents:%s", clickedIndex.row(), qPrintable(path));
    return false;
  }
  return FileTool::OpenLocalFile(path);
}

void ConfigsTable::onEditPreferenceSetting() const {
  const QString iniFileAbsPath = Configuration().fileName();
  if (!QFile::exists(iniFileAbsPath)) {
    LOG_ERR_P("[Failed] Cannot edit", ".ini file[%s] not found", qPrintable(iniFileAbsPath));
    return;
  }
  FileTool::OpenLocalFile(iniFileAbsPath);
}
