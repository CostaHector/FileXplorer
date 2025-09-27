#ifndef SCENEACTIONSSUBSCRIBE_H
#define SCENEACTIONSSUBSCRIBE_H

#include <QObject>
#include <QAction>
#include <QLineEdit>
#include <QListView>

#include "ScenesListModel.h"

class SceneActionsSubscribe : public QObject {
 public:
  using QObject::QObject;

  bool BindWidget(QListView* tableView, ScenesListModel* model);
  bool operator()();

  bool PageIndexIncDec(const QAction* pageAct);
  void SetScenesGroupByPage(bool groupByPageAction);

  void SetPageIndex();
  bool SetScenesCountPerPage();

  void SortSceneItems();

  int CombineMediaInfoIntoJson();
  void UpdateScnFilesOnly();
 private:
  ScenesListModel* _model{nullptr};
  QListView* _tableView{nullptr};
};
#endif  // SCENEACTIONSSUBSCRIBE_H
