#include "VideoPlayer.h"
#include "Actions/VideoPlayerActions.h"
#include "PublicVariable.h"
#include "Tools/JsonFileHelper.h"

#include <QVideoWidget>
#include <QtWidgets>

constexpr int VideoPlayer::MICROSECOND;
const QString VideoPlayer::PLAYLIST_DOCK_TITLE_TEMPLATE{"playlist: %1"};

VideoPlayer::VideoPlayer(QWidget* parent)
    : QMainWindow(parent),
      m_slider(new ClickableSlider),
      m_timeLabel(new QLabel("00000")),
      m_errorLabel(new QLabel),
      m_sliderTB(new QToolBar("slider", this)),
      m_controlTB(new QToolBar("play control", this)),
      m_videoWidget(new QVideoWidget),
      m_probe(new QVideoProbe),
      m_playListWid(new QListWidget),
      m_playlistDock(new QDockWidget("playlist", this)),
      m_performerWid(new PerformersWidget(this)),
      m_playListMenu(new QMenu("playList", this)) {
  m_mediaPlayer = new QMediaPlayer(this, QMediaPlayer::LowLatency);
  m_mediaPlayer->setVideoOutput(m_videoWidget);
  m_probe->setSource(m_mediaPlayer);  // Returns true, hopefully.

  m_playListWid->setContextMenuPolicy(Qt::CustomContextMenu);
  m_playListMenu->addAction(g_videoPlayerActions()._REVEAL_IN_EXPLORER);

  m_slider->setRange(0, 0);
  m_slider->reg(std::bind(&QMediaPlayer::setPosition, m_mediaPlayer, std::placeholders::_1));

  m_sliderTB->addAction(g_videoPlayerActions()._JUMP_LAST_HOT_SCENE);
  m_sliderTB->addAction(g_videoPlayerActions()._PLAY_PAUSE);
  m_sliderTB->addAction(g_videoPlayerActions()._JUMP_NEXT_HOT_SCENE);
  m_sliderTB->addWidget(m_slider);

  auto* spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  m_controlTB->addAction(g_videoPlayerActions()._LAST_VIDEO);
  m_controlTB->addAction(g_videoPlayerActions()._NEXT_VIDEO);
  m_controlTB->addAction(g_videoPlayerActions()._OPEN_A_VIDEO);
  m_controlTB->addSeparator();
  m_controlTB->addWidget(m_timeLabel);
  m_controlTB->addSeparator();
  m_controlTB->addAction(g_videoPlayerActions()._MARK_HOT_SCENE);
  m_controlTB->addAction(g_videoPlayerActions()._GRAB_FRAME);
  m_controlTB->addSeparator();
  m_controlTB->addAction(g_videoPlayerActions()._RENAME_VIDEO);
  m_controlTB->addAction(g_videoPlayerActions()._MOD_PERFORMERS);
  m_controlTB->addSeparator();
  m_controlTB->addActions(g_videoPlayerActions()._RATE_AG->actions());
  m_controlTB->addSeparator();
  m_controlTB->addWidget(spacer);
  m_controlTB->addSeparator();
  m_controlTB->addAction(g_videoPlayerActions()._CLEAR_VIDEOS_LIST);
  m_controlTB->addAction(g_videoPlayerActions()._LOAD_A_PATH);
  m_controlTB->addAction(g_videoPlayerActions()._VIDEOS_LIST_MENU);
  m_controlTB->setContentsMargins(0, 0, 0, 0);

  addToolBar(Qt::ToolBarArea::BottomToolBarArea, m_controlTB);
  addToolBarBreak(Qt::ToolBarArea::BottomToolBarArea);
  addToolBar(Qt::ToolBarArea::BottomToolBarArea, m_sliderTB);

  auto* _sb = new QStatusBar(this);
  _sb->addWidget(m_errorLabel);
  _sb->setVisible(false);
  setStatusBar(_sb);

  m_playlistDock->setWidget(m_playListWid);
  addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_playlistDock);

  setCentralWidget(m_videoWidget);

  subscribe();

  setWindowIcon(QIcon(":/themes/VIDEO_PLAYER"));
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

