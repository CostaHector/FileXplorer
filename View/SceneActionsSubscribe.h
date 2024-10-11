#ifndef SCENEACTIONSSUBSCRIBE_H
#define SCENEACTIONSSUBSCRIBE_H

#include <QObject>
#include <QAction>
#include <QLineEdit>
#include <QTableView>

#include "Model/ScenesTableModel.h"

class SceneActionsSubscribe : public QObject {
 public:
  explicit SceneActionsSubscribe(QObject* parent = nullptr) : QObject{parent} {};
  bool BindWidget(QTableView* tableView, ScenesTableModel* model);
  bool operator()();

  bool PageIndexIncDec(const QAction* pageAct);

  void SetScenesGroupByPage(bool groupByPageAction);

  void SetPageIndex();

  bool SetScenesPerColumn();

  bool SetScenesPerRow();

  void SortSceneItems();

  void CombineMediaInfoIntoJson();
  void UpdateScnFilesOnly();
 private:
  ScenesTableModel* _model{nullptr};
  QTableView* _tableView{nullptr};
};
#endif  // SCENEACTIONSSUBSCRIBE_H
