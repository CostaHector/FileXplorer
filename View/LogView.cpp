#include "LogView.h"
#include "Actions/LogViewActions.h"
#include "PublicVariable.h"
#include "Tools/MessageOutput.h"

LogView::LogView(QWidget* parent) : QMainWindow{parent} {
  m_logMenu->addAction(g_logAg()._COPY_LOCATE_INFO);
  m_logMenu->addAction(g_logAg()._REFRESH_LOG);
  m_logMenu->addSeparator();
  m_logMenu->addAction(g_logAg()._REVEAL_LOGS_FILE);
  m_logMenu->addMenu(g_logAg().getLogsTypeMenu());

  m_logTypeToolbar = g_logAg().getLogsTypeToolBar();
  m_logToolBar->addWidget(m_logTypeToolbar);
  m_logToolBar->addSeparator();
  m_logToolBar->addAction(g_logAg()._COPY_LOCATE_INFO);
  m_logToolBar->addAction(g_logAg()._REFRESH_LOG);
  m_logToolBar->addSeparator();
  m_logToolBar->addAction(g_logAg()._REVEAL_LOGS_FILE);
  addToolBar(m_logToolBar);

  m_logTable->BindMenu(m_logMenu);

  m_logProxyModel->setSourceModel(m_logModel);
  m_logTable->setModel(m_logProxyModel);
  m_logTable->InitTableView();

  m_logViewSplitter->addWidget(m_logTable);
  m_logViewSplitter->addWidget(m_logDetails);

  setCentralWidget(m_logViewSplitter);

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
  m_logViewSplitter->restoreState(PreferenceSettings().value("LOG_VIEW_SPLITTER_STATE", QByteArray()).toByteArray());
  m_logTable->InitTableView();
}

void LogView::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("LOGTABLE_GEOMETRY", saveGeometry());
  PreferenceSettings().setValue("LOG_VIEW_SPLITTER_STATE", m_logViewSplitter->saveState());
  return QMainWindow::closeEvent(event);
}

void LogView::subscribe() {
  connect(g_logAg()._REFRESH_LOG, &QAction::triggered, this, &LogView::RefreshLogs);
  connect(g_logAg()._COPY_LOCATE_INFO, &QAction::triggered, this, &LogView::CopyNameAndLineNo);
  connect(g_logAg()._REVEAL_LOGS_FILE, &QAction::triggered, this, &LogView::RevealInNativeEditor);
  connect(m_logTypeToolbar, &QToolBar::actionTriggered, this, &LogView::onLogTypeChanged);
  connect(m_logTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &LogView::onSelectedANewLog);
}

#include "Tools/MessageOutput.h"
void LogView::RefreshLogs() {
  MessageOutput::flush();
  m_logModel->_reloadLogFiles();
  m_logTable->scrollToBottom();
}

void LogView::onLogTypeChanged() {
  if (g_logAg()._ALL->isChecked()) {
    m_logProxyModel->setFilterFixedString("");
    return;
  }
  QStringList types;
  foreach (QAction* act, m_logTypeToolbar->actions()) {
    if (act->isSeparator()) {
      continue;
    }
    if (act->isChecked()) {
      types.append(act->text());
    }
  }
  const QString& pat = '[' + types.join("") + ']';
  m_logProxyModel->setFilterRegExp(pat);
}

#include <QApplication>
#include <QClipboard>
void LogView::CopyNameAndLineNo() const {
  const auto& curIndex = m_logTable->currentIndex();
  const auto& srcIndex = m_logProxyModel->mapToSource(curIndex);
  if (not srcIndex.isValid()) {
    qDebug("Select a row and do it again");
    return;
  }
  const QString& info = m_logModel->getFileNameAndLineNo(srcIndex);
  QApplication::clipboard()->setText(info);
}

#include <QDesktopServices>
void LogView::RevealInNativeEditor() const {
  QDesktopServices::openUrl(QUrl::fromLocalFile(m_logModel->rootPath()));
}

void LogView::onSelectedANewLog() {
  const auto& curIndex = m_logTable->currentIndex();
  const auto& srcIndex = m_logProxyModel->mapToSource(curIndex);
  if (not srcIndex.isValid()) {
    qDebug("Select a row to see log details");
    return;
  }
  const QString& txt = m_logModel->fullInfo(srcIndex);
  m_logDetails->setText(txt);
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
