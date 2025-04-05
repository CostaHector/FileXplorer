#include "LongPathFolderRenamer.h"
#include "FileOperation/FileOperatorPub.h"
#include "Component/NotificatorFrame.h"
#include "public/PublicVariable.h"
#include "public/UndoRedo.h"
#include <QLabel>

LongPathFolderRenamer::LongPathFolderRenamer(QWidget* parent) : QDialog{parent}, windowTitleFormat{"%1 folder(s) path too long(now: %2)"} {
  setWindowFlag(Qt::WindowMaximizeButtonHint);  // WindowMinMaxButtonsHint;

  m_statusTE = new QPlainTextEdit();
  m_preTE = new QPlainTextEdit();
  m_oldTE = new QPlainTextEdit();
  m_newTE = new QPlainTextEdit();
  m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);

  m_buttonBox->setOrientation(Qt::Orientation::Horizontal);
  m_buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet(SUBMIT_BTN_STYLE);
  m_buttonBox->button(QDialogButtonBox::Help)->setText("See commands...");

  m_dropSectionLE = new QLineEdit("-1", this);
  m_maxPathLengthLE = new QLineEdit("260", this);
  m_dropControlBar = new QToolBar{"Drop Control Toolbar", this};
  m_dropControlBar->addWidget(new QLabel{"Drop Section:"});
  m_dropControlBar->addWidget(m_dropSectionLE);
  m_dropControlBar->addWidget(new QLabel{"Max path length:"});
  m_dropControlBar->addWidget(m_maxPathLengthLE);

  m_statusTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_statusTE->setReadOnly(true);
  m_statusTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_statusTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_preTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_preTE->setReadOnly(true);
  m_preTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_preTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_oldTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_oldTE->setReadOnly(true);
  m_oldTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_oldTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_newTE->setWordWrapMode(QTextOption::WrapMode::NoWrap);
  m_newTE->setReadOnly(false);
  m_newTE->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  m_newTE->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_nameEditLayout = new QHBoxLayout;
  m_nameEditLayout->addWidget(m_statusTE);
  m_nameEditLayout->addWidget(m_preTE);
  m_nameEditLayout->addWidget(m_oldTE);
  m_nameEditLayout->addWidget(m_newTE);

  m_nameEditLayout->setStretch(0, 1);
  m_nameEditLayout->setStretch(1, 2);
  m_nameEditLayout->setStretch(2, 8);
  m_nameEditLayout->setStretch(3, 8);
  m_nameEditLayout->setSpacing(0);
  m_nameEditLayout->setContentsMargins(0, 0, 0, 0);

  m_mainLayout = new QVBoxLayout();
  m_mainLayout->addWidget(m_dropControlBar);
  m_mainLayout->addLayout(m_nameEditLayout);
  m_mainLayout->addWidget(m_buttonBox);

  setLayout(m_mainLayout);
  Subscribe();

  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/LONG_PATH_FINDER"));
  ReadSettings();
}

void LongPathFolderRenamer::ReadSettings() {
  if (PreferenceSettings().contains("LONG_PATH_FOLDER_RENAMER_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("LONG_PATH_FOLDER_RENAMER_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

void LongPathFolderRenamer::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("LONG_PATH_FOLDER_RENAMER_GEOMETRY", saveGeometry());
  return QDialog::closeEvent(event);
}

void LongPathFolderRenamer::Subscribe() {
  connect(m_newTE->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](const int position) {
    m_statusTE->verticalScrollBar()->setValue(position);
    m_preTE->verticalScrollBar()->setValue(position);
    m_oldTE->verticalScrollBar()->setValue(position);
    m_newTE->verticalScrollBar()->setValue(position);
  });
  connect(m_dropSectionLE, &QLineEdit::textEdited, this, &LongPathFolderRenamer::DropSectionChanged);

  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, [this]() { onApply(false, true); });
  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Help), &QPushButton::clicked, this, [this]() { onApply(true, false); });
  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &LongPathFolderRenamer::close);
}

