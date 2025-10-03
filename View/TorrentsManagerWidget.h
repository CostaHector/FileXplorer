#ifndef TORRENTSMANAGERWIDGET_H
#define TORRENTSMANAGERWIDGET_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QLineEdit>
#include "CustomTableView.h"
#include "TorrDb.h"
#include <QToolBar>

namespace TorrentsManagerWidgetMock {
inline std::pair<QString, QString>& MockTorrDatabase() {
  static std::pair<QString, QString> torrDataBase2ConnPair;
  return torrDataBase2ConnPair;
}
inline QString& MockDeleteTorrWhereClause() {
  static QString deleteTorrWhereClause;
  return deleteTorrWhereClause;
}
inline bool& MockQryBeforeDropTable() {
  static bool confirmDrop = false;
  return confirmDrop;
}
}

class TorrentsManagerWidget : public QMainWindow {
 public:
  explicit TorrentsManagerWidget(QWidget* parent = nullptr);

  void subscribe();
  bool onInitDataBase();
  void onInitATable();
  bool onInsertIntoTable();
  bool onDropATable();
  bool onDeleteFromTable();

  bool onSubmit();

  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent* event) override;
  void updateWindowsSize();

private:
  TorrDb mDb;
  QLineEdit* m_searchLE {nullptr};
  QToolBar* m_searchToobar {nullptr};
  CustomTableView* m_torrentsListView {nullptr};
  QSqlTableModel* m_torrentsDBModel {nullptr};
};

#endif  // TORRENTSMANAGERWIDGET_H
