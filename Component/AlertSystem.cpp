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

namespace ALERT_TABLE {
const QString Number = "ID";
const QString Name = "Name";
const QString Value = "Value";
const QString Note = "Note";

const QStringList ALERT_TABLE_HEADER{Number, Name, Value, Note};
const int NUMBER_INDEX = ALERT_TABLE_HEADER.indexOf(Number);
const int NAME_INDEX = ALERT_TABLE_HEADER.indexOf(Name);
const int VALUE_INDEX = ALERT_TABLE_HEADER.indexOf(Value);
const int NOTE_INDEX = ALERT_TABLE_HEADER.indexOf(Note);
}  // namespace ALERT_TABLE
using namespace ALERT_TABLE;

AlertSystem::AlertSystem(QWidget* parent)
    : QDialog{parent},
      m_failItemCnt{new QLabel("0", this)},
      m_alertsTable{new QTableWidget(0, ALERT_TABLE_HEADER.size(), this)},
      m_recheckButtonBox{new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Ok, Qt::Orientation::Horizontal, this)} {
  m_recheckButtonBox->button(QDialogButtonBox::StandardButton::Open)->setIcon(QIcon(":/themes/CONFIGURE"));
  for (int c = 0; c < ALERT_TABLE_HEADER.size(); ++c) {
    m_alertsTable->setHorizontalHeaderItem(c, new QTableWidgetItem(ALERT_TABLE_HEADER[c]));
  }
  m_alertsTable->horizontalHeader()->setSectionResizeMode(NUMBER_INDEX, QHeaderView::ResizeMode::ResizeToContents);
  m_alertsTable->horizontalHeader()->setSectionResizeMode(NAME_INDEX, QHeaderView::ResizeMode::ResizeToContents);
  m_alertsTable->horizontalHeader()->setSectionResizeMode(VALUE_INDEX, QHeaderView::ResizeMode::Interactive);
  m_alertsTable->horizontalHeader()->setStretchLastSection(true);
  m_alertsTable->setAlternatingRowColors(true);
  m_alertsTable->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
  m_alertsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  m_alertsTable->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked |
                                 QAbstractItemView::EditKeyPressed);  // only F2 works. QAbstractItemView::NoEditTriggers;
  m_alertsTable->setDragDropMode(QAbstractItemView::NoDragDrop);
  m_alertsTable->setContextMenuPolicy(Qt::CustomContextMenu);

  const auto fontSize = PreferenceSettings().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(m_alertsTable->font());
  defaultFont.setPointSize(fontSize);
  m_alertsTable->setFont(defaultFont);

  QString fileVal;
  int r = 0;
  QFileIconProvider iconProvider;
#ifdef _WIN32
  m_alertsTable->insertRow(r);
  auto fileKey = MemoryKey::WIN32_PERFORMERS_TABLE;
  fileVal = PreferenceSettings().value(fileKey.name, fileKey.v).toString();
  m_alertsTable->setItem(r, NUMBER_INDEX, new QTableWidgetItem(QString::number(r)));
  m_alertsTable->setItem(r, NAME_INDEX, new QTableWidgetItem(fileKey.name));
  m_alertsTable->setItem(r, VALUE_INDEX, new QTableWidgetItem(iconProvider.icon(QFileInfo(fileVal)), fileVal));
  m_alertsTable->setItem(r, NOTE_INDEX, new QTableWidgetItem("Used in Json editor. Provide the ability to filter performers out from giving string"));
  ++r;
  m_alertsTable->insertRow(r);
  fileKey = MemoryKey::WIN32_AKA_PERFORMERS;
  fileVal = PreferenceSettings().value(fileKey.name, fileKey.v).toString();
  m_alertsTable->setItem(r, NUMBER_INDEX, new QTableWidgetItem(QString::number(r)));
  m_alertsTable->setItem(r, NAME_INDEX, new QTableWidgetItem(fileKey.name));
  m_alertsTable->setItem(r, VALUE_INDEX, new QTableWidgetItem(iconProvider.icon(QFileInfo(fileVal)), fileVal));
  m_alertsTable->setItem(r, NOTE_INDEX, new QTableWidgetItem("Used in Quick Where Window to join where clause."));
  ++r;
  m_alertsTable->insertRow(r);
  fileKey = MemoryKey::WIN32_STANDARD_STUDIO_NAME;
  fileVal = PreferenceSettings().value(fileKey.name, fileKey.v).toString();
  m_alertsTable->setItem(r, NUMBER_INDEX, new QTableWidgetItem(QString::number(r)));
  m_alertsTable->setItem(r, NAME_INDEX, new QTableWidgetItem(fileKey.name));
  m_alertsTable->setItem(r, VALUE_INDEX, new QTableWidgetItem(iconProvider.icon(QFileInfo(fileVal)), fileVal));
  m_alertsTable->setItem(r, NOTE_INDEX, new QTableWidgetItem("Used in Json Editor to guest studio name."));
  ++r;
  m_alertsTable->insertRow(r);
  fileKey = MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH;
  fileVal = PreferenceSettings().value(fileKey.name, fileKey.v).toString();
  m_alertsTable->setItem(r, NUMBER_INDEX, new QTableWidgetItem(QString::number(r)));
  m_alertsTable->setItem(r, NAME_INDEX, new QTableWidgetItem(fileKey.name));
  m_alertsTable->setItem(r, VALUE_INDEX, new QTableWidgetItem(iconProvider.icon(QFileInfo(fileVal)), fileVal));
  m_alertsTable->setItem(r, NOTE_INDEX, new QTableWidgetItem("Used in reveal in terminal."));
  ++r;
  m_alertsTable->insertRow(r);
  auto folderKey = MemoryKey::WIN32_RUNLOG;
  fileVal = PreferenceSettings().value(folderKey.name, folderKey.v).toString();
  m_alertsTable->setItem(r, NUMBER_INDEX, new QTableWidgetItem(QString::number(r)));
  m_alertsTable->setItem(r, NAME_INDEX, new QTableWidgetItem(folderKey.name));
  m_alertsTable->setItem(r, VALUE_INDEX, new QTableWidgetItem(iconProvider.icon(QFileInfo(fileVal)), fileVal));
  m_alertsTable->setItem(r, NOTE_INDEX, new QTableWidgetItem("Used in log records when some file operation failed."));
  ++r;
