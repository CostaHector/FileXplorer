/* https://github.com/Nevcairiel/LAVFilters/releases */

#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QKeyEvent>
#include <QListWidget>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QSplitter>
#include <QToolBar>
#include <QVideoProbe>
#include <QWidget>

#include "Component/ClickableSlider.h"
#include "Component/JsonPerformersListInputer.h"
#include "public/PublicVariable.h"

#include "Model/VidModel.h"
#include "View/VidsPlayListView.h"

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QLabel;
class QUrl;
QT_END_NAMESPACE

class VideoPlayer : public QMainWindow {
  Q_OBJECT
 public:
  explicit VideoPlayer(QWidget* parent = nullptr);
  ~VideoPlayer();

  auto operator()(const QString& path) -> bool;                 // one file or one folder
  auto operator()(const QStringList& fileAbsPathList) -> bool;  // selections

  void setUrl(const QUrl& url);
  auto subscribe() -> void;

  auto onModeName() -> bool;
  auto onModPerformers() -> bool;

  auto onGrabAFrame(const QVideoFrame& frame) -> bool;
  auto onMarkHotScenes() -> bool;

  auto onJumpToNextHotScene() -> bool;
  auto onJumpToLastHotScene() -> bool;
  auto onPositionAdd(const int ms = 10 * 1000) -> bool;

  auto onRateForThisMovie(const QAction* checkedAction) -> bool;
  void onPlayLastVideo();
  void onPlayNextVideo();

  void onListWidgetDoubleClicked(const QModelIndex& ind);

  void onScrollToAnotherFolder(int inc = 1);
  void onScrollToNextFolder() { onScrollToAnotherFolder(1); }
  void onScrollToLastFolder() { onScrollToAnotherFolder(-1); }

  void onShowPlaylist(bool keepShow);
  void onClearPlaylist();
  void openAFolder(const QString& folderPath = "");

  void onVolumeMute(const bool isMute);
  void onVolumeValueChange(const int logScaleValue);

  auto updateWindowsSize() -> void {
    if (PreferenceSettings().contains("VideoPlayerGeometry")) {
      restoreGeometry(PreferenceSettings().value("VideoPlayerGeometry").toByteArray());
    } else {
      setGeometry(QRect(0, 0, 600, 400));
    }
    m_playlistSplitter->restoreState(PreferenceSettings().value("VideoPlayerSplitterState", QByteArray()).toByteArray());
  }

  auto closeEvent(QCloseEvent* event) -> void override {
    setUrl({});
    PreferenceSettings().setValue("VideoPlayerGeometry", saveGeometry());
    qDebug("Video Player geometry was resize to (%d, %d, %d, %d)", geometry().x(), geometry().y(), geometry().width(), geometry().height());
    PreferenceSettings().setValue("VideoPlayerSplitterState", m_playlistSplitter->saveState());
    QMainWindow::closeEvent(event);
  }

  auto keyPressEvent(QKeyEvent* e) -> void override;

  int onRecycleSelectedItems();

 public slots:
  void openFile(const QString& filePath = "");
  void play();

 private slots:
  void mediaStateChanged(QMediaPlayer::State state);
  void onPlayerPositionChanged(qint64 position);
  void durationChanged(qint64 duration);
  void onSetPlayerPosition(int position);
  void handleError();

 private:
  static inline QString MillionSecond2hhmmss(qint64 ms) { return QTime(0, 0, 0, 0).addMSecs(ms).toString("hh:mm:ss"); }

  auto loadHotSceneList() -> void;
  auto loadVideoRate() -> void;
  inline auto JsonFileValidCheck(const QString& op = "do this") -> QString;

  QUrl m_playingUrl;
  QMediaPlayer* m_mediaPlayer;
  ClickableSlider* m_timeSlider;
  QSlider* m_volumnSlider;
  QString m_timeTemplate;
  QLabel* m_timeLabel;
  QLabel* m_errorLabel;

  QToolBar* m_sliderTB;
  QToolBar* m_controlTB;

  QVideoWidget* m_videoWidget;
  QVideoProbe* m_probe;

  static constexpr int DURATION_PLACEHOLDER_LENGTH = 5;

  QList<int> m_hotSceneList;
  constexpr static int MICROSECOND = 1000;

  VidModel* m_playListModel;
  VidsPlayListView* m_playListWid;

  QObject* m_watcher = nullptr;

  QSplitter* m_playlistSplitter;

  static const QString PLAYLIST_DOCK_TITLE_TEMPLATE;

  JsonPerformersListInputer* m_performerWid;

  QStatusBar* m_playerStatusBar;

  QVariantHash m_dict;
};

#endif  // VIDEOPLAYER_H
