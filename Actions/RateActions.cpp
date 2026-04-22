#include "RateActions.h"
#include "PublicMacro.h"
#include "RateHelper.h"
#include "MenuToolButton.h"
#include "NotificatorMacro.h"
#include "MemoryKey.h"
#include "InputDialogHelper.h"

RateActions& RateActions::GetInst(RateRequestFrom reqFrom) {
  if (reqFrom < RateRequestFrom::FROM_BEGIN || reqFrom >= RateRequestFrom::FROM_BUTT) {
    LOG_C("rate request[%d] out of range[%d, %d)", (int)reqFrom, (int)RateRequestFrom::FROM_BEGIN, (int)RateRequestFrom::FROM_BUTT);
    reqFrom = RateRequestFrom::FILE_XPLORER;
  }
  // RateActions of BASIC_VIDEO_PLAYER will be used in VideoView and BasicVideoView twice. so for BasicVideoView use constructor directly
  static RateActions inst[(int)RateRequestFrom::FROM_BUTT];
  return inst[(int)reqFrom];
}

// Only call me in BasicVideoView
RateActions::RateActions(QObject* parent) : QObject{parent} {
  RATE_AGS = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(RATE_AGS);
  RATE_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  ALL_RATE_ACTIONS_LIST.reserve(RateHelper::BUTT_V - RateHelper::MIN_V);
  for (int rate = RateHelper::BUTT_V - 1; rate >= RateHelper::MIN_V; --rate) {
    QAction* pAct = new (std::nothrow) QAction{QIcon(RateHelper::GetRatePixmap(rate)),  //
                                               QString::number(rate) + tr(" score"),    //
                                               this};
    CHECK_NULLPTR_RETURN_VOID(pAct);
    pAct->setData(rate);
    const int keypad = (rate == 10) ? Qt::Key_Plus : (Qt::Key_0 + rate);
    pAct->setShortcut(QKeySequence(Qt::ControlModifier | Qt::KeypadModifier | keypad));
    RATE_AGS->addAction(pAct);
    ALL_RATE_ACTIONS_LIST.push_back(pAct);
  }

  _RATE_RECURSIVELY = new (std::nothrow) QAction{QIcon{":img/LIKE_RECURSIVELY"}, tr("Rate recusively"), this};
  CHECK_NULLPTR_RETURN_VOID(_RATE_RECURSIVELY);
  _RATE_RECURSIVELY->setToolTip("Rate only unrated movies in selected folder and its subfolders");

  _RATE_RECURSIVELY_OVERRIDE = new (std::nothrow) QAction{tr("Rate recusively(force)"), this};
  CHECK_NULLPTR_RETURN_VOID(_RATE_RECURSIVELY_OVERRIDE);
  _RATE_RECURSIVELY_OVERRIDE->setToolTip("Rate all movies, overwriting existing ratings");

  _INCREASING_RATING = new QAction{QIcon{":img/INCREASING_RATING"}, tr("Increase rating"), this};
  _INCREASING_RATING->setToolTip("Increase the movie rating by 1 point");
  _DECREASING_RATING = new QAction{QIcon{":img/DECREASING_RATING"}, tr("Decrease rating"), this};
  _DECREASING_RATING->setToolTip("Decrease the movie rating by 1 point");
  _INCREASING_RATETING_RECURSIVELY = new QAction{tr("Increase rating recusively"), this};
  _INCREASING_RATETING_RECURSIVELY->setToolTip("Increase all movies under path rating by 1 point");
  _DECREASING_RATETING_RECURSIVELY = new QAction{tr("Decrease rating recusively"), this};
  _DECREASING_RATETING_RECURSIVELY->setToolTip("Decrease the movie under path rating by 1 point");

  subscribe();
}