#else
  m_alertsTable->insertRow(r);
  auto fileKey = MemoryKey::LINUX_PERFORMERS_TABLE;
  fileVal = PreferenceSettings().value(fileKey.name, fileKey.v).toString();
  m_alertsTable->setItem(r, NUMBER_INDEX, new QTableWidgetItem(QString::number(r)));
  m_alertsTable->setItem(r, NAME_INDEX, new QTableWidgetItem(fileKey.name));
  m_alertsTable->setItem(r, VALUE_INDEX, new QTableWidgetItem(iconProvider.icon(QFileInfo(fileVal)), fileVal));
  m_alertsTable->setItem(r, NOTE_INDEX, new QTableWidgetItem("Used in Json editor. Provide the ability to filter performers out from giving string"));
  ++r;
  m_alertsTable->insertRow(r);
  fileKey = MemoryKey::LINUX_AKA_PERFORMERS;
  fileVal = PreferenceSettings().value(fileKey.name, fileKey.v).toString();
  m_alertsTable->setItem(r, NUMBER_INDEX, new QTableWidgetItem(QString::number(r)));
  m_alertsTable->setItem(r, NAME_INDEX, new QTableWidgetItem(fileKey.name));
  m_alertsTable->setItem(r, VALUE_INDEX, new QTableWidgetItem(iconProvider.icon(QFileInfo(fileVal)), fileVal));
  m_alertsTable->setItem(r, NOTE_INDEX, new QTableWidgetItem("Used in Quick Where Window to join where clause."));
  ++r;
  m_alertsTable->insertRow(r);
  fileKey = MemoryKey::LINUX_STANDARD_STUDIO_NAME;
  fileVal = PreferenceSettings().value(fileKey.name, fileKey.v).toString();
  m_alertsTable->setItem(r, NUMBER_INDEX, new QTableWidgetItem(QString::number(r)));
  m_alertsTable->setItem(r, NAME_INDEX, new QTableWidgetItem(fileKey.name));
  m_alertsTable->setItem(r, VALUE_INDEX, new QTableWidgetItem(iconProvider.icon(QFileInfo(fileVal)), fileVal));
  m_alertsTable->setItem(r, NOTE_INDEX, new QTableWidgetItem("Used in Json Editor to guest studio name."));
  ++r;
  m_alertsTable->insertRow(r);
  fileKey = MemoryKey::LINUX_TERMINAL_OPEN_BATCH_FILE_PATH;
  fileVal = PreferenceSettings().value(fileKey.name, fileKey.v).toString();
  m_alertsTable->setItem(r, NUMBER_INDEX, new QTableWidgetItem(QString::number(r)));
  m_alertsTable->setItem(r, NAME_INDEX, new QTableWidgetItem(fileKey.name));
  m_alertsTable->setItem(r, VALUE_INDEX, new QTableWidgetItem(iconProvider.icon(QFileInfo(fileVal)), fileVal));
  m_alertsTable->setItem(r, NOTE_INDEX, new QTableWidgetItem("Used in reveal in terminal."));
  ++r;
  m_alertsTable->insertRow(r);
  auto folderKey = MemoryKey::LINUX_RUNLOG;
  fileVal = PreferenceSettings().value(folderKey.name, folderKey.v).toString();
  m_alertsTable->setItem(r, NUMBER_INDEX, new QTableWidgetItem(QString::number(r)));
  m_alertsTable->setItem(r, NAME_INDEX, new QTableWidgetItem(folderKey.name));
  m_alertsTable->setItem(r, VALUE_INDEX, new QTableWidgetItem(iconProvider.icon(QFileInfo(fileVal)), fileVal));
  m_alertsTable->setItem(r, NOTE_INDEX, new QTableWidgetItem("Used in log records when some file operation failed."));
  ++r;
