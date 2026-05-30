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

QAction* MultiPar2Actions::newCreatePar2Action(int rateOfRedundancy) {
  QAction* createParFile = new QAction{QIcon{":img/RATE_OF_REDUNDANCY_" + QString::number(rateOfRedundancy)}, //
                                       tr("Create PAR2 (%1%)").arg(rateOfRedundancy),                         //
                                       this};
  createParFile->setToolTip(QString{"Generate PAR2 recovery files at %1% redundancy"}.arg(rateOfRedundancy));
  createParFile->setProperty("RateOfRedundancy", rateOfRedundancy);
  CREATE_PAR2_AG->addAction(createParFile);
  return createParFile;
}

MultiPar2Actions::MultiPar2Actions(QObject* parent)
  : QObject{parent} {
  CREATE_PAR2_AG = new QActionGroup{this};

  _CREATE_PAR2_FILES_10 = newCreatePar2Action(10);
  _CREATE_PAR2_FILES_15 = newCreatePar2Action(15);
  _CREATE_PAR2_FILES_20 = newCreatePar2Action(20);

  _CREATE_PAR2_FILES_CUSTOM = new QAction{QIcon{":img/RENAME"}, tr("Create PAR2 (Custom)"), this};
  _CREATE_PAR2_FILES_CUSTOM->setToolTip(tr("Generate PAR2 files with user-defined redundancy"));

  _CREATE_PAR2_FILES_AUTOMATIC = new QAction{QIcon{":img/RATE_OF_REDUNDANCY"}, tr("Create PAR2 (Automatic)"), this};
  _CREATE_PAR2_FILES_AUTOMATIC->setToolTip(tr("Generate PAR2 files using redundancy determined by rates value in json"));

  _VERIFY_IF_NEED_RECOVERY = new QAction{QIcon{":img/VERIFY_INTEGRITY"}, tr("Verify Integrity"), this};
  _VERIFY_IF_NEED_RECOVERY->setToolTip(tr("Check file integrity and verify recovery data"));

  subscribe();
}

bool MultiPar2Actions::EmitCreatePar2Req(const QAction* pCreatePar2Act) {
  CHECK_NULLPTR_RETURN_FALSE(pCreatePar2Act);
  QVariant rateOfRedundancyVar = pCreatePar2Act->property("RateOfRedundancy");
  if (!rateOfRedundancyVar.isValid()) {
    LOG_E("Invalid redundancy rate property");
    return false;
  }
  bool isOk{false};
  const int rateOfRedundancy = rateOfRedundancyVar.toInt(&isOk);
  if (!isOk) {
    LOG_E("Failed to convert redundancy rate value");
    return false;
  }
  if (!MultiParKey::isRateOfRedundancyValid(rateOfRedundancy)) {
    LOG_E("Redundancy rate %d out of valid range [%d, %d]", rateOfRedundancy, MultiParKey::RATE_OF_REDUNDANCY_MIN, MultiParKey::RATE_OF_REDUNDANCY_MAX);
    return false;
  }
  emit createPar2Req(rateOfRedundancy);
  return true;
}

bool MultiPar2Actions::EmitCreatePar2ReqCustom() {
  static const QString title{tr("Set Redundancy Rate")};
  static const QString label = tr("Redundancy Rate [%1, %2]").arg(MultiParKey::RATE_OF_REDUNDANCY_MIN).arg(MultiParKey::RATE_OF_REDUNDANCY_MAX);

  static constexpr int RATE_STEP = 1;
  bool bUserAccept{false};
  const int oldRateOfRedundancy = getConfig(MultiPar2MemoryKey::CUSTOM_RATE_OF_REDUNDANCY).toInt();
  int newRateOfRedundancy{-1};

  std::tie(bUserAccept, newRateOfRedundancy)
      = InputDialogHelper::GetIntWithInitial(nullptr, title, label, oldRateOfRedundancy, MultiParKey::RATE_OF_REDUNDANCY_MIN, MultiParKey::RATE_OF_REDUNDANCY_MAX, RATE_STEP);

  if (!bUserAccept) {
    LOG_INFO_NP("Operation cancelled", "User declined to set redundancy rate");
    return false;
  }

  if (!MultiParKey::isRateOfRedundancyValid(newRateOfRedundancy)) {
    LOG_E("Invalid redundancy rate: %d", newRateOfRedundancy);
    return false;
  }

  if (newRateOfRedundancy != oldRateOfRedundancy) {
    setConfig(MultiPar2MemoryKey::CUSTOM_RATE_OF_REDUNDANCY, newRateOfRedundancy);
  }

  emit createPar2Req(newRateOfRedundancy);
  return true;
}

void MultiPar2Actions::subscribe() {
  connect(CREATE_PAR2_AG, &QActionGroup::triggered, this, &MultiPar2Actions::EmitCreatePar2Req);
  connect(_CREATE_PAR2_FILES_CUSTOM, &QAction::triggered, this, &MultiPar2Actions::EmitCreatePar2ReqCustom);
}

QWidget* MultiPar2Actions::GetCreatePar2ToolButton(QWidget* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  QList<QAction*> dropdownActions{_CREATE_PAR2_FILES_10, _CREATE_PAR2_FILES_15, _CREATE_PAR2_FILES_20, _CREATE_PAR2_FILES_CUSTOM, _CREATE_PAR2_FILES_AUTOMATIC};
  MenuToolButton* crtPar2Button = new (std::nothrow) MenuToolButton(dropdownActions,                              //
                                                                    QToolButton::MenuButtonPopup,                 //
                                                                    Qt::ToolButtonStyle::ToolButtonTextUnderIcon, //
                                                                    IMAGE_SIZE::TABS_ICON_IN_MENU_36,             //
                                                                    parent);
  CHECK_NULLPTR_RETURN_NULLPTR(crtPar2Button);
  crtPar2Button->SetCaption(QIcon{":img/CREATE_MULTI_PAR2"}, tr("Create Par2"));
  crtPar2Button->InitDefaultActionFromQSetting(MultiPar2MemoryKey::LAST_TIME_CREATE_PAR2_TERIGGERED_ACTION, true);
  return crtPar2Button;
}
