#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "CustomTableView.h"
#include "Model/LogModel.h"
#include "Model/LogProxyModel.h"

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <QSplitter>
#include <QTextEdit>

class LogView : public QMainWindow {
  Q_OBJECT
 public:
  explicit LogView(QWidget* parent = nullptr);

  void ReadSettings();
  virtual void hideEvent(QHideEvent *event) override;
  virtual void closeEvent(QCloseEvent* event) override;

  void subscribe();

  void RefreshLogs();
  void onLogTypeChanged();

  void CopyNameAndLineNo() const;
  void RevealInNativeEditor() const;

  void onSelectedANewLog();
 signals:

 private:
  QMenu* m_logMenu{new QMenu{"Log Menu", this}};

  QToolBar* m_logTypeToolbar{nullptr};
  QToolBar* m_logToolBar{new QToolBar{"log toolbar", this}};
  LogModel* m_logModel{new LogModel{this}};
  LogProxyModel* m_logProxyModel{new LogProxyModel{this}};
  CustomTableView* m_logTable{new CustomTableView{"LOG_TABLE", this}};
  QTextEdit* m_logDetails{new QTextEdit{this}};
  QSplitter* m_logViewSplitter{new QSplitter{this}};
};

#endif  // LOGVIEW_H