#endif
  m_alertsTable->setSortingEnabled(true);  // don't set sort before table already set
  m_alertsTable->sortItems(NAME_INDEX, Qt::AscendingOrder);
  for (int row = 0; row < m_alertsTable->rowCount(); ++row) {
    InitLineColor(row);
  }
  RefreshWindowIcon();

  auto* lo = new QVBoxLayout(this);
  lo->addWidget(m_failItemCnt);
  lo->addWidget(m_alertsTable);
  lo->addWidget(m_recheckButtonBox);
  setLayout(lo);

  // cellChanged
  connect(m_alertsTable, &QTableWidget::cellChanged, this, &AlertSystem::on_cellChanged);
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

bool AlertSystem::isRowItemPass(const int row) const {
  const QString& keyName = m_alertsTable->item(row, NAME_INDEX)->text();
  const QString& keyValue = m_alertsTable->item(row, VALUE_INDEX)->text();
  bool isPass = false;
#ifdef _WIN32
  if (keyName == MemoryKey::WIN32_PERFORMERS_TABLE.name) {
    isPass = MemoryKey::WIN32_PERFORMERS_TABLE.checker(keyValue);
  } else if (keyName == MemoryKey::WIN32_AKA_PERFORMERS.name) {
    isPass = MemoryKey::WIN32_AKA_PERFORMERS.checker(keyValue);
  } else if (keyName == MemoryKey::WIN32_STANDARD_STUDIO_NAME.name) {
    isPass = MemoryKey::WIN32_STANDARD_STUDIO_NAME.checker(keyValue);
  } else if (keyName == MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH.name) {
    isPass = MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH.checker(keyValue);
  } else if (keyName == MemoryKey::WIN32_RUNLOG.name) {
    isPass = MemoryKey::WIN32_RUNLOG.checker(keyValue);
  }
#else
  if (keyName == MemoryKey::LINUX_PERFORMERS_TABLE.name) {
    isPass = MemoryKey::LINUX_PERFORMERS_TABLE.checker(keyValue);
  } else if (keyName == MemoryKey::LINUX_AKA_PERFORMERS.name) {
    isPass = MemoryKey::LINUX_AKA_PERFORMERS.checker(keyValue);
  } else if (keyName == MemoryKey::LINUX_STANDARD_STUDIO_NAME.name) {
    isPass = MemoryKey::LINUX_STANDARD_STUDIO_NAME.checker(keyValue);
  } else if (keyName == MemoryKey::LINUX_TERMINAL_OPEN_BATCH_FILE_PATH.name) {
    isPass = MemoryKey::LINUX_TERMINAL_OPEN_BATCH_FILE_PATH.checker(keyValue);
  } else if (keyName == MemoryKey::LINUX_RUNLOG.name) {
    isPass = MemoryKey::LINUX_RUNLOG.checker(keyValue);
  }
#endif
  return isPass;
}

bool AlertSystem::InitLineColor(const int row) {
  bool isPass = isRowItemPass(row);
  m_checkItemStatus[m_alertsTable->item(row, NAME_INDEX)->text()] = isPass;
  m_alertsTable->item(row, NAME_INDEX)->setIcon(isPass ? QIcon(":/themes/PASS") : QIcon(":/themes/FAILED"));
  if (isPass) {
    return isPass;
  }
  for (int c = 0; c != ALERT_TABLE_HEADER.size(); ++c) {
    m_alertsTable->item(row, c)->setBackground(TOMATO_COLOR);
  }
  return isPass;
}

bool AlertSystem::RefreshLineColor(const int row) {
  bool isPass = isRowItemPass(row);
  const QString& key = m_alertsTable->item(row, NAME_INDEX)->text();
  const QString& val = m_alertsTable->item(row, VALUE_INDEX)->text();
  m_checkItemStatus[key] = isPass;
  if (isPass and PreferenceSettings().contains(key)) {
    PreferenceSettings().setValue(key, val);  // update it
    Notificator::information("Good job! Value Fixed", key);
  } else {
    Notificator::information("Oops! Value Wrong", key);
  }
  QColor color = isPass ? LIGHT_GREEN_COLOR : TOMATO_COLOR;
  m_alertsTable->item(row, NAME_INDEX)->setIcon(isPass ? QIcon(":/themes/PASS") : QIcon(":/themes/FAILED"));
  for (int c = 0; c != ALERT_TABLE_HEADER.size(); ++c) {
    m_alertsTable->item(row, c)->setBackground(color);
  }
  return isPass;
}

bool AlertSystem::on_cellChanged(const int row, const int column) {
  if (column != VALUE_INDEX) {
    return true;
  }
  m_alertsTable->blockSignals(true);
  RefreshLineColor(row);
  m_alertsTable->blockSignals(false);
  RefreshWindowIcon();
  return true;
}

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