void VideoPlayer::openFile(const QString& filePath) {
  QUrl fileUrl = QUrl::fromLocalFile(filePath);
  if (not QFileInfo(filePath).isFile()) {
    QFileDialog fileDialog(
        this, "Open Movie",
        PreferenceSettings().value(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.v).toString());
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
  m_playListWid->addItem(fileUrl.toLocalFile());
  setUrl(fileUrl);
  play();
}

void VideoPlayer::setUrl(const QUrl& url) {
  m_errorLabel->setText(QString());
  if (url.isLocalFile()) {
    const QString& vidsPath = url.toLocalFile();
    setWindowFilePath(vidsPath);
    const QString& jsonPath = GetJsonFilePath(vidsPath);
    m_dict = JsonFileHelper::MovieJsonLoader(jsonPath);
  } else {
    m_dict.clear();
  }
  loadVideoRate();
  loadHotSceneList();
  g_videoPlayerActions()._PLAY_PAUSE->setEnabled(url.isLocalFile());
  m_mediaPlayer->setMedia(url);
}

auto VideoPlayer::loadVideoRate() -> void {
  if (m_dict.contains(JSONKey::Rate)) {
    bool isInt = false;
    int rate = m_dict[JSONKey::Rate].toInt(&isInt);
    if (isInt and 0 <= rate and rate < g_videoPlayerActions()._RATE_LEVEL_COUNT) {
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
  if (not m_playListWid->currentItem()) {
    qDebug("current item is nullptr, cannot %s", op.toStdString().c_str());
    return {};
  }
  const QString& jsonPath = GetJsonFilePath(m_playListWid->currentItem()->text());
  if (not QFile::exists(jsonPath)) {
    qDebug("json file[%s] not exists. cannot %s", jsonPath.toStdString().c_str(), op.toStdString().c_str());
    return {};
  }
  return jsonPath;
}

void VideoPlayer::subscribe() {
  connect(g_videoPlayerActions()._REVEAL_IN_EXPLORER, &QAction::triggered, this, [this]() {
    if (m_playListWid->currentItem())
      QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(m_playListWid->currentItem()->text()).absolutePath()));
  });
  connect(m_playListWid, &QListView::customContextMenuRequested, this, [this](const QPoint pnt) {
    m_playListMenu->popup(m_playListWid->mapToGlobal(pnt));  // or QCursor::pos()
  });

  connect(m_playListWid, &QListWidget::itemDoubleClicked, this, [this](const QListWidgetItem* item) {
    setUrl(QUrl::fromLocalFile(item->text()));
    play();
  });

  connect(g_videoPlayerActions()._JUMP_NEXT_HOT_SCENE, &QAction::triggered, this, &VideoPlayer::onJumpToNextHotScene);
  connect(g_videoPlayerActions()._JUMP_LAST_HOT_SCENE, &QAction::triggered, this, &VideoPlayer::onJumpToLastHotScene);
  connect(g_videoPlayerActions()._RATE_AG, &QActionGroup::triggered, this, &VideoPlayer::onRateForThisMovie);

  connect(g_videoPlayerActions()._MARK_HOT_SCENE, &QAction::triggered, this, &VideoPlayer::onMarkHotScenes);

  connect(g_videoPlayerActions()._GRAB_FRAME, &QAction::triggered, this, [this]() -> void {
    connect(m_probe, &QVideoProbe::videoFrameProbed, this, &VideoPlayer::onGrabAFrame);
    qDebug("current %d (ms)", m_mediaPlayer->position());
  });

  connect(g_videoPlayerActions()._RENAME_VIDEO, &QAction::triggered, this, &VideoPlayer::onModeName);
  connect(g_videoPlayerActions()._MOD_PERFORMERS, &QAction::triggered, this, &VideoPlayer::onModPerformers);

  connect(g_videoPlayerActions()._NEXT_VIDEO, &QAction::triggered, this, &VideoPlayer::onPlayNextVideo);
  connect(g_videoPlayerActions()._LAST_VIDEO, &QAction::triggered, this, &VideoPlayer::onPlayLastVideo);

  connect(g_videoPlayerActions()._OPEN_A_VIDEO, &QAction::triggered, this, [this](){openFile();});
  connect(g_videoPlayerActions()._LOAD_A_PATH, &QAction::triggered, this, [this](){openAFolder();});

  connect(g_videoPlayerActions()._PLAY_PAUSE, &QAction::triggered, this, &VideoPlayer::play);

  connect(g_videoPlayerActions()._VIDEOS_LIST_MENU, &QAction::triggered, this, &VideoPlayer::onShowPlaylist);
  connect(g_videoPlayerActions()._CLEAR_VIDEOS_LIST, &QAction::triggered, this, &VideoPlayer::onClearPlaylist);

  connect(m_slider, &QAbstractSlider::sliderMoved, this, &VideoPlayer::setPosition);

  connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &VideoPlayer::mediaStateChanged);
  connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);

  connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
  connect(m_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &VideoPlayer::handleError);
}

