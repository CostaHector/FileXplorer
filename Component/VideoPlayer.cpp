﻿#include "VideoPlayer.h"
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/VideoPlayerActions.h"

#include "Component/Notificator.h"

#include "Tools/Json/JsonHelper.h"
#include "Tools/VideoPlayerWatcher.h"

#include "public/MemoryKey.h"
#include "public/PathTool.h"
#include "public/UndoRedo.h"
#include "public/PublicMacro.h"
#include "public/StyleSheet.h"
#include "public/PublicVariable.h"

#include <QVideoWidget>
#include <QtWidgets>

constexpr int VideoPlayer::MICROSECOND;
const QString VideoPlayer::PLAYLIST_DOCK_TITLE_TEMPLATE{"playlist: %1"};

VideoPlayer::VideoPlayer(QWidget* parent)
    : QMainWindow(parent),
      m_mediaPlayer(new QMediaPlayer(this, QMediaPlayer::LowLatency)),
      m_timeSlider(new ClickableSlider),
      m_volumnSlider(new QSlider(Qt::Orientation::Horizontal, this)),
      m_timeTemplate{"%1|%2"},
      m_timeLabel{new QLabel(m_timeTemplate, this)},
      m_errorLabel(new QLabel),
      m_sliderTB(new QToolBar("slider", this)),
      m_controlTB(g_videoPlayerActions().GetPlayControlToolBar(this, m_timeLabel)),
      m_videoWidget(new QVideoWidget),
      m_probe(new QVideoProbe),
      m_playListModel{new VidModel{this}},
      m_playListWid{new VidsPlayListView{m_playListModel, this}},
      m_playlistSplitter(new QSplitter(Qt::Orientation::Horizontal, this)),
      m_castEditDlg(nullptr),
      m_playerStatusBar{new QStatusBar{this}} {
  m_probe->setSource(m_mediaPlayer);  // Returns true, hopefully.

  m_timeSlider->setRange(0, 0);
  m_timeSlider->reg(std::bind(&QMediaPlayer::setPosition, m_mediaPlayer, std::placeholders::_1));

  m_volumnSlider->setRange(0, 100);
  m_volumnSlider->setMaximumWidth(60);

  onVolumeMute(PreferenceSettings().value(MemoryKey::VIDEO_PLAYER_MUTE.name, MemoryKey::VIDEO_PLAYER_MUTE.v).toBool());

  int logScaleVolume = PreferenceSettings().value(MemoryKey::VIDEO_PLAYER_VOLUME.name, MemoryKey::VIDEO_PLAYER_VOLUME.v).toInt();
  m_volumnSlider->setValue(logScaleVolume);
  onVolumeValueChange(logScaleVolume);

  m_sliderTB->addAction(g_videoPlayerActions()._JUMP_LAST_HOT_SCENE);
  m_sliderTB->addAction(g_videoPlayerActions()._PLAY_PAUSE);
  m_sliderTB->addAction(g_videoPlayerActions()._JUMP_NEXT_HOT_SCENE);
  m_sliderTB->addSeparator();
  m_sliderTB->addWidget(m_timeSlider);
  m_sliderTB->addAction(g_videoPlayerActions()._LAST_10_SECONDS);
  m_sliderTB->addAction(g_videoPlayerActions()._NEXT_10_SECONDS);
  m_sliderTB->addSeparator();
  m_sliderTB->addAction(g_videoPlayerActions()._VOLUME_CTRL_MUTE);
  m_sliderTB->addWidget(m_volumnSlider);

  m_controlTB->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());

  addToolBar(Qt::ToolBarArea::BottomToolBarArea, m_controlTB);
  addToolBarBreak(Qt::ToolBarArea::BottomToolBarArea);
  addToolBar(Qt::ToolBarArea::BottomToolBarArea, m_sliderTB);

  addActions(m_sliderTB->actions());
  addActions(m_controlTB->actions());

  m_playerStatusBar->addWidget(m_errorLabel);
  m_playerStatusBar->setVisible(false);
  setStatusBar(m_playerStatusBar);

  m_playlistSplitter->setOpaqueResize(false);
  m_playlistSplitter->addWidget(m_videoWidget);
  m_playlistSplitter->addWidget(m_playListWid);
  setCentralWidget(m_playlistSplitter);

  subscribe();
  m_watcher = new VideoPlayerWatcher(this, m_videoWidget, m_playListWid);

  updateWindowsSize();
  setWindowTitle("Video Player");
  setWindowIcon(QIcon(":img/VIDEO_PLAYER"));
}

