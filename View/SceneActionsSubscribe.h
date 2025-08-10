#ifndef SCENEACTIONSSUBSCRIBE_H
#define SCENEACTIONSSUBSCRIBE_H

#include <QObject>
#include <QAction>
#include <QLineEdit>
#include <QListView>

#include "ScenesListModel.h"

class SceneActionsSubscribe : public QObject {
 public:
  explicit SceneActionsSubscribe(QObject* parent = nullptr) : QObject{parent} {};
  bool BindWidget(QListView* tableView, ScenesListModel* model);
  bool operator()();

  bool PageIndexIncDec(const QAction* pageAct);
  void SetScenesGroupByPage(bool groupByPageAction);

  void SetPageIndex();
  bool SetScenesPerColumn();

  void SortSceneItems();

  void CombineMediaInfoIntoJson();
  void UpdateScnFilesOnly();
 private:
  ScenesListModel* _model{nullptr};
  QListView* _tableView{nullptr};
};
#endif  // SCENEACTIONSSUBSCRIBE_H