auto LongPathFolderRenamer::InitTextContent(const QString& p) -> void {
  m_lpf(p);
  m_lpf.CheckTooLongPathCount();
  m_statusTE->setPlainText(m_lpf.m_status);
  m_preTE->setPlainText(m_lpf.pres.join('\n'));
  m_oldTE->setPlainText(m_lpf.olds.join('\n'));
  m_newTE->setPlainText(m_lpf.news.join('\n'));
  setWindowTitle(windowTitleFormat.arg(m_lpf.pres.size()).arg(m_lpf.StillTooLongPathCount()));
}

void LongPathFolderRenamer::UpdateStatus(const QStringList& news) {
  m_lpf.news = news;
  m_lpf.CheckTooLongPathCount();
  m_statusTE->setPlainText(m_lpf.m_status);
  setWindowTitle(windowTitleFormat.arg(m_lpf.pres.size()).arg(m_lpf.StillTooLongPathCount()));
}

auto LongPathFolderRenamer::DropSectionChanged(const QString& newDropSectionStr) -> void {
  bool isInt = false;
  int newDropSection = newDropSectionStr.toInt(&isInt);
  if (!isInt || newDropSection >= 0) {
    qDebug("invalid drop section index[%s]", qPrintable(newDropSectionStr));
    return;
  }
  m_lpf.SetDropSectionWhenTooLong(newDropSection);
  m_lpf.CheckTooLongPathCount();
  m_statusTE->setPlainText(m_lpf.m_status);
  m_newTE->setPlainText(m_lpf.news.join('\n'));
  setWindowTitle(windowTitleFormat.arg(m_lpf.pres.size()).arg(m_lpf.StillTooLongPathCount()));
}

auto LongPathFolderRenamer::onApply(const bool isOnlyHelp, const bool isInterative) -> bool {
  const QStringList& newCompleteNameList = m_newTE->toPlainText().split('\n');
  if (newCompleteNameList.size() != m_lpf.news.size()) {
    qWarning("Don't remove/add lines, before[%d], after[%d]", m_lpf.news.size(), newCompleteNameList.size());
    Notificator::warning("Don't remove/add lines", QString("before:%1 -> after:%2").arg(m_lpf.news.size()).arg(newCompleteNameList.size()));
    return false;
  }
  UpdateStatus(newCompleteNameList);
  const auto tooLongCnt = m_lpf.StillTooLongPathCount();
  if (tooLongCnt != 0) {
    qWarning("There are still %d(s) path too long, edit first", tooLongCnt);
    Notificator::warning("Edit them first", QString("There are still %1 path(s) too long").arg(tooLongCnt));
    return false;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE cmds;
  for (int i = 0; i < newCompleteNameList.size(); ++i) {
    if (newCompleteNameList[i].isEmpty()) {
      qWarning("new name of [%s] is empty", qPrintable(m_lpf.olds[i]));
      Notificator::warning("Empty name find", QString("new name of [%1] is empty").arg(m_lpf.olds[i]));
      return false;
    }
    if (m_lpf.olds[i] == newCompleteNameList[i]) {
      continue;
    }
    cmds.append(ACMD{RENAME, {m_lpf.pres[i], m_lpf.olds[i], m_lpf.pres[i], newCompleteNameList[i]}});
  }
  FileOperatorType::BATCH_COMMAND_LIST_TYPE reversedcmds{cmds.crbegin(), cmds.crend()};  // rename files first, than its folders;

  if (isOnlyHelp) {
    if (m_commandsPreview == nullptr) {
      m_commandsPreview = new QPlainTextEdit;
    }
    for (const ACMD& cmd : reversedcmds) {
      m_commandsPreview->appendPlainText(cmd.lst.join('\t'));
    }
    m_commandsPreview->setWindowTitle(QString("Rename names | Total %1 Command(s)").arg(reversedcmds.size()));
    m_commandsPreview->setMinimumWidth(1024);
    m_commandsPreview->raise();
    m_commandsPreview->show();
    return true;
  }
  bool isAllSuccess = g_undoRedo.Do(reversedcmds);
  if (isInterative) {
    if (isAllSuccess) {
      qInfo("Batch rename ok %d command(s).", reversedcmds.size());
      Notificator::goodNews("Batch rename ok", QString("%1 command(s).").arg(reversedcmds.size()));
    }
  }
  close();
  return isAllSuccess;
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  LongPathFolderRenamer lpfr;
  lpfr.InitTextContent("E:/115/0805");
  lpfr.show();
  return a.exec();
}
#endif
