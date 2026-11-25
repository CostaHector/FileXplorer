#include "ScenePageControl.h"
#include "SceneKey.h"
#include "Configuration.h"
#include "PublicCheckMacro.h"
#include "ImageTool.h"

#include <QVBoxLayout>
#include <QShortcut>
#include <QToolBar>
#include <QAction>
#include <QSpinBox>

ScenePageControl* ScenePageControl::GInstance(ScenePageControl* pInitValue, bool bReset) {
  static ScenePageControl* pInstance = pInitValue;
  if (bReset) {
    pInstance = pInitValue;
    return nullptr;
  }
  if (pInstance == nullptr) {
    if (pInitValue == nullptr) {
      LOG_W("Instance not init at all");
    } else {
      pInstance = pInitValue;
      LOG_W("Instance should init before here. adjust init position");
    }
  }
  return pInstance;
}

void UpdateSpinBoxPrefixAndSuffix(QSpinBox& spinxBox) {
  spinxBox.setPrefix(QString::number(spinxBox.minimum()) + "<=");
  spinxBox.setSuffix("<" + QString::number(spinxBox.maximum() + 1));
}

ScenePageControl::ScenePageControl(QWidget* parent)
  : QWidget{parent} {
  {
    mPageIndexInputSB = new (std::nothrow) QSpinBox{this};
    CHECK_NULLPTR_RETURN_VOID(mPageIndexInputSB);
    mPageIndexInputSB->setRange(0, 0);
    mPageIndexInputSB->setValue(0);
    UpdateSpinBoxPrefixAndSuffix(*mPageIndexInputSB);

    mPageIndexInputSB->setAlignment(Qt::AlignmentFlag::AlignHCenter);
    mPageIndexInputSB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // -1 comma, +1 period
    QShortcut* m_scDown = new (std::nothrow) QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Comma), mPageIndexInputSB);
    CHECK_NULLPTR_RETURN_VOID(m_scDown);
    QShortcut* m_scUp = new (std::nothrow) QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Period), mPageIndexInputSB);
    CHECK_NULLPTR_RETURN_VOID(m_scUp);
    connect(m_scDown, &QShortcut::activated, mPageIndexInputSB, &QSpinBox::stepDown);
    connect(m_scUp, &QShortcut::activated, mPageIndexInputSB, &QSpinBox::stepUp);
    mPageIndexInputSB->setToolTip(QString{"Jump to the page input here.\n"
                                          "(%1) for previous page\n"
                                          "(%2) for next page"}.arg(m_scDown->key().toString(), m_scUp->key().toString()));
  }

  QToolBar* mPagesSelectTB = new (std::nothrow) QToolBar{"Page Navigation", this};
  {
    CHECK_NULLPTR_RETURN_VOID(mPagesSelectTB);
    mPagesSelectTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
    mPagesSelectTB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _THE_FRONT_PAGE = mPagesSelectTB->addAction(QIcon(":img/PAGINATION_START"), tr("First Page"));
    _THE_FRONT_PAGE->setToolTip(QString("<b>%1 (%2)</b><br/>Go to first page").arg(_THE_FRONT_PAGE->text(), _THE_FRONT_PAGE->shortcut().toString()));
    mPagesSelectTB->widgetForAction(_THE_FRONT_PAGE)->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    mPagesSelectTB->addWidget(mPageIndexInputSB);
    _THE_BACK_PAGE = mPagesSelectTB->addAction(QIcon(":img/PAGINATION_END"), tr("Last Page"));
    _THE_BACK_PAGE->setToolTip(QString("<b>%1 (%2)</b><br/>Go to last page").arg(_THE_BACK_PAGE->text(), _THE_BACK_PAGE->shortcut().toString()));
    mPagesSelectTB->widgetForAction(_THE_BACK_PAGE)->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
  }

  _PAGE_NAVI_AG = new QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(_PAGE_NAVI_AG);
  _PAGE_NAVI_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  _PAGE_NAVI_AG->addAction(_THE_FRONT_PAGE);
  _PAGE_NAVI_AG->addAction(_THE_BACK_PAGE);

  mScenesPerPageInputSB = new (std::nothrow) QSpinBox(this);
  CHECK_NULLPTR_RETURN_VOID(mScenesPerPageInputSB);
  mScenesPerPageInputSB->setRange(-1, 10000);
  mScenesPerPageInputSB->setSingleStep(10);
  const int sceneCnt1Page = Configuration().value(SceneKey::CNT_EACH_PAGE.name, SceneKey::CNT_EACH_PAGE.toVariant()).toInt();
  mScenesPerPageInputSB->setValue(sceneCnt1Page);
  mScenesPerPageInputSB->setPrefix("Each page ");
  mScenesPerPageInputSB->setSuffix(" scene(s)");
  mScenesPerPageInputSB->setAlignment(Qt::AlignmentFlag::AlignHCenter);

  QVBoxLayout* mainLayout = new QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(mainLayout);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(mScenesPerPageInputSB, 1);
  mainLayout->addWidget(mPagesSelectTB, 2);

  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

  subscribe();
}

void ScenePageControl::subscribe() {
  using VALUE_CHANGED_PARMS_INT_VERSION = void(QSpinBox::*)(int);
  connect(mScenesPerPageInputSB, (VALUE_CHANGED_PARMS_INT_VERSION)&QSpinBox::valueChanged, this, &ScenePageControl::EmitScenesCountPerPageChanged);
  connect(mPageIndexInputSB, (VALUE_CHANGED_PARMS_INT_VERSION)&QSpinBox::valueChanged, this, &ScenePageControl::currentPageIndexChanged);
  connect(_PAGE_NAVI_AG, &QActionGroup::triggered, this, &ScenePageControl::onPageIndexFrontBackActTriggered);
}

bool ScenePageControl::onPageIndexFrontBackActTriggered(const QAction* pageAct) {
  CHECK_NULLPTR_RETURN_FALSE(pageAct);
  const int beforePageInd = mPageIndexInputSB->value();
  int afterPageInd = beforePageInd;
  const int totalPagesCount = mPageIndexInputSB->maximum(); // "0<="x"<totalPagesCount:
  if (pageAct == _THE_FRONT_PAGE) {
    afterPageInd = 0;
  } else if (pageAct == _THE_BACK_PAGE) {
    afterPageInd = totalPagesCount;
  } else {
    LOG_D("Unhandled page navigation action[%s]", qPrintable(pageAct->text()));
    return false;
  }

  if (afterPageInd == beforePageInd) {
    LOG_D("Page index remains %d, ignore switch page", beforePageInd);
    return true;
  }
  LOG_D("page index changed: %d->%d", beforePageInd, afterPageInd);
  mPageIndexInputSB->setValue(afterPageInd);
  return true;
}

bool ScenePageControl::EmitScenesCountPerPageChanged(int scenesPerPageCnt) {
  Configuration().setValue(SceneKey::CNT_EACH_PAGE.name, scenesPerPageCnt);
  emit maxScenesCountPerPageChanged(scenesPerPageCnt);
  return true;
}

void ScenePageControl::onPagesCountChanged(int newPagesCount) {
  mPageIndexInputSB->setRange(0, newPagesCount - 1);
  UpdateSpinBoxPrefixAndSuffix(*mPageIndexInputSB);
}