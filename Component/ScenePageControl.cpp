#include "ScenePageControl.h"
#include "MemoryKey.h"
#include "Configuration.h"
#include "ImageTool.h"
#include "NotificatorMacro.h"
#include <QFormLayout>
#include <QIntValidator>
#include <QLabel>

ScenePageControl::ScenePageControl(const QString& title, QWidget* parent)
  : QToolBar{title, parent} {
  _THE_FRONT_PAGE = new (std::nothrow) QAction(QIcon(":img/PAGINATION_START"), tr("First Page"), this);
  CHECK_NULLPTR_RETURN_VOID(_THE_FRONT_PAGE);
  _THE_FRONT_PAGE->setToolTip(
      QString("<b>%1 (%2)</b><br/>Go to first page").arg(_THE_FRONT_PAGE->text(), _THE_FRONT_PAGE->shortcut().toString()));

  _PREVIOUS_PAGE = new (std::nothrow) QAction(QIcon(":img/PAGINATION_LAST"), tr("Previous Page"), this);
  CHECK_NULLPTR_RETURN_VOID(_PREVIOUS_PAGE);
  _PREVIOUS_PAGE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Comma));
  _PREVIOUS_PAGE->setToolTip(
      QString("<b>%1 (%2)</b><br/>Go to previous page").arg(_PREVIOUS_PAGE->text(), _PREVIOUS_PAGE->shortcut().toString()));

  _NEXT_PAGE = new (std::nothrow) QAction(QIcon(":img/PAGINATION_NEXT"), tr("Next Page"), this);
  CHECK_NULLPTR_RETURN_VOID(_NEXT_PAGE);
  _NEXT_PAGE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Period));
  _NEXT_PAGE->setToolTip(QString("<b>%1 (%2)</b><br/>Go to next page").arg(_NEXT_PAGE->text(), _NEXT_PAGE->shortcut().toString()));

  _THE_BACK_PAGE = new (std::nothrow) QAction(QIcon(":img/PAGINATION_END"), tr("Last Page"), this);
  CHECK_NULLPTR_RETURN_VOID(_THE_BACK_PAGE);
  _THE_BACK_PAGE->setToolTip(
      QString("<b>%1 (%2)</b><br/>Go to last page").arg(_THE_BACK_PAGE->text(), _THE_BACK_PAGE->shortcut().toString()));
  {
    using namespace ScenePageNaviHelper;
    mPageNaviIntAction.init(                    //
        {{_THE_FRONT_PAGE, PageNaviE::FRONT},   //
         {_PREVIOUS_PAGE, PageNaviE::PREVIOUS}, //
         {_NEXT_PAGE, PageNaviE::NEXT},         //
         {_THE_BACK_PAGE, PageNaviE::BACK}},    //
        PageNaviE::END_INVALID,
        QActionGroup::ExclusionPolicy::None); //
  }

  const int sceneCnt1Page = Configuration().value(SceneKey::CNT_EACH_PAGE.name, SceneKey::CNT_EACH_PAGE.toVariant()).toInt();
  mScenesPerPageLE = new (std::nothrow) QLineEdit(QString::number(sceneCnt1Page), this);
  CHECK_NULLPTR_RETURN_VOID(mScenesPerPageLE);
  mScenesPerPageLE->setAlignment(Qt::AlignmentFlag::AlignHCenter);

  mPagesSelectTB = new (std::nothrow) QToolBar{"Page Navigation", this};
  CHECK_NULLPTR_RETURN_VOID(mPagesSelectTB);
  mPagesSelectTB->addActions({_THE_FRONT_PAGE, _PREVIOUS_PAGE});
  {
    mPageIndexInputLE = new (std::nothrow) QLineEdit{"0", this};
    CHECK_NULLPTR_RETURN_VOID(mPageIndexInputLE);
    mPageIndexInputLE->setValidator(new (std::nothrow) QIntValidator{-1, 10000});
    mPageIndexInputLE->setAlignment(Qt::AlignmentFlag::AlignHCenter);
    mPageIndexInputLE->setToolTip("Enter page number and press Enter to jump");
    mPageInfoLabel = new QLabel{"/0", this};
  }
  mPagesSelectTB->addWidget(mPageIndexInputLE);
  mPagesSelectTB->addWidget(mPageInfoLabel);
  mPagesSelectTB->addActions({_NEXT_PAGE, _THE_BACK_PAGE});
  mPagesSelectTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));

  addWidget(new QLabel{"Scenes per page:", this});
  addWidget(mScenesPerPageLE);
  addSeparator();
  addWidget(mPagesSelectTB);
  setOrientation(Qt::Orientation::Vertical);

  subscribe();
}

