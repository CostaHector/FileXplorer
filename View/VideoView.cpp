#include "VideoView.h"
#include "DataFormatter.h"
#include "DualIconCheckableAction.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include <QMouseEvent>
#include <QFile>
#include <QResizeEvent>
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include <QDirIterator>

constexpr QMediaPlaylist::PlaybackMode VideoView::DEFAULT_PLAYBACK_MODE;
constexpr int VideoView::SOURCE_INDEX_COLUMN;

VideoView::VideoView(bool bBasicMode, QWidget* parent)
  : QSplitter{Qt::Orientation::Horizontal, parent} {
  mBasicVideoView = new (std::nothrow) BasicVideoView{false, this};

  mSelectVideoFolder = new (std::nothrow) QAction{QIcon{":/VideoPlayer/OPEN_A_FOLDER"}, tr("select a folder"), this};

  mSeekBackwardHotAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/JUMP_LAST_HOT_SCENE"}, tr("previous hot point"), this};
  mSeekForwardHotAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/JUMP_NEXT_HOT_SCENE"}, tr("next hot point"), this};

  mPlayPrevAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VIDEO_PREVIOUS"}, tr("play previous video"), this};
  mPlayNextAct = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VIDEO_NEXT"}, tr("play next video"), this};

  mShowFrames = new (std::nothrow) QAction{QIcon{""}, tr("show frames"), this};
  mShowFrames->setCheckable(true);
  mShowFrames->setChecked(false);

  mShowVideoList = new (std::nothrow) QAction{QIcon{":/VideoPlayer/VIDEO_LIST"}, tr("show video list"), this};
  mShowVideoList->setCheckable(true);
  mShowVideoList->setChecked(true);

  mPlaybackMode_CurrentItemOnce = new (std::nothrow)
      QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_CURRENT_ITEM_ONCE"}, tr("current item once"), this};
  mPlaybackMode_CurrentItemOnce->setCheckable(true);
  mPlaybackMode_CurrentItemInLoop = new (std::nothrow)
      QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_CURRENT_ITEM_IN_LOOP"}, tr("current item in loop"), this};
  mPlaybackMode_CurrentItemInLoop->setCheckable(true);
  mPlaybackMode_Sequential = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_SEQUENTIAL"}, tr("sequential"), this};
  mPlaybackMode_Sequential->setCheckable(true);
  mPlaybackMode_Loop = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_LOOP"}, tr("loop"), this};
  mPlaybackMode_Loop->setCheckable(true);
  mPlaybackMode_Random = new (std::nothrow) QAction{QIcon{":/VideoPlayer/PLAYBACK_MODE_RANDOM"}, tr("random"), this};
  mPlaybackMode_Random->setCheckable(true);

  mPlaybackIntAction.init({{mPlaybackMode_CurrentItemOnce, QMediaPlaylist::PlaybackMode::CurrentItemOnce},     //
                           {mPlaybackMode_CurrentItemInLoop, QMediaPlaylist::PlaybackMode::CurrentItemInLoop}, //
                           {mPlaybackMode_Sequential, QMediaPlaylist::PlaybackMode::Sequential},
                           {mPlaybackMode_Loop, QMediaPlaylist::PlaybackMode::Loop},
                           {mPlaybackMode_Random, QMediaPlaylist::PlaybackMode::Random}},
                          DEFAULT_PLAYBACK_MODE,
                          QActionGroup::ExclusionPolicy::Exclusive);
  const int playbackModeInt
      = Configuration().value(MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE.name, MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE.v).toInt();
  QMediaPlaylist::PlaybackMode initPlaybackMode = mPlaybackIntAction.intVal2Enum(playbackModeInt);
  mPlaybackIntAction.setCheckedIfActionExist(initPlaybackMode);

  mPlaybackMode = new (std::nothrow) MenuToolButton{mPlaybackIntAction.getActionEnumAscendingList(),
                                                    QToolButton::ToolButtonPopupMode::InstantPopup,
                                                    Qt::ToolButtonStyle::ToolButtonTextBesideIcon};
  mPlaybackMode->SetCaption(QIcon{""}, tr("Playback Mode"), "Change Playback Mode");

  mExtendedFunctionCtrlBar = new (std::nothrow) ToolBarWidget{QBoxLayout::Direction::LeftToRight, this};
  mExtendedFunctionCtrlBar->setVisible(!bBasicMode);
  mExtendedFunctionCtrlBar->addAction(mSelectVideoFolder);
  mExtendedFunctionCtrlBar->addSeparator();
  mExtendedFunctionCtrlBar->addAction(mSeekBackwardHotAct);
  mExtendedFunctionCtrlBar->addAction(mSeekForwardHotAct);
  mExtendedFunctionCtrlBar->addSeparator();
  mExtendedFunctionCtrlBar->addAction(mPlayPrevAct);
  mExtendedFunctionCtrlBar->addAction(mPlayNextAct);
  mExtendedFunctionCtrlBar->addSeparator();
  mExtendedFunctionCtrlBar->addWidget(mPlaybackMode);
  mExtendedFunctionCtrlBar->addAction(mShowFrames);
  mExtendedFunctionCtrlBar->addStretch();
  mExtendedFunctionCtrlBar->addAction(mShowVideoList);

  mExtendLeftWidget = new (std::nothrow) QWidget{this};
  mExtendLeftWidget->setVisible(!bBasicMode);
  mExtendLeftLayout = new (std::nothrow) QVBoxLayout{mExtendLeftWidget};
  mExtendLeftLayout->addWidget(mBasicVideoView);
  mExtendLeftLayout->addWidget(mExtendedFunctionCtrlBar);
  mExtendLeftLayout->setContentsMargins(0, 0, 0, 0);

  mVideoList = new (std::nothrow) VideoTableView{this};
  mVideoList->setPlaybackMode(initPlaybackMode);

  addWidget(mExtendLeftWidget);
  addWidget(mVideoList);
  setContentsMargins(0, 0, 0, 0);

  setWindowIcon(QIcon{":/VideoPlayer/VIDEO_PLAYER"});
  setWindowTitle("Video Player");

  subscribe();
}

