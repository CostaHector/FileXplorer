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
#include <QKeyEvent>
#include <QFile>

constexpr const char* STATISTIC_LABEL_ALL_PASSED_TEMPLATE{"All Passed"};
constexpr const char* STATISTIC_LABEL_PARTIAL_FAILED_TEMPLATE{R"(<font color="#FF0000">Failed: %d</font>)"};

ConfigsMgr::ConfigsMgr(QWidget* parent)
  : QDialog{parent} {
  m_helpToolBar = new (std::nothrow) QToolBar{this};

  m_failedCountLabel = new (std::nothrow) QLabel{this};
  CHECK_NULLPTR_RETURN_VOID(m_failedCountLabel);
  m_helpToolBar->addWidget(m_failedCountLabel);
  m_recheckAction = m_helpToolBar->addAction(QIcon(":img/REFRESH_THIS_PATH"), "Re-check");
  m_recheckAction->setToolTip("Re-check configuration items for pass/fail status");
  m_helpToolBar->addSeparator();

  m_searchLineEdit = new (std::nothrow) QLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchLineEdit);
  m_searchLineEdit->setClearButtonEnabled(true);
  m_searchLineEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_searchAction = m_searchLineEdit->addAction(QIcon(":img/FILE_SYSTEM_FILTER"), QLineEdit::LeadingPosition);
  m_helpToolBar->addWidget(m_searchLineEdit);

  m_cfgsTable = new (std::nothrow) ConfigsTableView{"CONFIGS_TABLE", this};
  CHECK_NULLPTR_RETURN_VOID(m_cfgsTable);

  m_dlgBtnBox = new (std::nothrow) QDialogButtonBox{QDialogButtonBox::Open | QDialogButtonBox::Ok, Qt::Orientation::Horizontal, this};
  CHECK_NULLPTR_RETURN_VOID(m_dlgBtnBox);

  auto* lo = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(lo)
  lo->addWidget(m_helpToolBar);
  lo->addWidget(m_cfgsTable);
  lo->addWidget(m_dlgBtnBox);

  subscribe();

  setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
  setWindowTitle(g_fileLeafActions()._SETTINGS->text());
  setWindowIcon(g_fileLeafActions()._SETTINGS->icon());
  RefreshFailedCountLabel();
}

void ConfigsMgr::subscribe() {
  if (QPushButton* pOkBtn = m_dlgBtnBox->button(QDialogButtonBox::StandardButton::Ok)) {
    pOkBtn->setIcon(QIcon(":img/SAVED"));
    pOkBtn->setShortcut(QKeySequence(Qt::Key::Key_F10));
    StyleSheet::UpdateApplyPushButton(pOkBtn);
  }

  if (QPushButton* pOpen = m_dlgBtnBox->button(QDialogButtonBox::StandardButton::Open)) {
    pOpen->setText("Edit");
    pOpen->setIcon(QIcon(":img/CONFIGURE"));
    pOpen->setToolTip("Edit in config file directly");
    connect(pOpen, &QPushButton::clicked, this, &ConfigsMgr::onEditPreferenceSetting);
  }

  connect(m_dlgBtnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

  connect(m_searchLineEdit, &QLineEdit::returnPressed, this, &ConfigsMgr::onStartFilter);
  connect(m_searchAction, &QAction::triggered, this, &ConfigsMgr::onStartFilter);

  connect(m_recheckAction, &QAction::triggered, this, &ConfigsMgr::RefreshFailedCountLabel);
  connect(m_cfgsTable, &ConfigsTableView::modelCfgFailedCountChanged, this, &ConfigsMgr::UpdateFailedCountLabel);
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

QWidget* ConfigsMgr::focusWidgetCore(ConfigsMgr* self) { // test-usage only
  CHECK_NULLPTR_RETURN_NULLPTR(self);
  return self->focusWidget();
}

void ConfigsMgr::keyPressEvent(QKeyEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  // Return/Enter Eater/Consumer
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    QWidget* focused = focusWidgetCore(this);
    if (focused == m_searchLineEdit) {
      event->ignore();
      return;
    }
  }
  QDialog::keyPressEvent(event);
}

void ConfigsMgr::UpdateFailedCountLabel(int newFailedCount) {
  if (newFailedCount == 0) {
    m_failedCountLabel->setText(STATISTIC_LABEL_ALL_PASSED_TEMPLATE);
  } else {
    m_failedCountLabel->setText(QString::asprintf(STATISTIC_LABEL_PARTIAL_FAILED_TEMPLATE, newFailedCount));
  }
}

void ConfigsMgr::RefreshFailedCountLabel() {
  const int failsCnt = m_cfgsTable->GetFailedCnt();
  UpdateFailedCountLabel(failsCnt);
}

bool ConfigsMgr::onEditPreferenceSetting() const {
  const QString iniFileAbsPath = Configuration().fileName();
  if (!QFile::exists(iniFileAbsPath)) {
    LOG_ERR_P("[Failed] Cannot edit", ".ini file[%s] not found", qPrintable(iniFileAbsPath));
    return false;
  }
  return FileTool::OpenLocalFile(iniFileAbsPath);
}

void ConfigsMgr::onStartFilter() {
  m_cfgsTable->setFilter(m_searchLineEdit->text());
}
