#include "AlertSystem.h"
#include <QDesktopServices>
#include <QFileIconProvider>
#include <QHeaderView>
#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>

#include "Actions/FileLeafAction.h"
#include "Notificator.h"
#include "public/PublicVariable.h"

AlertSystem::AlertSystem(QWidget* parent)
    : QDialog{parent},
      m_failItemCnt{new QLabel("0", this)},
      m_recheckButtonBox{
          new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Ok | QDialogButtonBox::Retry, Qt::Orientation::Horizontal, this)} {
  m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Open)->setIcon(QIcon(":img/CONFIGURE"));
  m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Retry)->setText("Recheck");
  m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Retry)->setIcon(QIcon(":img/RELOAD_FROM_DISK"));
  m_alertModel->setRootPath("");
  m_alertsTable->setModel(m_alertModel);
  m_alertsTable->setEditTriggers(QAbstractItemView::EditTrigger::EditKeyPressed);

  auto* lo = new QVBoxLayout(this);
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
  RefreshWindowIcon();
}

void AlertSystem::hideEvent(QHideEvent* event) {
  g_fileLeafActions()._ALERT_ITEMS->setChecked(false);
  QDialog::hideEvent(event);
}

void AlertSystem::RefreshWindowIcon() {
  qInfo("Alert System RefreshWindowIcon");
  const int failsCnt = m_alertModel->failCount();
  setWindowIcon(QIcon(failsCnt > 0 ? ":img/ALERT_ACTIVE" : ":img/ALERT"));
  QString msg;
  if (failsCnt > 0) {
    msg = QString("<b><font color=\"#FF0000\">%1</font></b>").arg(failsCnt);
  } else {
    msg = QString("<b>%1</b>").arg(failsCnt);
  }
  m_failItemCnt->setText("Fail item(s): " + msg);
  setWindowTitle(QString("Alarm System | %1/%2 Fail(s)").arg(failsCnt).arg(m_alertModel->rowCount()));
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
    qDebug("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::critical("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Remember to reload", "don't forget it");
}

//#define __NAME__EQ__MAIN__ 1

#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  AlertSystem as;
  as.show();
  return a.exec();
}
#endif
