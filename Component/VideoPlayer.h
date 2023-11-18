#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QDockWidget>
#include <QKeyEvent>
#include <QListWidget>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QToolBar>
#include <QVideoProbe>
#include <QWidget>

#include "Component/ClickableSlider.h"
#include "Component/PerformersWidget.h"
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

  void onShowPlaylist();
  void onClearPlaylist();
  void openAFolder(const QString& folderPath = "");

  auto updateWindowsSize() -> void {
    if (PreferenceSettings().contains("VideoPlayerGeometry")) {
      restoreGeometry(PreferenceSettings().value("VideoPlayerGeometry").toByteArray());
    } else {
      setGeometry(QRect(0, 0, 600, 400));
    }
  }

  auto closeEvent(QCloseEvent* event) -> void override {
    setUrl({});
    PreferenceSettings().setValue("VideoPlayerGeometry", saveGeometry());
    qDebug("Resize Video Player to (%d, %d, %d, %d)", geometry().x(), geometry().y(), geometry().width(), geometry().height());
    QMainWindow::closeEvent(event);
  }

  auto keyPressEvent(QKeyEvent* e) -> void override {
    if (e->modifiers() == Qt::AltModifier and (e->key() == Qt::Key_Enter or e->key() == Qt::Key_Return)) {
      m_playlistDock->hide();
      m_sliderTB->show();
      m_controlTB->hide();
      setWindowState(Qt::WindowMaximized);
      return;
    } else if (e->key() == Qt::Key_Escape) {
      m_playlistDock->show();
      m_sliderTB->show();
      m_controlTB->show();
      setWindowState(Qt::WindowMaximized);
      return;
    } else if (e->key() == Qt::Key_F11) {
      m_playlistDock->hide();
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
  void positionChanged(qint64 position);
  void durationChanged(qint64 duration);
  void setPosition(int position);
  void handleError();

 private:
  auto loadHotSceneList() -> void;
  auto loadVideoRate() -> void;
  inline auto JsonFileValidCheck(const QString& op = "do this") -> QString;

  QMediaPlayer* m_mediaPlayer;
  ClickableSlider* m_slider;
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
  QDockWidget* m_playlistDock;

  static const QString PLAYLIST_DOCK_TITLE_TEMPLATE;

  PerformersWidget* m_performerWid;
  QVariantHash m_dict;

  QMenu* m_playListMenu;
};

#endif  // VIDEOPLAYER_H
