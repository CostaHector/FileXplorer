#include "AlertSystem.h"
#include "Notificator.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "FileLeafAction.h"

#include <QDesktopServices>
#include <QFileIconProvider>
#include <QHeaderView>
#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>

AlertSystem::AlertSystem(QWidget* parent)
    : QDialog{parent}  //
{
  m_failItemCnt = new (std::nothrow) QLabel{"", this};
  CHECK_NULLPTR_RETURN_VOID(m_failItemCnt)
  m_failItemCnt->setFont(StyleSheet::TEXT_EDIT_FONT);

  m_alertModel = new (std::nothrow) PreferenceModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_alertModel)
  m_alertsTable = new (std::nothrow) CustomTableView{"ALERT_SYSTEM", this};
  CHECK_NULLPTR_RETURN_VOID(m_alertsTable)
  m_alertModel->setRootPath("");
  m_alertsTable->setModel(m_alertModel);
  m_alertsTable->setEditTriggers(QAbstractItemView::EditTrigger::EditKeyPressed);

  m_recheckButtonBox = new (std::nothrow) QDialogButtonBox{QDialogButtonBox::Open | QDialogButtonBox::Ok | QDialogButtonBox::Retry, Qt::Orientation::Horizontal, this};
  CHECK_NULLPTR_RETURN_VOID(m_recheckButtonBox)
  m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Open)->setIcon(QIcon(":img/CONFIGURE"));
  m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Retry)->setText("Recheck");
  m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Retry)->setIcon(QIcon(":img/RELOAD_FROM_DISK"));

  auto* lo = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(lo)
  lo->addWidget(m_failItemCnt);
  lo->addWidget(m_alertsTable);
  lo->addWidget(m_recheckButtonBox);
  setLayout(lo);

  connect(m_alertsTable, &QTableView::doubleClicked, this, &AlertSystem::on_cellDoubleClicked);
  connect(m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &QDialog::accept);
  connect(m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Open), &QPushButton::clicked, this, &AlertSystem::onEditPreferenceSetting);
  connect(m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Retry), &QPushButton::clicked, this, &AlertSystem::RefreshWindowIcon);

  connect(m_alertModel, &QAbstractItemModel::dataChanged, this, &AlertSystem::RefreshWindowIcon);

  ReadSettings();
  setWindowIcon(QIcon{":img/SETTINGS"});
  RefreshWindowIcon();
}

void AlertSystem::ReadSettings() {
  if (PreferenceSettings().contains("ALERT_SYSTEM_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("ALERT_SYSTEM_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  m_alertsTable->InitTableView();
}

void AlertSystem::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void AlertSystem::hideEvent(QHideEvent* event) {
  g_fileLeafActions()._SETTINGS->setChecked(false);
  QDialog::hideEvent(event);
}

void AlertSystem::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("ALERT_SYSTEM_GEOMETRY", saveGeometry());
  return QDialog::closeEvent(event);
}

void AlertSystem::RefreshWindowIcon() {
  const int totalCnt{m_alertModel->rowCount()};
  const int failsCnt = m_alertModel->failCount();

  QString msg{R"(<b>)"};
  QString plainText;
  if (failsCnt != 0) {
    msg += R"(<font color="#FF0000">)";
    plainText = QString{"%1 in %2 setting(s) error"}.arg(failsCnt).arg(totalCnt);
    msg += plainText;
    msg += R"(</font>)";
  } else {
    plainText = QString("All %1 setting passed").arg(totalCnt);
    msg += plainText;
  }
  msg += R"(</b>)";
  m_failItemCnt->setText(msg);
  setWindowTitle("Alert System | " + plainText);
}

bool AlertSystem::on_cellDoubleClicked(const QModelIndex& clickedIndex) const {
  const QString& path = m_alertModel->filePath(clickedIndex);
  const auto ret = QDesktopServices::openUrl(QUrl::fromLocalFile(path));
  Notificator::information(QString("Try open [%1]:").arg(path), QString::number(ret));
  return ret;
}

void AlertSystem::onEditPreferenceSetting() const {
  QString fileAbsPath = PreferenceSettings().fileName();
  if (!QFile::exists(fileAbsPath)) {
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Remember to reload", "don't forget it");
}

// #define __NAME__EQ__MAIN__ 1

#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  AlertSystem as;
  as.show();
  return a.exec();
}
#endif