bool VideoPlayer::onModeName() {
  if (not m_playListWid->currentItem()) {
    qDebug("cannot rename nothing select");
    return true;
  }
  const QFileInfo vidFi(m_playListWid->currentItem()->text());
  const QFileInfo jsonFi(GetJsonFilePath(vidFi.absoluteFilePath()));
  if (not vidFi.exists()) {
    return true;
  }

  bool okClicked = false;
  const QString& newFileName = QInputDialog::getItem(this, "Rename Videos", vidFi.absolutePath(), {vidFi.fileName()}, 0, true, &okClicked);
  if (not okClicked or newFileName.isEmpty()) {
    qDebug("Cancel rename");
    return false;
  }
  if (vidFi.fileName() == newFileName) {
    return true;
  }
  const auto currentPosition = m_mediaPlayer->position();
  setUrl({});

  const QString& newFileAbsPath = vidFi.absoluteDir().absoluteFilePath(newFileName);
  const bool renameResult = QFile::rename(vidFi.absoluteFilePath(), newFileAbsPath);

  if (not renameResult) {
    const QString& msg = QString("Rename [%1] -> [%2] failed").arg(vidFi.fileName()).arg(newFileName);
    qDebug("%s", msg.toStdString().c_str());
    QMessageBox::warning(this, "Rename failed", msg, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
    setUrl(QUrl::fromLocalFile(vidFi.absoluteFilePath()));
  } else {
    if (jsonFi.exists()) {
      const QString& newJsonAbsPath = GetJsonFilePath(newFileAbsPath);
      const bool renameJsonResult = QFile::rename(jsonFi.absoluteFilePath(), newJsonAbsPath);
      if (not renameJsonResult) {
        const QString& msg = QString("Rename Json [%1] -> [%2] failed").arg(vidFi.fileName()).arg(newFileName);
        QMessageBox::warning(this, "Rename Json failed", msg, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
      }
    }
    m_playListWid->currentItem()->setText(newFileAbsPath);
    setUrl(QUrl::fromLocalFile(newFileAbsPath));
  }
  m_mediaPlayer->setPosition(currentPosition);
  play();
  return renameResult;
}

bool VideoPlayer::onModPerformers() {
  const QString& jsonPath = JsonFileValidCheck("mod performers");
  if (jsonPath.isEmpty()) {
    return false;
  }
  m_performerWid->reloadPerformersFromJsonFile(jsonPath, m_dict);
  m_performerWid->show();
  return true;
}

bool VideoPlayer::onGrabAFrame(const QVideoFrame& frame) {
  disconnect(m_probe, &QVideoProbe::videoFrameProbed, nullptr, nullptr);
  if (not m_playListWid->currentItem()) {
    qDebug("nothing select to grab image");
    return false;
  }
  const auto& img = frame.image();
  if (img.isNull()) {
    qDebug("image is null");
    return false;
  }
  const int seconds = m_slider->value() / MICROSECOND;
  const QFileInfo fi(m_playListWid->currentItem()->text());
  const QString& imgAbsPath =
      QString("%1/%2 %3.png").arg(fi.absolutePath()).arg(fi.completeBaseName()).arg(seconds, DURATION_PLACEHOLDER_LENGTH, 10, QChar('0'));
  qDebug("Grabbed image named: %s", imgAbsPath.toStdString().c_str());
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
  m_dict.insert(JSONKey::Hot, hotVariantList);
  bool dumpRet = JsonFileHelper::MovieJsonDumper(m_dict, jsonPath);
  qDebug("Mark result: %d", dumpRet);
  return dumpRet;
}

auto VideoPlayer::loadHotSceneList() -> void {
  m_hotSceneList.clear();
  if (m_dict.contains(JSONKey::Hot)) {
    for (const QVariant& pos : m_dict[JSONKey::Hot].toList()) {
      m_hotSceneList.append(pos.toInt());
    }
  }
}

auto VideoPlayer::onJumpToNextHotScene() -> bool {
  if (m_hotSceneList.isEmpty()) {
    qDebug("empty hot scenes list");
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

auto VideoPlayer::onJumpToLastHotScene() -> bool {
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

auto VideoPlayer::onRateForThisMovie(const QAction* checkedAction) -> bool {
  qDebug("Rate: %s score.", checkedAction->text().toStdString().c_str());
  const QString& jsonPath = JsonFileValidCheck("rate movie");
  if (jsonPath.isEmpty()) {
    return false;
  }
  int score = checkedAction->text().back().toLatin1() - '0';
  m_dict.insert(JSONKey::Rate, score);
  bool dumpRet = JsonFileHelper::MovieJsonDumper(m_dict, jsonPath);
  qDebug("Rate result: %d", dumpRet);
  return dumpRet;
}

void VideoPlayer::onPlayLastVideo() {
  if (m_playListWid->count() == 0) {
    return;
  }
  int lastRow = m_playListWid->currentRow() - 1;
  if (lastRow < 0) {
    qDebug("next is to the last");
    lastRow = m_playListWid->count() - 1;
  }
  m_playListWid->setCurrentRow(lastRow);
  setUrl(QUrl::fromLocalFile(m_playListWid->item(lastRow)->text()));
  play();
}

void VideoPlayer::onPlayNextVideo() {
  if (m_playListWid->count() == 0) {
    return;
  }
  int nextRow = m_playListWid->currentRow() + 1;
  if (nextRow >= m_playListWid->count()) {
    qDebug("next is to the first");
    nextRow = 0;
  }
  m_playListWid->setCurrentRow(nextRow);
  setUrl(QUrl::fromLocalFile(m_playListWid->item(nextRow)->text()));
  play();
}

void VideoPlayer::onShowPlaylist() {
  if (g_videoPlayerActions()._VIDEOS_LIST_MENU->isChecked() == m_playlistDock->isVisible()) {
    return;
  }
  m_playlistDock->setVisible(g_videoPlayerActions()._VIDEOS_LIST_MENU->isChecked());
}

void VideoPlayer::onClearPlaylist() {
  m_playListWid->clear();
}

void VideoPlayer::openAFolder(const QString& folderPath) {
  QString loadFromPath;
  if (not QFileInfo(folderPath).isDir()) {
    const QString& loadFromDefaultPath =
        PreferenceSettings().value(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.v).toString();
    loadFromPath = QFileDialog::getExistingDirectory(this, "load videos from a folder", loadFromDefaultPath);
  }
  QFileInfo loadFromFi(loadFromPath);
  if (not loadFromFi.isDir()) {
    return;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_VIDEO_PLAYER_OPEN_PATH.name, loadFromFi.absoluteFilePath());
  QDirIterator it(loadFromPath, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    m_playListWid->addItem(it.filePath());
  }
  m_playlistDock->setWindowTitle(PLAYLIST_DOCK_TITLE_TEMPLATE.arg(m_playListWid->count()));
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

void VideoPlayer::positionChanged(qint64 position) {
  m_timeLabel->setText(QString("%1").arg(position / MICROSECOND, DURATION_PLACEHOLDER_LENGTH, 10, QChar(' ')));
  m_slider->setValue(position);
}

void VideoPlayer::durationChanged(qint64 duration) {
  qDebug("Duration changed to %d", duration);
  m_slider->setRange(0, duration);
}

void VideoPlayer::setPosition(int position) {
  m_mediaPlayer->setPosition(position);
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

QString VideoPlayer::GetJsonFilePath(const QString& vidsPath) const {
  const int sufLen = vidsPath.lastIndexOf('.');
  const QString& jsonPath = vidsPath.left(sufLen) + ".json";
  return jsonPath;
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  VideoPlayer player;
  player.show();
  player("E:/Leaked And Loaded/Leaked And Loaded - Billy Santoro, Gage Santoro.ts");
  a.exec();
  return 0;
}
#endif