VideoView::~VideoView() {
  Configuration().setValue(MemoryKey::VIDEO_PLAYER_PLAYBACK_MODE.name, mPlaybackIntAction.curVal());
}

void VideoView::onReqModeChange(bool bBasicMode) {
  mExtendedFunctionCtrlBar->setVisible(!bBasicMode);
  mVideoList->setVisible(!bBasicMode);
}

void VideoView::subscribe() {
  connect(mBasicVideoView, &BasicVideoView::reqFunctionModeChange, this, &VideoView::onReqModeChange);

  connect(mPlayPrevAct, &QAction::triggered, mVideoList, &VideoTableView::PlayPreviousVideo);
  connect(mPlayNextAct, &QAction::triggered, mVideoList, &VideoTableView::PlayNextVideo);
  connect(mPlaybackIntAction.getActionGroup(), &QActionGroup::triggered, this, &VideoView::onPlaybackModeChanged);
  connect(mShowVideoList, &QAction::toggled, mVideoList, &VideoTableView::setVisible);
  connect(mVideoList, &VideoTableView::reqPlayMedia, mBasicVideoView, &BasicVideoView::PlayAVideo);

  connect(mBasicVideoView->GetVideoWidget(), &PausableVideoWidget::reqToolBarVisibilityChange, this, &VideoView::onChangeToolBarVisibility);
}

void VideoView::onPlaybackModeChanged(const QAction* newPlaybackModeAct) {
  const QMediaPlaylist::PlaybackMode newPlaybackMode = mPlaybackIntAction.act2Enum(newPlaybackModeAct);
  mVideoList->setPlaybackMode(newPlaybackMode);
}

int VideoView::PlayAPath(const QString& folderPath) {
  return mVideoList->setPlayPath(folderPath);
}

int VideoView::PlayVideos(const QString& rootPath, const QStringList& mediafileNames) {
  return mVideoList->setMediaFiles(rootPath, mediafileNames);
}

int VideoView::onSelectAFolder() {
  QString defaultOpenPathLocatedIn
      = Configuration().value(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.v).toString();
  if (!QFile::exists(defaultOpenPathLocatedIn)) {
    defaultOpenPathLocatedIn = SystemPath::HOME_PATH();
  }
  const QString dirSelected = QFileDialog::getExistingDirectory(this,
                                                                "Select a media folder", //
                                                                defaultOpenPathLocatedIn);
  if (dirSelected.isEmpty()) {
    return -1;
  }
  Configuration().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, dirSelected);
  const int mediaFilesCnt = mVideoList->setPlayPath(dirSelected);
  return mediaFilesCnt;
}

void VideoView::StopPlay() {
  if (mBasicVideoView) {
    mBasicVideoView->StopPlay();
  }
}

void VideoView::onChangeToolBarVisibility(bool visibility) {
  mExtendedFunctionCtrlBar->setVisible(visibility);
  if (mShowVideoList->isChecked() != visibility) {
    mShowVideoList->setChecked(visibility);
  }
  mBasicVideoView->movePauseBtnToCenter();
}
