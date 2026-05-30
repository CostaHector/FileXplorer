#include "MultiPar2Actions.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "StyleSheet.h"
#include "ImageTool.h"
#include "MenuToolButton.h"
#include "InputDialogHelper.h"
#include "MultiParKey.h"
#include "MultiPar2MemoryKey.h"
#include "Configuration.h"
#include <QToolBar>
#include <QActionGroup>

MultiPar2Actions& MultiPar2Actions::GetInst() {
  static MultiPar2Actions inst;
  return inst;
}

QAction* MultiPar2Actions::newCreateAction(int rateOfRedundancy) {
  QAction* createParFile = new QAction{QIcon{":img/CREATE_MULTI_PAR2"}, tr("Create Par2") + QString::asprintf(" (%d%%)", rateOfRedundancy), this};
  createParFile->setToolTip(QString::asprintf("Create Par2 for a file with rate of redundancy %d%%", rateOfRedundancy));
  createParFile->setProperty("RateOfRedundancy", rateOfRedundancy);
  CREATE_PAR2_AG->addAction(createParFile);
  return createParFile;
}

MultiPar2Actions::MultiPar2Actions(QObject* parent)
  : QObject{parent} {
  CREATE_PAR2_AG = new QActionGroup{this};

  _CREATE_PAR2_FILES_10 = newCreateAction(10);
  _CREATE_PAR2_FILES_20 = newCreateAction(20);
  _CREATE_PAR2_FILES_30 = newCreateAction(30);
  _CREATE_PAR2_FILES_40 = newCreateAction(40);

  _CREATE_PAR2_FILES_CUSTOM = new QAction{tr("Create Par2 (Custom)"), this};
  _CREATE_PAR2_FILES_CUSTOM->setToolTip("Create Par2 for a file with custom rate of redundancy");

  _VERIFY_IF_NEED_RECOVERY = new QAction{QIcon{":img/VERIFY_INTEGRITY"}, tr("Verify Integrity"), this};

  subscribe();
}

void MultiPar2Actions::EmitCreatePar2Req(const QAction* pCreatePar2Act) {
  CHECK_FALSE_RETURN_VOID(pCreatePar2Act);
  QVariant rateOfRedundancyVar = pCreatePar2Act->property("RateOfRedundancy");
  if (!rateOfRedundancyVar.isValid()) {
    LOG_E("Rate of redundancy property invalid");
    return;
  }
  bool isOk{false};
  const int rateOfRedundancy = rateOfRedundancyVar.toInt(&isOk);
  if (!isOk) {
    LOG_E("Rate of redundancy variant invalid");
    return;
  }
  if (!MultiParKey::isRateOfRedundancyValid(rateOfRedundancy)) {
    LOG_E("Rate of redundancy[%d] out of range", rateOfRedundancy);
    return;
  }
  emit createPar2Req(rateOfRedundancy);
}

void MultiPar2Actions::EmitCreatePar2CustomReq() {
  const QString label{QString::asprintf("%d<=Rate of Redundancy<=%d", MultiParKey::RATE_OF_REDUNDANCY_MIN, MultiParKey::RATE_OF_REDUNDANCY_MAX)};
  static constexpr int RATE_OF_DEDUNDANCY_STEP = 1;
  bool bUserAccept{false};
  const int oldRateOfRedundancy{getConfig(MultiPar2MemoryKey::CUSTOM_RATE_OF_REDUNDANCY).toInt()};
  int newRateOfRedundancy{-1};
  std::tie(bUserAccept, newRateOfRedundancy) = InputDialogHelper::GetIntWithInitial(nullptr,
                                                                                 "Input rate of redundancy",          //
                                                                                 label,                               //
                                                                                 oldRateOfRedundancy,                    //
                                                                                 MultiParKey::RATE_OF_REDUNDANCY_MIN, //
                                                                                 MultiParKey::RATE_OF_REDUNDANCY_MAX, //
                                                                                 RATE_OF_DEDUNDANCY_STEP);
  if (!bUserAccept) {
    LOG_INFO_NP("Skip", "User cancelled input rate of redundancy");
    return;
  }
  if (!MultiParKey::isRateOfRedundancyValid(newRateOfRedundancy)) {
    LOG_E("Rate of redundancy[%d] out of range", newRateOfRedundancy);
    return;
  }
  if (newRateOfRedundancy != oldRateOfRedundancy) {
    setConfig(MultiPar2MemoryKey::CUSTOM_RATE_OF_REDUNDANCY, newRateOfRedundancy);
  }
  emit createPar2Req(newRateOfRedundancy);
}

void MultiPar2Actions::subscribe() {
  connect(CREATE_PAR2_AG, &QActionGroup::triggered, this, &MultiPar2Actions::EmitCreatePar2Req);
  connect(_CREATE_PAR2_FILES_CUSTOM, &QAction::triggered, this, &MultiPar2Actions::EmitCreatePar2CustomReq);
}

QWidget* MultiPar2Actions::GetToolBar(QWidget* parent) {
  // ":img/CREATE_MULTI_PAR2"
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  QToolBar* tb = new QToolBar{"MultiPar Toolbar", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(tb);
  tb->setOrientation(Qt::Orientation::Vertical);

  MenuToolButton* crtPar2Button{nullptr};
  {
    QList<QAction*> dropdownActions{_CREATE_PAR2_FILES_10, _CREATE_PAR2_FILES_20, _CREATE_PAR2_FILES_30, _CREATE_PAR2_FILES_40, _CREATE_PAR2_FILES_CUSTOM};
    crtPar2Button = new (std::nothrow) MenuToolButton(dropdownActions,                               //
                                                      QToolButton::MenuButtonPopup,                     //
                                                      Qt::ToolButtonStyle::ToolButtonTextBesideIcon, //
                                                      IMAGE_SIZE::TABS_ICON_IN_MENU_24,              //
                                                      tb);
    CHECK_NULLPTR_RETURN_NULLPTR(crtPar2Button);
    crtPar2Button->SetCaption(QIcon{":img/CREATE_MULTI_PAR2"}, tr("Create Par2"));
    crtPar2Button->InitDefaultActionFromQSetting(MultiPar2MemoryKey::LAST_TIME_CREATE_PAR2_TERIGGERED_ACTION, true);
  }

  tb->addWidget(crtPar2Button);
  tb->addAction(_VERIFY_IF_NEED_RECOVERY);
  tb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  tb->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  SetLayoutAlightment(tb->layout(), Qt::AlignLeft);
  return tb;
}
