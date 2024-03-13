#include "LogView.h"
#include "PublicVariable.h"
#include "Tools/MessageOutput.h"

LogView::LogView(QWidget* parent) : QMainWindow{parent} {
  m_logToolBar->addAction(m_refreshLogs);
  addToolBar(m_logToolBar);

  m_logMenu->addAction(_COPY_NAME_AND_LINE);
  m_logMenu->addAction(m_refreshLogs);
  m_logTable->BindMenu(m_logMenu);

  m_logTable->setModel(m_logModel);
  setCentralWidget(m_logTable);

  m_typeAGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  setWindowTitle(tr("Log View"));
  setWindowIcon(QIcon(":/themes/FLOW_LOGS"));
  ReadSettings();

  subscribe();
  RefreshLogs();
}

void LogView::ReadSettings() {
  if (PreferenceSettings().contains("LOGTABLE_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("LOGTABLE_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

void LogView::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("LOGTABLE_GEOMETRY", saveGeometry());
  return QMainWindow::closeEvent(event);
}

void LogView::subscribe() {
  connect(m_refreshLogs, &QAction::triggered, this, &LogView::RefreshLogs);
  connect(_COPY_NAME_AND_LINE, &QAction::triggered, this, &LogView::CopyNameAndLineNo);
}

#include "Tools/MessageOutput.h"
void LogView::RefreshLogs() {
  MessageOutput::flush();
  m_logModel->_reloadLogFiles();
  m_logTable->scrollToBottom();
}

#include <QApplication>
#include <QClipboard>
void LogView::CopyNameAndLineNo() {
  const auto& curIndex = m_logTable->currentIndex();
  if (not curIndex.isValid()) {
    qDebug("Select a row and do it again");
    return;
  }
  const QString& info = m_logModel->getFileNameAndLineNo(curIndex);
  QApplication::clipboard()->setText(info);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

#include <QApplication>
int main(int argc, char* argv[]) {
  MessageOutput mo;

  QApplication a(argc, argv);
  LogView logView;
  logView.show();
  a.exec();
  return 0;
}
#endif
