#include "SceneActionsSubscribe.h"
#include "SceneInPageActions.h"
#include "SceneInfoManager.h"
#include "NotificatorMacro.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include <QToolBar>
#include <QMessageBox>

bool SceneActionsSubscribe::BindWidget(QListView* tableView, ScenesListModel* model) {
  CHECK_NULLPTR_RETURN_FALSE(tableView);
  CHECK_NULLPTR_RETURN_FALSE(model);
  _tableView = tableView;
  _model = model;
  return true;
}

bool SceneActionsSubscribe::PageIndexIncDec(const QAction* pageAct) {
  auto& ags = g_SceneInPageActions();
  QString beforeIndexStr = ags.mPageIndexInputLE->text();
  bool isNumber{false};
  int beforePageInd = beforeIndexStr.toInt(&isNumber);
  if (!isNumber) {
    LOG_D("Error before index");
    return false;
  }
  int dstPageInd = beforePageInd;
  int maxPage = _model->GetPageCnt();
  if (pageAct == ags._NEXT_PAGE) {
    ++dstPageInd;
    dstPageInd %= maxPage;
  } else if (pageAct == ags._LAST_PAGE) {
    --dstPageInd;
    if (dstPageInd < 0) {
      dstPageInd = maxPage - 1;
    }
  } else if (pageAct == ags._THE_FIRST_PAGE) {
    dstPageInd = 0;
  } else if (pageAct == ags._THE_LAST_PAGE) {
    dstPageInd = maxPage - 1;
  } else {
    LOG_D("nothing triggered");
    return false;
  }

  if (dstPageInd == beforePageInd) {
    LOG_D("Page remains %d, ignore switch page", beforePageInd);
    return true;
  }
  LOG_D("page index changed: %d->%d", beforePageInd, dstPageInd);
  ags.mPageIndexInputLE->setText(QString::number(dstPageInd));
  SetPageIndex();
  return true;
}

void SceneActionsSubscribe::SetScenesGroupByPage(bool groupByPageAction) {
  auto& ags = g_SceneInPageActions();
  CHECK_NULLPTR_RETURN_VOID(ags.mPageIndexInputLE)
  CHECK_NULLPTR_RETURN_VOID(ags.mPagesSelectTB)

  ags.mPageIndexInputLE->setEnabled(groupByPageAction);
  ags.mPagesSelectTB->setEnabled(groupByPageAction);

  CHECK_NULLPTR_RETURN_VOID(_model)
  SetScenesCountPerPage();
}

void SceneActionsSubscribe::SetPageIndex() {
  CHECK_NULLPTR_RETURN_VOID(_model);

  auto& ags = g_SceneInPageActions();
  const QString& pageIndStr = ags.mPageIndexInputLE->text();
  bool isNumber{false};
  int pageIndex = pageIndStr.toInt(&isNumber);
  if (!isNumber) {
    LOG_D("Page Index str[%s] invalid", qPrintable(pageIndStr));
    return;
  }
  _model->SetPageIndex(pageIndex);
}

bool SceneActionsSubscribe::SetScenesCountPerPage() {
  CHECK_NULLPTR_RETURN_FALSE(_model);

  auto& ags = g_SceneInPageActions();

  int scenesCnt1Page = -1;
  if (ags._GROUP_BY_PAGE->isChecked()) {
    const QString& scenesCnt1PageStr = ags.mPageDimensionLE->text();
    bool isPageScenesCntValid = false;
    scenesCnt1Page = scenesCnt1PageStr.toInt(&isPageScenesCntValid);
    if (!isPageScenesCntValid) {
      LOG_D("Page scenes count str[%s] invalid", qPrintable(scenesCnt1PageStr));
      return false;
    }
  }

  Configuration().setValue("SCENES_COUNT_EACH_PAGE", scenesCnt1Page);
  _model->ChangeItemsCntIn1Page(scenesCnt1Page);
  return true;
}

void SceneActionsSubscribe::SortSceneItems() {
  auto& ags = g_SceneInPageActions();
  CHECK_NULLPTR_RETURN_VOID(ags._ORDER_AG);
  CHECK_NULLPTR_RETURN_VOID(ags._REVERSE_SORT);
  QAction* triggerAct = ags._ORDER_AG->checkedAction();
  CHECK_NULLPTR_RETURN_VOID(triggerAct)
  const QString sortOptionStr{triggerAct->text()};
  const bool bReverse{ags._REVERSE_SORT->isChecked()};
  // LOG_D("sort option: %s, bReverse: %d", qPrintable(sortOptionStr), bReverse);
  // todo:
}

int SceneActionsSubscribe::CombineMediaInfoIntoJson() {
  CHECK_NULLPTR_RETURN_FALSE(_model);
  CHECK_NULLPTR_RETURN_FALSE(_tableView);
  const QString& rootPath = _model->rootPath();
  if (rootPath.count('/') < 2) {  // large folder
    LOG_D("Combine Media Info may cause lag. As [%s] contains a large json/vid/img(s)", qPrintable(rootPath));
    const auto ret =
        QMessageBox::warning(_tableView, "Large folder alert(May cause LAG)", rootPath, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      LOG_D("User cancel Combine Media Info on a large path[%s]", qPrintable(rootPath));
      return 0;
    }
  }

  SceneInfoManager::ScnMgr jdr;
  int jsonUpdatedCnt = jdr(rootPath).m_jsonUpdatedCnt;
  if (jsonUpdatedCnt >= 0) {
    LOG_OK_P("Json(s) Update succeed", "count: %d, rootpath:%s", jsonUpdatedCnt, qPrintable(rootPath));
  }

  int scnFileCnt = jdr.WriteDictIntoScnFiles();
  if (scnFileCnt == -1) {
    LOG_ERR_NP("Combine scn file failed. May path not exist", rootPath);
    return scnFileCnt;
  }
  if (scnFileCnt == 0) {
    LOG_OK_NP("Skip. No json file find, No need to combine scn file", rootPath);
    return scnFileCnt;
  }
  LOG_OK_P("Update scn file(s) succeed", "cnt: %d, rootpath:%s", scnFileCnt, qPrintable(rootPath));
  _model->setRootPath(rootPath, true);
  return scnFileCnt;
}

bool SceneActionsSubscribe::operator()() {
  if (_model == nullptr) {
    LOG_W("_model is nullptr");
    return false;
  }
  if (_tableView == nullptr) {
    LOG_W("_tableView is nullptr");
    return false;
  }

  auto& ags = g_SceneInPageActions();
  connect(ags._COMBINE_MEDIAINFOS_JSON, &QAction::triggered, this, &SceneActionsSubscribe::CombineMediaInfoIntoJson);
  connect(ags._ORDER_AG, &QActionGroup::triggered, this, &SceneActionsSubscribe::SortSceneItems);
  connect(ags._REVERSE_SORT, &QAction::triggered, this, &SceneActionsSubscribe::SortSceneItems);
  connect(ags._GROUP_BY_PAGE, &QAction::toggled, this, &SceneActionsSubscribe::SetScenesGroupByPage);
  connect(ags.mPageDimensionLE, &QLineEdit::textChanged, this, &SceneActionsSubscribe::SetScenesCountPerPage);
  connect(ags.mPageIndexInputLE, &QLineEdit::textChanged, this, &SceneActionsSubscribe::SetPageIndex);
  connect(ags.mPagesSelectTB, &QToolBar::actionTriggered, this, &SceneActionsSubscribe::PageIndexIncDec);
  return true;
}
