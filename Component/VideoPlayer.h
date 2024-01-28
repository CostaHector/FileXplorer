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
#include "PublicVariable.h"

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

  auto operator()(const QString& path) -> bool;
  auto PlaySelections(const QStringList& fileAbsPathList) -> bool;

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

  void onListWidgetDoubleClicked(QListWidgetItem* item);
  void onUpdatePlayableList();

  void onRevealInSystemExplorer();
  void onRecycleSelectedItems();

  void onScrollToAnotherFolder(int inc = 1);
  void onScrollToNextFolder() { onScrollToAnotherFolder(1); }
  void onScrollToLastFolder() { onScrollToAnotherFolder(-1); }

  void onShowPlaylist();
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

  auto keyPressEvent(QKeyEvent* e) -> void override {
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
    }
    QWidget::keyPressEvent(e);
  }

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
  auto loadHotSceneList() -> void;
  auto loadVideoRate() -> void;
  inline auto JsonFileValidCheck(const QString& op = "do this") -> QString;

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

  QListWidget* m_playListWid;
  QSplitter* m_playlistSplitter;

  static const QString PLAYLIST_DOCK_TITLE_TEMPLATE;
  static const QColor RECYCLED_ITEM_COLOR;

  JsonPerformersListInputer* m_performerWid;
  QVariantHash m_dict;

  QMenu* m_playListMenu;
};

#endif  // VIDEOPLAYER_H
