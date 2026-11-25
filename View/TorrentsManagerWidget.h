#ifndef TORRENTSMANAGERWIDGET_H
#define TORRENTSMANAGERWIDGET_H

#include <QMainWindow>
#include <QLineEdit>
#include <QToolBar>
#include "CustomTableView.h"
#include "SqlTableModelPub.h"
#include "TorrDb.h"

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

  bool onSubmitAll();

  void showEvent(QShowEvent *event) override;

private:
  TorrDb mDb;
  QLineEdit* m_searchLE {nullptr};
  QToolBar* m_searchToobar {nullptr};
  CustomTableView* m_torrentsListView {nullptr};
  SqlTableModelPub* m_torrentsDBModel {nullptr};
};

#endif  // TORRENTSMANAGERWIDGET_H