void ScenePageControl::subscribe() {
  connect(mScenesPerPageLE, &QLineEdit::returnPressed, this, &ScenePageControl::SetScenesCountPerPage);
  connect(mPageIndexInputLE, &QLineEdit::returnPressed, this, &ScenePageControl::SetPageIndex);
  connect(mPageNaviIntAction.getActionGroup(), &QActionGroup::triggered, this, &ScenePageControl::PageIndexIncDec);
}

bool ScenePageControl::SetScenesCountPerPage() {
  int scenesCnt1Page = -1;
  const QString& scenesCnt1PageStr = mScenesPerPageLE->text();
  bool isPageScenesCntValid = false;
  scenesCnt1Page = scenesCnt1PageStr.toInt(&isPageScenesCntValid);
  if (!isPageScenesCntValid) {
    LOG_D("Page scenes count str[%s] invalid", qPrintable(scenesCnt1PageStr));
    return false;
  }
  Configuration().setValue(SceneKey::CNT_EACH_PAGE.name, scenesCnt1Page);
  LOG_D("Scene count each page: %d", scenesCnt1Page);
  emit maxScenesCountPerPageChanged(scenesCnt1Page);
  return true;
}

void ScenePageControl::SetPageIndex() {
  const QString& pageIndStr = mPageIndexInputLE->text();
  bool isNumber{false};
  int pageIndex = pageIndStr.toInt(&isNumber);
  if (!isNumber) {
    LOG_D("Page Index str[%s] invalid", qPrintable(pageIndStr));
    return;
  }
  LOG_D("Page index:: %d", pageIndex);
  emit currentPageIndexChanged(pageIndex);
}

bool ScenePageControl::PageIndexIncDec(const QAction* pageAct) {
  QString beforeIndexStr = mPageIndexInputLE->text();
  bool isNumber{false};
  const int beforePageInd = beforeIndexStr.toInt(&isNumber);
  if (!isNumber) {
    LOG_D("Error before index");
    return false;
  }
  int afterPageInd = beforePageInd;

  using namespace ScenePageNaviHelper;

  PageNaviE naviE = mPageNaviIntAction.act2Enum(pageAct);
  switch (naviE) {
    case PageNaviE::FRONT: {
      afterPageInd = 0;
      break;
    }
    case PageNaviE::PREVIOUS: {
      if (beforePageInd > 0) {
        afterPageInd = beforePageInd - 1;
      } else if (mPagesCount > 0) {
        afterPageInd = mPagesCount - 1;
      }
      break;
    }
    case PageNaviE::NEXT: {
      if (beforePageInd < mPagesCount - 1) {
        afterPageInd = beforePageInd + 1;
      } else if (mPagesCount > 0) {
        afterPageInd = 0;
      }
      break;
    }
    case PageNaviE::BACK: {
      afterPageInd = mPagesCount > 0 ? mPagesCount - 1 : 0;
      break;
    }
    default: {
      LOG_D("Unhandled page navigation type[%s]", c_str(naviE));
      return false;
    }
  }

  if (afterPageInd == beforePageInd) {
    LOG_D("Page remains %d, ignore switch page", beforePageInd);
    return true;
  }
  LOG_D("page index changed: %d->%d", beforePageInd, afterPageInd);
  mPageIndexInputLE->setText(QString::number(afterPageInd));
  emit currentPageIndexChanged(afterPageInd);
  return true;
}

void ScenePageControl::onPagesCountChanged(int newPagesCount) {
  mPagesCount = newPagesCount;
  mPageInfoLabel->setText(QString::asprintf("/%d", mPagesCount));
}
