#include "AlertSystem.h"
#include <QDesktopServices>
#include <QFileIconProvider>
#include <QHeaderView>
#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>

#include "NotificatorFrame.h"
#include "PublicVariable.h"
using namespace STATUS_COLOR;

AlertSystem::AlertSystem(QWidget* parent)
    : QDialog{parent},
      m_failItemCnt{new QLabel("0", this)},
      m_recheckButtonBox{new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Ok, Qt::Orientation::Horizontal, this)} {
  m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Open)->setIcon(QIcon(":/themes/CONFIGURE"));

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
}

void AlertSystem::RefreshWindowIcon() {
  int failsCnt = m_checkItemStatus.values().count(false);
  setWindowIcon(QIcon(failsCnt > 0 ? ":/themes/ALERT_ACTIVE" : ":/themes/ALERT"));
  QString msg;
  if (failsCnt > 0) {
    msg = QString("<b><font color=\"#FF0000\">%1</font></b>").arg(failsCnt);
  } else {
    msg = QString("<b>%1</b>").arg(failsCnt);
  }
  m_failItemCnt->setText("Fail item(s): " + msg);
  setWindowTitle(QString("Alarm System | %1/%2 Fail(s)").arg(failsCnt).arg(m_checkItemStatus.size()));
}

// bool AlertSystem::InitLineColor(const int row) {
//   bool isPass = isRowItemPass(row);
//   m_checkItemStatus[m_alertsTable->item(row, NAME_INDEX)->text()] = isPass;
//   m_alertsTable->item(row, NAME_INDEX)->setIcon(isPass ? QIcon(":/themes/PASS") : QIcon(":/themes/FAILED"));
//   if (isPass) {
//     return isPass;
//   }
//   for (int c = 0; c != ALERT_TABLE_HEADER.size(); ++c) {
//     m_alertsTable->item(row, c)->setBackground(TOMATO_COLOR);
//   }
//   return isPass;
// }

// bool AlertSystem::RefreshLineColor(const int row) {
//   bool isPass = isRowItemPass(row);
//   const QString& key = m_alertsTable->item(row, NAME_INDEX)->text();
//   const QString& val = m_alertsTable->item(row, VALUE_INDEX)->text();
//   m_checkItemStatus[key] = isPass;
//   if (isPass and PreferenceSettings().contains(key)) {
//     PreferenceSettings().setValue(key, val);  // update it
//     Notificator::information("Good job! Value Fixed", key);
//   } else {
//     Notificator::information("Oops! Value Wrong", key);
//   }
//   QColor color = isPass ? LIGHT_GREEN_COLOR : TOMATO_COLOR;
//   m_alertsTable->item(row, NAME_INDEX)->setIcon(isPass ? QIcon(":/themes/PASS") : QIcon(":/themes/FAILED"));
//   for (int c = 0; c != ALERT_TABLE_HEADER.size(); ++c) {
//     m_alertsTable->item(row, c)->setBackground(color);
//   }
//   return isPass;
// }

bool AlertSystem::on_cellDoubleClicked(const QModelIndex& clickedIndex) const {
  const QString& path = m_alertModel->filePath(clickedIndex);
  return QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

// bool AlertSystem::on_cellChanged(const int row, const int column) {
//   if (column != VALUE_INDEX) {
//     return true;
//   }
//   m_alertsTable->blockSignals(true);
//   RefreshLineColor(row);
//   m_alertsTable->blockSignals(false);
//   RefreshWindowIcon();
//   return true;
// }

void AlertSystem::onEditPreferenceSetting() const {
  QString fileAbsPath = PreferenceSettings().fileName();
  if (not QFile::exists(fileAbsPath)) {
    qDebug("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
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
