#ifndef VIDEOPLAYERACTIONS_H
#define VIDEOPLAYERACTIONS_H

#include <QActionGroup>
#include <QObject>
#include <QToolBar>
#include <QLabel>

class VideoPlayerActions : public QObject {
  Q_OBJECT
 public:
  explicit VideoPlayerActions(QObject* parent = nullptr);

  QToolBar* GetPlayControlToolBar(QWidget* parent=nullptr, QLabel* label=nullptr);

  QAction* _VOLUME_CTRL_MUTE{new (std::nothrow) QAction(tr("Mute"), this)};
  QAction* _UPDATE_ITEM_PLAYABLE{new (std::nothrow) QAction(QIcon(":img/REFRESH_THIS_PATH"), tr("Update"), this)};
  QAction* _MOVE_SELECTED_ITEMS_TO_TRASHBIN{new (std::nothrow) QAction(QIcon(":img/MOVE_TO_TRASH_BIN"), tr("Trashbin"), this)};
  QAction* _SCROLL_TO_NEXT_FOLDER{new (std::nothrow) QAction(QIcon(":img/NEXT_FOLDER"), tr("Nxt folder"), this)};
  QAction* _SCROLL_TO_LAST_FOLDER{new (std::nothrow) QAction(QIcon(":img/LAST_FOLDER"), tr("Lst folder"), this)};
  QAction* _JUMP_LAST_HOT_SCENE{new (std::nothrow) QAction(QIcon(":img/JUMP_LAST_HOT_SCENE"), tr("last hot scene"), this)};
  QAction* _JUMP_NEXT_HOT_SCENE{new (std::nothrow) QAction(QIcon(":img/JUMP_NEXT_HOT_SCENE"), tr("next hot scene"), this)};
  QAction* _LAST_10_SECONDS{new (std::nothrow) QAction("-10s", this)};
  QAction* _NEXT_10_SECONDS{new (std::nothrow) QAction("+10s", this)};
  QAction* _AUTO_PLAY_NEXT_VIDEO{new (std::nothrow) QAction(tr("Autoplay"), this)};
  QAction* _PLAY_PAUSE{new (std::nothrow) QAction(QIcon(":img/PLAY_VIDEO"), tr("play/pause"), this)};
  QAction* _LAST_VIDEO{new (std::nothrow) QAction(QIcon(":img/LAST_VIDEO"), tr("last video"), this)};
  QAction* _NEXT_VIDEO{new (std::nothrow) QAction(QIcon(":img/NEXT_VIDEO"), tr("next video"), this)};
  QAction* _OPEN_A_VIDEO{new (std::nothrow) QAction(QIcon(":img/OPEN_A_VIDEO"), tr("open a video"), this)};
  QAction* _LOAD_A_PATH{new (std::nothrow) QAction(QIcon(":img/OPEN_A_FOLDER"), tr("load a path"), this)};
  QAction* _CLEAR_VIDEOS_LIST{new (std::nothrow) QAction(QIcon(":img/EMPTY_LISTWIDGET"), tr("clear playlist"), this)};

  QAction* _SHOW_VIDEOS_LIST{new (std::nothrow) QAction(QIcon(":img/VIDEOS_LIST_MENU"), tr("keep show playlist"), this)};
  QAction* _MARK_HOT_SCENE{new (std::nothrow) QAction(QIcon(":img/MARK_HOT_SCENE_POSITION"), tr("mark"), this)};
  QAction* _GRAB_FRAME{new (std::nothrow) QAction(QIcon(":img/GRAB_FRAME"), tr("grab"), this)};
  QAction* _RENAME_VIDEO{new (std::nothrow) QAction(QIcon(":img/RENAME"), tr("rename"), this)};
  QAction* _MOD_CAST{new (std::nothrow) QAction(QIcon(":img/CAST_SET"), tr("mod performers"), this)};

  QActionGroup* _RATE_AG{GetRateActionGroups()};
  QAction* _REVEAL_IN_EXPLORER{new (std::nothrow) QAction(QIcon(":img/REVEAL_IN_EXPLORER"), tr("Reveal in explorer"), this)};

  const int _RATE_LEVEL_COUNT{_RATE_AG->actions().size()};

 private:
  QActionGroup* GetRateActionGroups();
};

VideoPlayerActions& g_videoPlayerActions();
#endif  // VIDEOPLAYERACTIONS_H
