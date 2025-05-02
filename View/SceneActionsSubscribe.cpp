#include "SceneActionsSubscribe.h"
#include "Actions/SceneInPageActions.h"
#include "Tools/SceneInfoManager.h"
#include "Component/Notificator.h"
#include "public/PublicVariable.h"
#include <QToolBar>
#include <QMessageBox>

bool SceneActionsSubscribe::BindWidget(QListView* tableView, ScenesListModel* model) {
  if (tableView == nullptr) {
    qWarning("tableView is nullptr");
    return false;
  }
  if (model == nullptr) {
    qWarning("model is nullptr");
    return false;
  }
  _tableView = tableView;
  _model = model;
  return true;
}

bool SceneActionsSubscribe::PageIndexIncDec(const QAction* pageAct) {
  auto& ags = g_SceneInPageActions();
  QString beforeIndexStr = ags.mPageIndexInputLE->text();
  bool isNumber{false};
  int beforePageInd = beforeIndexStr.toInt(&isNumber);
  if (not isNumber) {
    qDebug("Error before index");
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
    qDebug("nothing triggered");
    return false;
  }

  if (dstPageInd == beforePageInd) {
    qDebug("Page remains %d, ignore switch page", beforePageInd);
    return true;
  }
  qDebug("page index changed: %d->%d", beforePageInd, dstPageInd);
  ags.mPageIndexInputLE->setText(QString::number(dstPageInd));
  emit ags.mPageIndexInputLE->textChanged(ags.mPageIndexInputLE->text());
  return true;
}

void SceneActionsSubscribe::SetScenesGroupByPage(bool groupByPageAction) {
  auto& ags = g_SceneInPageActions();
  if (ags.mPagesSelectTB == nullptr) {
    qCritical("mPagesSelectTB is nullptr");
    return;
  }
  ags.mPageIndexInputLE->setEnabled(groupByPageAction);
  ags.mPagesSelectTB->setEnabled(groupByPageAction);

  if (_model == nullptr) {
    qCritical("_model is nullptr");
    return;
  }

  if (groupByPageAction) {
    SetScenesPerColumn();
  } else {
    _model->ShowAllScenesInOnePage();
  }
}

void SceneActionsSubscribe::SetPageIndex() {
  auto& ags = g_SceneInPageActions();
  const QString& pageIndStr = ags.mPageIndexInputLE->text();
  bool isNumber{false};
  int pageIndex = pageIndStr.toInt(&isNumber);
  if (not isNumber) {
    qDebug("Page Index str[%s] invalid", qPrintable(pageIndStr));
    return;
  }
  if (_model == nullptr) {
    qWarning("_model is nullptr");
    return;
  }
  _model->SetPageIndex(pageIndex);
}

bool SceneActionsSubscribe::SetScenesPerColumn() {
  if (_model == nullptr) {
    qWarning("_model is nullptr");
    return false;
  }

  auto& ags = g_SceneInPageActions();
  const QString& scenesCnt1PageStr = ags.mPageDimensionLE->text();
  bool isPageScenesCntValid = false;
  const int scenesCnt1Page = scenesCnt1PageStr.toInt(&isPageScenesCntValid);
  if (!isPageScenesCntValid) {
    qDebug("Page scenes count str[%s] invalid", qPrintable(scenesCnt1PageStr));
    return false;
  }
  PreferenceSettings().setValue("SCENES_COUNT_EACH_PAGE", scenesCnt1Page);
  _model->ChangeItemsCntIn1Page(scenesCnt1Page);
  return true;
}

void SceneActionsSubscribe::SortSceneItems() {
  auto& ags = g_SceneInPageActions();
  if (ags._ORDER_AG == nullptr || ags._REVERSE_SORT == nullptr) {
    qCritical("_ORDER_AG or _REVERSE_SORT is nullptr");
    return;
  }
  QAction* triggerAct = ags._ORDER_AG->checkedAction();
  if (triggerAct == nullptr) {
    qCritical("triggerAct is nullptr, nothing sort option is select");
    return;
  }
  const QString sortOptionStr{triggerAct->text()};
  const bool bReverse{ags._REVERSE_SORT->isChecked()};
  SceneInfoManager::SceneSortOption sortOption = SceneInfoManager::GetSortOptionFromStr(sortOptionStr);
  qDebug("sort option: %s, bReverse: %d", qPrintable(sortOptionStr), bReverse);
  _model->SortOrder(sortOption, bReverse);
}

