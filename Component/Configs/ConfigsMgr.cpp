#include "ConfigsMgr.h"
#include "NotificatorMacro.h"
#include "Configuration.h"
#include "FileLeafAction.h"
#include "FileTool.h"
#include "StyleSheet.h"
#include "PublicMacro.h"

#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>

ConfigsMgr::ConfigsMgr(QWidget* parent)
  : QDialog{parent} {
  m_failItemCnt = new (std::nothrow) QLabel{"Configs status here", this};
  CHECK_NULLPTR_RETURN_VOID(m_failItemCnt);

  m_alertsTable = new (std::nothrow) ConfigsTableView{"CONFIGS_TABLE", this};
  CHECK_NULLPTR_RETURN_VOID(m_alertsTable);

  constexpr QDialogButtonBox::StandardButtons stdBtns{QDialogButtonBox::Open | QDialogButtonBox::Ok | QDialogButtonBox::Retry};
  m_dlgBtnBox = new (std::nothrow) QDialogButtonBox{stdBtns, Qt::Orientation::Horizontal, this};
  CHECK_NULLPTR_RETURN_VOID(m_dlgBtnBox);

  auto* lo = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(lo)
  lo->addWidget(m_failItemCnt);
  lo->addWidget(m_alertsTable);
  lo->addWidget(m_dlgBtnBox);

  subscribe();

  setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
  setWindowTitle("Configs Table");
  setWindowIcon(QIcon{":img/SETTINGS"});
  RefreshWindowIcon();
}

void ConfigsMgr::subscribe() {
  if (QPushButton* pOkBtn = m_dlgBtnBox->button(QDialogButtonBox::StandardButton::Ok)) {
    pOkBtn->setShortcut(QKeySequence(Qt::Key::Key_F10));
    pOkBtn->setIcon(QIcon(":img/SAVED"));
    StyleSheet::UpdateApplyPushButton(pOkBtn);
    connect(pOkBtn, &QPushButton::clicked, this, &QDialog::accept);
  }

  if (QPushButton* pOpen = m_dlgBtnBox->button(QDialogButtonBox::StandardButton::Open)) {
    pOpen->setText("Edit");
    pOpen->setIcon(QIcon(":img/CONFIGURE"));
    pOpen->setToolTip("Edit in config file directly");
    connect(pOpen, &QPushButton::clicked, this, &ConfigsMgr::onEditPreferenceSetting);
  }

  if (QPushButton* pRetry = m_dlgBtnBox->button(QDialogButtonBox::StandardButton::Retry)) {
    pRetry->setText("Recheck");
    pRetry->setIcon(QIcon(":JsonEditor/RELOAD_FROM_DISK"));
    connect(pRetry, &QPushButton::clicked, this, &ConfigsMgr::RefreshWindowIcon);
  }

  connect(m_alertsTable, &ConfigsTableView::modelDataChanged, this, &ConfigsMgr::RefreshWindowIcon);
}

void ConfigsMgr::showEvent(QShowEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void ConfigsMgr::hideEvent(QHideEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  g_fileLeafActions()._SETTINGS->setChecked(false);
  QDialog::hideEvent(event);
}

void ConfigsMgr::RefreshWindowIcon() {
  int failsCnt{0}, totalCnt{0};
  std::tie(failsCnt, totalCnt) = m_alertsTable->GetStatistics();

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

bool ConfigsMgr::onEditPreferenceSetting() const {
  const QString iniFileAbsPath = Configuration().fileName();
  if (!QFile::exists(iniFileAbsPath)) {
    LOG_ERR_P("[Failed] Cannot edit", ".ini file[%s] not found", qPrintable(iniFileAbsPath));
    return false;
  }
  return FileTool::OpenLocalFile(iniFileAbsPath);
}