VideoPlayer::~VideoPlayer() {}

bool VideoPlayer::operator()(const QString& path) {
  QFileInfo fi(path);
  if (fi.isFile()) {
    openFile(path);
  } else if (fi.isDir()) {
    openAFolder(path);
  } else {
    return false;
  }
  return true;
}

bool VideoPlayer::operator()(const QStringList& fileAbsPathList) {
  const int rowToPlay = m_playListWid->count();
  const int vidCntDelta = m_playListWid->appendToPlayList(fileAbsPathList);
  if (vidCntDelta <= 0) {
    qDebug("No playable vid(s) find in %d item(s) selectied", fileAbsPathList.size());
    Notificator::warning("No playable vid(s) find", QString("%1 item(s) selectied").arg(fileAbsPathList.size()));
    return true;
  }
  m_playListWid->clearSelection();
  m_playListWid->setCurrentRow(rowToPlay);
  setUrl(QUrl::fromLocalFile(m_playListWid->currentFilePath()));
  play();
  return true;
}

void VideoPlayer::openFile(const QString& filePath) {
  QUrl fileUrl = QUrl::fromLocalFile(filePath);
  if (not QFileInfo(filePath).isFile()) {
    QFileDialog fileDialog(this, "Open Movie", PreferenceSettings().value(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.v).toString());
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    QStringList supportedMimeTypes = m_mediaPlayer->supportedMimeTypes();
    if (!supportedMimeTypes.isEmpty()) {
      fileDialog.setMimeTypeFilters(supportedMimeTypes);
    }
    if (fileDialog.exec() != QDialog::Accepted) {
      return;
    }
    fileUrl = fileDialog.selectedUrls().constFirst();
  }
  PreferenceSettings().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, QFileInfo(fileUrl.toLocalFile()).absolutePath());

  m_playListWid->appendToPlayList({fileUrl.toLocalFile()});
  m_playListWid->clearSelection();
  m_playListWid->setCurrentRow(m_playListWid->count() - 1);

  setUrl(QUrl::fromLocalFile(m_playListWid->currentFilePath()));
  play();
}

void VideoPlayer::openAFolder(const QString& folderPath) {
  QString loadFromPath = folderPath;
  if (not QFileInfo(loadFromPath).isDir()) {
    const QString& loadFromDefaultPath = PreferenceSettings().value(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.v).toString();
    loadFromPath = QFileDialog::getExistingDirectory(this, "load videos from a folder", loadFromDefaultPath);
  }
  QFileInfo loadFromFi(loadFromPath);
  if (not loadFromFi.isDir()) {
    return;
  }
  const int rowToPlay = m_playListWid->count();
  PreferenceSettings().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, loadFromFi.absoluteFilePath());
  const int vidCntDelta = m_playListModel->appendAPath(loadFromPath);
  if (vidCntDelta <= 0) {
    qDebug("No vids find in path[%s]", qPrintable(loadFromPath));
    return;
  }
  m_playListWid->clearSelection();
  m_playListWid->setCurrentRow(rowToPlay);
  setUrl(QUrl::fromLocalFile(m_playListWid->currentFilePath()));
  play();
}