void SceneActionsSubscribe::CombineMediaInfoIntoJson() {
  if (_model == nullptr || _tableView == nullptr) {
    qDebug("_model or _tableView is nullptr");
    return;
  }
  const QString& rootPath = _model->rootPath();
  if (rootPath.count('/') < 2) {  // large folder
    qDebug("Combine Media Info may cause lag. As [%s] contains a large json/vid/img(s)", qPrintable(rootPath));
    const auto ret =
        QMessageBox::warning(_tableView, "Large folder alert(May cause LAG)", rootPath, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      qDebug("User cancel Combine Media Info on a large path[%s]", qPrintable(rootPath));
      return;
    }
  }

  JsonDataRefresher jdr;
  int jsonUpdatedCnt = jdr(rootPath);
  if (jsonUpdatedCnt >= 0) {
    Notificator::goodNews(QString("%1 Json(s) Update succeed").arg(jsonUpdatedCnt), qPrintable(rootPath));
  }
  int scnFileCnt = jdr.GenerateScnFiles();
  if (scnFileCnt == -1) {
    Notificator::badNews("Combine scn file failed. May path not exist", qPrintable(rootPath));
    return;
  }
  if (scnFileCnt == 0) {
    Notificator::goodNews("Skip. No json file find, No need to combine scn file", qPrintable(rootPath));
    return;
  }
  Notificator::goodNews(QString("Update %1 scn file(s) succeed").arg(scnFileCnt), qPrintable(rootPath));
  _model->setRootPath(rootPath, true);
}

void SceneActionsSubscribe::UpdateScnFilesOnly() {
  if (_model == nullptr || _tableView == nullptr) {
    qDebug("_model or _tableView is nullptr");
    return;
  }
  const QString& rootPath = _model->rootPath();
  if (rootPath.count('/') < 2) {  // large folder
    qDebug("Update Scn file may cause lag. As [%s] contains a large json(s)", qPrintable(rootPath));
    const auto ret =
        QMessageBox::warning(_tableView, "Large folder alert(May cause LAG)", rootPath, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      qDebug("User cancel Update Scn file on a large path[%s]", qPrintable(rootPath));
      return;
    }
  }

  int scnFileCnt = SceneInfoManager::GenerateAScnFile(rootPath);
  if (scnFileCnt == -1) {
    Notificator::badNews("Update scn file failed. May path not exist", qPrintable(rootPath));
    return;
  }
  if (scnFileCnt == 0) {
    Notificator::goodNews("Skip. No json file find, No need to update scn file at all", qPrintable(rootPath));
    return;
  }
  Notificator::goodNews(QString("Update %1 scn file(s) succeed").arg(scnFileCnt), qPrintable(rootPath));
  _model->setRootPath(rootPath);
}

bool SceneActionsSubscribe::operator()() {
  if (_model == nullptr) {
    qWarning("_model is nullptr");
    return false;
  }
  if (_tableView == nullptr) {
    qWarning("_tableView is nullptr");
    return false;
  }

  auto& ags = g_SceneInPageActions();
  connect(ags._COMBINE_MEDIAINFOS_JSON, &QAction::triggered, this, &SceneActionsSubscribe::CombineMediaInfoIntoJson);
  connect(ags._UPDATE_SCN_ONLY, &QAction::triggered, this, &SceneActionsSubscribe::UpdateScnFilesOnly);
  connect(ags._ORDER_AG, &QActionGroup::triggered, this, &SceneActionsSubscribe::SortSceneItems);
  connect(ags._REVERSE_SORT, &QAction::triggered, this, &SceneActionsSubscribe::SortSceneItems);
  connect(ags._GROUP_BY_PAGE, &QAction::triggered, this, &SceneActionsSubscribe::SetScenesGroupByPage);
  connect(ags.mPageDimensionLE, &QLineEdit::textChanged, this, &SceneActionsSubscribe::SetScenesPerColumn);
  connect(ags.mPageIndexInputLE, &QLineEdit::textChanged, this, &SceneActionsSubscribe::SetPageIndex);
  connect(ags.mPagesSelectTB, &QToolBar::actionTriggered, this, &SceneActionsSubscribe::PageIndexIncDec);
  return true;
}