QMenu* RateActions::GetRateMenu(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  QMenu* rateMenu = new (std::nothrow) QMenu{tr("Rate"), notNullParent};
  rateMenu->setIcon(QIcon{":img/LIKE"});
  rateMenu->setToolTip("Rate for your movie");
  rateMenu->addSection("Specify a rate value");
  rateMenu->addActions(GetAllRateActionsList());
  rateMenu->addSeparator();
  rateMenu->addSection("Adjust +1/-1 rate point");
  rateMenu->addActions(GetAdjustRateActions());
  rateMenu->addAction(_INCREASING_RATETING_RECURSIVELY);
  rateMenu->addAction(_DECREASING_RATETING_RECURSIVELY);
  rateMenu->addSeparator();
  rateMenu->addSection("Recursively rate all videos with user specified value");
  rateMenu->addAction(_RATE_RECURSIVELY);
  rateMenu->addAction(_RATE_RECURSIVELY_OVERRIDE);
  rateMenu->setToolTipsVisible(true);
  return rateMenu;
}

QWidget* RateActions::GetRateToolButton(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  QMenu* pDropdownMenu = GetRateMenu(notNullParent);
  CHECK_NULLPTR_RETURN_NULLPTR(pDropdownMenu);
  MenuToolButton* rateToolButton = new (std::nothrow) MenuToolButton(pDropdownMenu,                            //
                                                                     QToolButton::InstantPopup,                //
                                                                     Qt::ToolButtonStyle::ToolButtonIconOnly,  //
                                                                     IMAGE_SIZE::TABS_ICON_IN_MENU_16, notNullParent);
  CHECK_NULLPTR_RETURN_NULLPTR(rateToolButton);
  rateToolButton->SetCaption(QIcon{":img/LIKE"}, tr("Rate"), "Rate for your movie");
  return rateToolButton;
}

void RateActions::subscribe() {
  connect(RATE_AGS, &QActionGroup::triggered, this, &RateActions::onRateActionTriggered);
  connect(_RATE_RECURSIVELY, &QAction::triggered, this, [this]() { emit RateMovieRecursivelyReq(false); });
  connect(_RATE_RECURSIVELY_OVERRIDE, &QAction::triggered, this, [this]() { emit RateMovieRecursivelyReq(true); });
  connect(_INCREASING_RATING, &QAction::triggered, this, [this]() {emit AdjustRateMovieReq(1);});
  connect(_DECREASING_RATING, &QAction::triggered, this, [this]() {emit AdjustRateMovieReq(-1);});
  connect(_INCREASING_RATETING_RECURSIVELY, &QAction::triggered, this, [this]() {emit AdjustRateMovieRecursivelyReq(1);});
  connect(_DECREASING_RATETING_RECURSIVELY, &QAction::triggered, this, [this]() {emit AdjustRateMovieRecursivelyReq(-1);});
}

bool RateActions::onRateActionTriggered(const QAction* pActTriggered) {
  CHECK_NULLPTR_RETURN_FALSE(pActTriggered);
  bool bOk = false;
  int newRate = pActTriggered->data().toInt(&bOk);
  if (!bOk) {
    LOG_W("data property in QAction is not a number");
    return false;
  }
  emit RateMovieReq(newRate);
  return true;
}

int RateActions::onRateMoviesRecursively(const QString& rootPath, bool bOverrideForce, QWidget* parent) const {
  const QString title{bOverrideForce ? "Rate All Movies - Overwrite Existing" : "Rate Unrated Movies Only"};
  QString message{QString::asprintf("Set rating for movies in:\n%s\n\n", qPrintable(rootPath))};
  message += bOverrideForce ? "This will overwrite ALL existing ratings." : "Only movies without ratings or current rating value is 0 will be affected.";

  const int defaultRate = Configuration().value(VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE.name, VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE.v).toInt();

  bool bAccept{false};
  int newRate{defaultRate};
  std::tie(bAccept, newRate) = InputDialogHelper::GetIntWithInitial(parent, title, message, defaultRate, RateHelper::MIN_V, RateHelper::MAX_V, 1);
  if (!bAccept) {
    LOG_INFO_NP("User cancel rate recursively", rootPath);
    return 0;
  }
  if (newRate != defaultRate) {
    Configuration().setValue(VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE.name, newRate);
  }
  const int succeedCnt = RateHelper::RateMovieRecursively(rootPath, newRate, bOverrideForce);
  LOG_OE_P(succeedCnt > 0, "Rate movie(s)", "%d item(s) have been rate to %d, override: %d", succeedCnt, newRate, bOverrideForce);
  return succeedCnt;
}