void VideoPlayer::onVolumeMute(const bool isMute) {
  m_mediaPlayer->setMuted(isMute);
  PreferenceSettings().setValue(MemoryKey::VIDEO_PLAYER_MUTE.name, isMute);
  if (isMute) {
    g_videoPlayerActions()._VOLUME_CTRL_MUTE->setIcon(QIcon(":img/VOLUME_MUTE"));
  } else {
    g_videoPlayerActions()._VOLUME_CTRL_MUTE->setIcon(QIcon(":img/VOLUME_UNMUTE"));
  }
}
void VideoPlayer::onVolumeValueChange(const int logScaleValue) {
  // Set slider axis is logarithmic scale while videoplayer use linear scale
  qreal linearVolume = 100 * QAudio::convertVolume(logScaleValue / qreal(100.0), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale);
  m_mediaPlayer->setVolume(linearVolume);
  qDebug("logarithmic:%d, linear:%f", logScaleValue, linearVolume);
  QToolTip::showText(cursor().pos(), "Volume:" + QString::number(linearVolume));
  PreferenceSettings().setValue(MemoryKey::VIDEO_PLAYER_VOLUME.name, logScaleValue);
}

void VideoPlayer::updateWindowsSize() {
  if (PreferenceSettings().contains("VideoPlayerGeometry")) {
    restoreGeometry(PreferenceSettings().value("VideoPlayerGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  m_playlistSplitter->restoreState(PreferenceSettings().value("VideoPlayerSplitterState", QByteArray()).toByteArray());
}

void VideoPlayer::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void VideoPlayer::closeEvent(QCloseEvent* event) {
  setUrl({});
  PreferenceSettings().setValue("VideoPlayerGeometry", saveGeometry());
  qDebug("Video Player geometry was resize to (%d, %d, %d, %d)", geometry().x(), geometry().y(), geometry().width(), geometry().height());
  PreferenceSettings().setValue("VideoPlayerSplitterState", m_playlistSplitter->saveState());
  QMainWindow::closeEvent(event);
}

void VideoPlayer::keyPressEvent(QKeyEvent* e) {
  if (e->modifiers() == Qt::AltModifier and (e->key() == Qt::Key_Enter or e->key() == Qt::Key_Return)) {
    m_playListWid->hide();
    m_sliderTB->show();
    m_controlTB->hide();
    setWindowState(Qt::WindowMaximized);
    return;
  } else if (e->key() == Qt::Key_Escape) {
    m_playListWid->show();
    m_sliderTB->show();
    m_controlTB->show();
    setWindowState(Qt::WindowMaximized);
    return;
  } else if (e->key() == Qt::Key_F11) {
    m_playListWid->hide();
    m_sliderTB->hide();
    m_controlTB->hide();
    setWindowState(Qt::WindowFullScreen);
    return;
  } else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
    setUrl(QUrl::fromLocalFile(m_playListWid->currentFilePath()));
    play();
    return;
  } else if (e->key() == Qt::Key_Space) {
    bool beforeChecked = g_videoPlayerActions()._PLAY_PAUSE->isChecked();
    g_videoPlayerActions()._PLAY_PAUSE->setChecked(not beforeChecked);
    emit g_videoPlayerActions()._PLAY_PAUSE->triggered(not beforeChecked);
    return;
  }
  QWidget::keyPressEvent(e);
}

int VideoPlayer::onRecycleSelectedItems() {
  if (not m_playListWid->selectionModel()->hasSelection()) {
    qDebug("Recycle skip. Select before delete.");
    Notificator::information("Recycle skip", "Select before delete");
    return 0;
  }

  const QStringList& rmvFilesLst = m_playListModel->getToRemoveFileList(m_playListWid->selectionModel()->selectedIndexes());
  const int nextPlayRow = m_playListModel->getNextAvailableVidUrl(m_playingUrl, m_playListWid->selectionModel()->selectedIndexes());

  if (nextPlayRow == -1) {  // nothing playable found
    setUrl({});
  } else {
    const QUrl nextPlayUrl{QUrl::fromLocalFile(m_playListModel->filePath(nextPlayRow))};
    if (nextPlayUrl != m_playingUrl and nextPlayUrl.isLocalFile()) {
      // to recycle, play first unselected and available vid.
      m_playListWid->setCurrentRow(nextPlayRow);
      setUrl(nextPlayUrl);
      if (g_videoPlayerActions()._AUTO_PLAY_NEXT_VIDEO->isChecked()) {
        play();
      }
    }
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE recycleCmds;
  recycleCmds.reserve(rmvFilesLst.size());
  for (const QString& pth : rmvFilesLst) {
    QFileInfo fi(pth);
    recycleCmds.append(ACMD::GetInstMOVETOTRASH(fi.absolutePath(), fi.fileName()));
  }

  if (recycleCmds.isEmpty()) {
    qDebug("Skip Recycle. No file need to recycle");
    Notificator::goodNews("Recycle succeed", "No file need to recycle");
    return 0;
  }
  bool recycleRet = g_undoRedo.Do(recycleCmds);
  if (recycleRet) {
    qDebug("Recycle succeed. %d files", recycleCmds.size());
    Notificator::goodNews("Recycle succeed", QString("%1 files").arg(recycleCmds.size()));
  } else {
    qWarning("Some recycle failed. %d files", recycleCmds.size());
    Notificator::badNews("Error", QString("%1 files Recycle Failed").arg(recycleCmds.size()));
  }
  m_playListModel->whenFilesDeleted(m_playListWid->selectionModel()->selection());
  return recycleCmds.size();
}

void VideoPlayer::setUrl(const QUrl& url) {
  m_playingUrl = url;
  m_errorLabel->setText(QString());
  if (url.isLocalFile()) {
    const QString& vidsPath = url.toLocalFile();
    setWindowFilePath(vidsPath);
    const QString& jsonPath = PathTool::FileExtReplacedWithJson(vidsPath);
    m_dict = JsonHelper::MovieJsonLoader(jsonPath);
  } else {
    m_dict.clear();
  }
  loadVideoRate();
  loadHotSceneList();
  g_videoPlayerActions()._PLAY_PAUSE->setEnabled(url.isLocalFile());
  if (not m_mediaPlayer->isVideoAvailable()) {
    // This property holds the video availability status for the current media.
    // If available, the QVideoWidget class can be used to view the video.
    m_mediaPlayer->setVideoOutput(m_videoWidget);
  }
  m_mediaPlayer->setMedia(url);
}

void VideoPlayer::loadVideoRate() {
  if (m_dict.contains(ENUM_2_STR(Rate))) {
    bool isInt = false;
    int rate = m_dict[ENUM_2_STR(Rate)].toInt(&isInt);
    if (isInt && 0 <= rate && rate < g_videoPlayerActions()._RATE_LEVEL_COUNT) {
      g_videoPlayerActions()._RATE_AG->actions()[rate]->setChecked(true);
    }
    return;
  }
  auto* checkedAct = g_videoPlayerActions()._RATE_AG->checkedAction();
  if (checkedAct) {
    checkedAct->setChecked(false);
  }
}

QString VideoPlayer::JsonFileValidCheck(const QString& op) {
  if (!m_playListWid->currentIndex().isValid()) {
    qDebug("current item is nullptr, cannot %s", qPrintable(op));
    return {};
  }
  const QString& jsonPath = PathTool::FileExtReplacedWithJson(m_playListWid->currentFilePath());
  if (not QFile::exists(jsonPath)) {
    qDebug("json file[%s] not exists. cannot %s", qPrintable(jsonPath), qPrintable(op));
    return {};
  }
  return jsonPath;
}

void VideoPlayer::subscribe() {
  connect(g_videoPlayerActions()._VOLUME_CTRL_MUTE, &QAction::triggered, this, &VideoPlayer::onVolumeMute);
  connect(m_volumnSlider, &QSlider::valueChanged, this, &VideoPlayer::onVolumeValueChange);

  connect(m_playListWid, &QListView::doubleClicked, this, &VideoPlayer::onListWidgetDoubleClicked);

  connect(g_videoPlayerActions()._JUMP_NEXT_HOT_SCENE, &QAction::triggered, this, &VideoPlayer::onJumpToNextHotScene);
  connect(g_videoPlayerActions()._JUMP_LAST_HOT_SCENE, &QAction::triggered, this, &VideoPlayer::onJumpToLastHotScene);

  connect(g_videoPlayerActions()._LAST_10_SECONDS, &QAction::triggered, this, [this]() { onPositionAdd(-10 * 1000); });
  connect(g_videoPlayerActions()._NEXT_10_SECONDS, &QAction::triggered, this, [this]() { onPositionAdd(10 * 1000); });

  connect(g_videoPlayerActions()._RATE_AG, &QActionGroup::triggered, this, &VideoPlayer::onRateForThisMovie);

  connect(g_videoPlayerActions()._MARK_HOT_SCENE, &QAction::triggered, this, &VideoPlayer::onMarkHotScenes);

  connect(g_videoPlayerActions()._GRAB_FRAME, &QAction::triggered, this, [this]() -> void {
    connect(m_probe, &QVideoProbe::videoFrameProbed, this, &VideoPlayer::onGrabAFrame);
    qDebug("current %lld (ms)", m_mediaPlayer->position());
  });

  connect(g_videoPlayerActions()._RENAME_VIDEO, &QAction::triggered, this, &VideoPlayer::onModeName);
  connect(g_videoPlayerActions()._MOD_CAST, &QAction::triggered, this, &VideoPlayer::onModCast);

  connect(g_videoPlayerActions()._NEXT_VIDEO, &QAction::triggered, this, &VideoPlayer::onPlayNextVideo);
  connect(g_videoPlayerActions()._LAST_VIDEO, &QAction::triggered, this, &VideoPlayer::onPlayLastVideo);
  connect(g_videoPlayerActions()._AUTO_PLAY_NEXT_VIDEO, &QAction::triggered, [](const bool checked) { PreferenceSettings().setValue(MemoryKey::AUTO_PLAY_NEXT_VIDEO.name, checked); });

  connect(g_videoPlayerActions()._OPEN_A_VIDEO, &QAction::triggered, this, [this]() { openFile(); });
  connect(g_videoPlayerActions()._LOAD_A_PATH, &QAction::triggered, this, [this]() { openAFolder(); });

  connect(g_videoPlayerActions()._PLAY_PAUSE, &QAction::triggered, this, &VideoPlayer::play);

  connect(g_videoPlayerActions()._SHOW_VIDEOS_LIST, &QAction::triggered, this, &VideoPlayer::onShowPlaylist);
  connect(g_videoPlayerActions()._CLEAR_VIDEOS_LIST, &QAction::triggered, this, &VideoPlayer::onClearPlaylist);

  connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &VideoPlayer::mediaStateChanged);

  connect(m_timeSlider, &QAbstractSlider::sliderMoved, this, &VideoPlayer::onSetPlayerPosition);
  connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::onPlayerPositionChanged);

  connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
  connect(m_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &VideoPlayer::handleError);

  connect(g_videoPlayerActions()._SCROLL_TO_LAST_FOLDER, &QAction::triggered, this, &VideoPlayer::onScrollToLastFolder);
  connect(g_videoPlayerActions()._SCROLL_TO_NEXT_FOLDER, &QAction::triggered, this, &VideoPlayer::onScrollToNextFolder);

  connect(m_playlistSplitter, &QSplitter::splitterMoved, this, [](int pos, int index) -> void { qDebug("pos %d, index %d", pos, index); });

  connect(g_videoPlayerActions()._MOVE_SELECTED_ITEMS_TO_TRASHBIN, &QAction::triggered, this, &VideoPlayer::onRecycleSelectedItems);
}

bool VideoPlayer::onModeName() {
  if (!m_playListWid->currentIndex().isValid()) {
    qInfo("Skip nothing was selected to rename");
    return true;
  }
  const QFileInfo vidFi{m_playListWid->currentFilePath()};
  const QFileInfo jsonFi{PathTool::FileExtReplacedWithJson(vidFi.absoluteFilePath())};
  if (not vidFi.exists()) {
    return true;
  }

  bool okClicked = false;
  const QString& newFileName = QInputDialog::getItem(this, "Rename Videos", vidFi.absolutePath(), {vidFi.fileName()}, 0, true, &okClicked);
  if (not okClicked or newFileName.isEmpty()) {
    qInfo("Skip User cancel rename");
    return false;
  }
  if (vidFi.fileName() == newFileName) {
    return true;
  }
  const auto currentPosition = m_mediaPlayer->position();
  setUrl({});

  const QString& newFileAbsPath = vidFi.absoluteDir().absoluteFilePath(newFileName);
  const bool renameResult = QFile::rename(vidFi.absoluteFilePath(), newFileAbsPath);

  if (!renameResult) {
    const QString& msg = QString("Rename [%1] -> [%2] failed").arg(vidFi.fileName()).arg(newFileName);
    qCritical("Result: %s", qPrintable(msg));
    QMessageBox::critical(this, "Rename failed", msg, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
    setUrl(QUrl::fromLocalFile(vidFi.absoluteFilePath()));
  } else {
    // rename vid ok, rename json now if exist
    if (jsonFi.exists()) {
      const QString& newJsonAbsPath = PathTool::FileExtReplacedWithJson(newFileAbsPath);
      const bool renameJsonResult = QFile::rename(jsonFi.absoluteFilePath(), newJsonAbsPath);
      if (!renameJsonResult) {
        const QString& msg = QString("Rename Json [%1] -> [%2] failed").arg(vidFi.fileName()).arg(newFileName);
        qCritical("Result: %s", qPrintable(msg));
        QMessageBox::critical(this, "Rename Json failed", msg, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
      }
    }
    m_playListModel->setData(m_playListWid->currentIndex(), newFileAbsPath, Qt::DisplayRole);
    setUrl(QUrl::fromLocalFile(newFileAbsPath));
  }
  m_mediaPlayer->setPosition(currentPosition);
  play();
  return renameResult;
}

bool VideoPlayer::onModCast() {
  const QString& jsonPath = JsonFileValidCheck("mod cast");
  if (jsonPath.isEmpty()) {
    qWarning("No corresponding json file find, cannot edit");
    return false;
  }
  if (!m_castEditDlg) {
    m_castEditDlg = new (std::nothrow) JsonPerformersListInputer(this);
    CHECK_NULLPTR_RETURN_FALSE(m_castEditDlg);
  }
  m_castEditDlg->reloadPerformersFromJsonFile(jsonPath, m_dict);
  m_castEditDlg->show();
  return true;
}

bool VideoPlayer::onGrabAFrame(const QVideoFrame& frame) {
  disconnect(m_probe, &QVideoProbe::videoFrameProbed, nullptr, nullptr);
  if (not m_playListWid->currentIndex().isValid()) {
    qInfo("[Skip] Nothing select to grab an image");
    return false;
  }
  const auto& img = frame.image();
  if (img.isNull()) {
    qWarning("image is null");
    return false;
  }
  const int seconds = m_timeSlider->value() / MICROSECOND;
  const QFileInfo fi(m_playListWid->currentFilePath());
  const QString& imgAbsPath = QString("%1/%2 %3.png").arg(fi.absolutePath()).arg(fi.completeBaseName()).arg(seconds, DURATION_PLACEHOLDER_LENGTH, 10, QChar('0'));
  qDebug("Grabbed image named: %s", qPrintable(imgAbsPath));
  const auto& outputImage = img.mirrored(false, true);
  const bool ret = outputImage.save(imgAbsPath);
  qDebug("Grab succeed %d", ret);
  return ret;
}

bool VideoPlayer::onMarkHotScenes() {
  const QString& jsonPath = JsonFileValidCheck("mark hot scenes");
  if (jsonPath.isEmpty()) {
    return false;
  }
  int hotPos = m_mediaPlayer->position();
  qDebug("Mark hot scene position %d (ms)", hotPos);
  m_hotSceneList.append(hotPos);
  std::sort(m_hotSceneList.begin(), m_hotSceneList.end());

  QList<QVariant> hotVariantList(m_hotSceneList.cbegin(), m_hotSceneList.cend());
  m_dict.insert(ENUM_2_STR(Hot), hotVariantList);
  bool dumpRet = JsonHelper::DumpJsonDict(m_dict, jsonPath);
  qDebug("Mark result: %d", dumpRet);
  return dumpRet;
}

void VideoPlayer::loadHotSceneList() {
  m_hotSceneList.clear();
  if (m_dict.contains(ENUM_2_STR(Hot))) {
    for (const QVariant& pos : m_dict[ENUM_2_STR(Hot)].toList()) {
      m_hotSceneList.append(pos.toInt());
    }
  }
}

bool VideoPlayer::onJumpToNextHotScene() {
  if (m_hotSceneList.isEmpty()) {
    qInfo("[Skip] Hot scenes list is empty");
    return false;
  }
  int tar = m_mediaPlayer->position();
  for (const int hotPos : m_hotSceneList) {
    if (hotPos > tar) {
      m_mediaPlayer->setPosition(hotPos);
      return true;
    }
  }
  qDebug("no next hot scene after %d seconds", tar);
  return false;
}

bool VideoPlayer::onJumpToLastHotScene() {
  if (m_hotSceneList.isEmpty()) {
    qDebug("empty hot scenes list");
    return false;
  }
  int tar = m_mediaPlayer->position();
  for (auto itHotPos = m_hotSceneList.crbegin(); itHotPos != m_hotSceneList.crend(); ++itHotPos) {
    if (*itHotPos > tar) {
      m_mediaPlayer->setPosition(*itHotPos);
      return true;
    }
  }
  qDebug("no last hot scene before %d seconds", tar);
  return false;
}

bool VideoPlayer::onPositionAdd(const int ms) {
  auto after = m_mediaPlayer->position() + ms;
  const bool autoNextEnabled = g_videoPlayerActions()._AUTO_PLAY_NEXT_VIDEO->isChecked();
  if (after < 0) {
    if (autoNextEnabled) {
      onPlayLastVideo();
      return true;
    }
    after = 0;
  } else if (after >= m_mediaPlayer->duration()) {
    if (autoNextEnabled) {
      onPlayNextVideo();
      return true;
    }
    after = m_mediaPlayer->duration();
  }
  m_mediaPlayer->setPosition(after);
  return true;
}

bool VideoPlayer::onRateForThisMovie(const QAction* checkedAction) {
  qDebug("Rate: %s score.", qPrintable(checkedAction->text()));
  const QString& jsonPath = JsonFileValidCheck("rate movie");
  if (jsonPath.isEmpty()) {
    return false;
  }
  int score = checkedAction->text().back().toLatin1() - '0';
  m_dict.insert(ENUM_2_STR(Rate), score);
  bool dumpRet = JsonHelper::DumpJsonDict(m_dict, jsonPath);
  qDebug("Rate result: %d", dumpRet);
  return dumpRet;
}

void VideoPlayer::onPlayLastVideo() {
  if (m_playListWid->count() == 0) {
    qDebug("playlist is empty");
    return;
  }
  int lastRow = m_playListWid->currentRow() - 1;
  if (lastRow < 0) {
    qDebug("next is to the last");
    lastRow = m_playListWid->count() - 1;
  }
  m_playListWid->setCurrentRow(lastRow);
  onListWidgetDoubleClicked(m_playListWid->currentIndex());
}

void VideoPlayer::onPlayNextVideo() {
  if (m_playListWid->count() == 0) {
    qDebug("playlist is empty");
    return;
  }
  int nextRow = m_playListWid->currentRow() + 1;
  if (nextRow >= m_playListWid->count()) {
    qDebug("next is to the first");
    nextRow = 0;
  }
  m_playListWid->setCurrentRow(nextRow);
  onListWidgetDoubleClicked(m_playListWid->currentIndex());
}

void VideoPlayer::onListWidgetDoubleClicked(const QModelIndex& ind) {
  const QString& clickedFilePath = m_playListWid->filePath(ind.row());
  if (not QFile::exists(clickedFilePath)) {
    qWarning("Cannot play not exists video[%s]", qPrintable(clickedFilePath));
    Notificator::badNews("Cannot play", QString("Video[%1] not exists").arg(clickedFilePath));
    return;
  }
  setUrl(QUrl::fromLocalFile(clickedFilePath));
  play();
}

void VideoPlayer::onScrollToAnotherFolder(int inc) {
  if (m_playListWid->count() == 0) {
    qDebug("playlist is empty");
    return;
  }
  static auto getDirName = [](const QString& filePath) -> QString { return QFileInfo(QFileInfo(filePath).absolutePath()).fileName(); };

  const QString& beforeDirName = getDirName(m_playListWid->currentFilePath());
  for (int i = m_playListWid->currentRow() + inc; 0 <= i and i < m_playListWid->count(); i += inc) {
    const QString& newDirName = m_playListWid->filePath(i);
    if (newDirName != beforeDirName) {
      const QModelIndex& ind = m_playListModel->index(i, 0);
      m_playListWid->setCurrentIndex(ind);
      m_playListWid->scrollTo(ind);
      onListWidgetDoubleClicked(ind);
      qDebug("Scroll to the %dth item, a folder named [%s]", i, qPrintable(newDirName));
      return;
    }
  }
  if (inc > 0) {
    m_playListWid->scrollToTop();
    qDebug("[%s] is already the first folder", qPrintable(beforeDirName));
    Notificator::information("Scroll <<", QString("[%1] is already the first folder").arg(beforeDirName));
  } else if (inc < 0) {
    m_playListWid->scrollToBottom();
    qDebug("[%s] is already the last folder", qPrintable(beforeDirName));
    Notificator::information("Scroll >>", QString("[%1] is already the last folder").arg(beforeDirName));
  }
}

void VideoPlayer::onShowPlaylist(bool keepShow) {
  PreferenceSettings().setValue(MemoryKey::KEEP_VIDEOS_PLAYLIST_SHOW.name, keepShow);
  m_playListWid->setVisible(keepShow);
  if (keepShow) {
    if (m_playListWid->currentIndex().isValid()) {
      QTimer::singleShot(100, [this]() { m_playListWid->scrollTo(m_playListWid->currentIndex()); });
    }
  }
  dynamic_cast<VideoPlayerWatcher*>(m_watcher)->setKeepListShow(keepShow);
}

void VideoPlayer::onClearPlaylist() {
  m_playListModel->clear();
}

void VideoPlayer::play() {
  switch (m_mediaPlayer->state()) {
    case QMediaPlayer::PlayingState:
      m_mediaPlayer->pause();
      break;
    default:
      m_mediaPlayer->play();
      break;
  }
}

void VideoPlayer::mediaStateChanged(QMediaPlayer::State state) {
  switch (state) {
    case QMediaPlayer::PlayingState:
      g_videoPlayerActions()._PLAY_PAUSE->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
      break;
    default:
      g_videoPlayerActions()._PLAY_PAUSE->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
      break;
  }
}

void VideoPlayer::onSetPlayerPosition(int position) {
  m_mediaPlayer->setPosition(position);
}

void VideoPlayer::onPlayerPositionChanged(qint64 position) {
  m_timeLabel->setText(m_timeTemplate.arg(MillionSecond2hhmmss(position)));
  m_timeSlider->setValue(position);
  if (position > 0 and position == m_timeSlider->maximum() and g_videoPlayerActions()._AUTO_PLAY_NEXT_VIDEO->isChecked()) {
    onPositionAdd(1);
  }
}

void VideoPlayer::durationChanged(qint64 duration) {
  qInfo("Duration changed to %lld", duration);
  m_timeSlider->setRange(0, duration);
  m_timeTemplate = "%1|" + MillionSecond2hhmmss(duration);
}

void VideoPlayer::handleError() {
  g_videoPlayerActions()._PLAY_PAUSE->setEnabled(false);
  const QString errorString = m_mediaPlayer->errorString();
  QString message = "Error: ";
  if (errorString.isEmpty())
    message += " #" + QString::number(int(m_mediaPlayer->error()));
  else
    message += errorString;
  m_errorLabel->setText(message);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  VideoPlayer player;
  player.show();
  player("E:/115/0419");
  a.exec();
  return 0;
}
#endif
