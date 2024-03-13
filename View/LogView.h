#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "CustomTableView.h"
#include "LogModel.h"

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QToolBar>

class LogView : public QMainWindow {
 public:
  explicit LogView(QWidget* parent = nullptr);

  void ReadSettings();
  void closeEvent(QCloseEvent* event) override;

  void subscribe();

  void RefreshLogs();

  void CopyNameAndLineNo();

 signals:
  QAction* m_refreshLogs{new QAction{QIcon(":/themes/RELOAD_FROM_DISK"), tr("Refresh"), this}};
  QActionGroup* m_typeAGS{new QActionGroup{this}};

  QAction* _COPY_NAME_AND_LINE{new QAction{tr("Copy name and lines"), this}};

  QMenu* m_logMenu{new QMenu{"Log Menu", this}};

  QToolBar* m_logToolBar{new QToolBar{"log toolbar", this}};
  LogModel* m_logModel{new LogModel{this}};
  CustomTableView* m_logTable{new CustomTableView{"LOG_TABLE", this}};
};

#endif  // LOGVIEW_H
