#ifndef VIDEOPLAYERACTIONS_H
#define VIDEOPLAYERACTIONS_H

#include <QObject>
#include <QActionGroup>

class VideoPlayerActions : public QObject
{
  Q_OBJECT
 public:
  explicit VideoPlayerActions(QObject *parent = nullptr);

  QActionGroup* GetRateActionGroups();

  QAction* _UPDATE_ITEM_PLAYABLE;
  QAction* _MOVE_SELECTED_ITEMS_TO_TRASHBIN;

  QAction* _SCROLL_TO_NEXT_FOLDER;
  QAction* _SCROLL_TO_LAST_FOLDER;

  QAction* _JUMP_LAST_HOT_SCENE;
  QAction* _JUMP_NEXT_HOT_SCENE;
  QAction* _LAST_10_SECONDS;
  QAction* _NEXT_10_SECONDS;
  QAction* _AUTO_PLAY_NEXT_VIDEO;

  QAction* _PLAY_PAUSE;

  QAction* _LAST_VIDEO;
  QAction* _NEXT_VIDEO;

  QAction* _OPEN_A_VIDEO;
  QAction* _LOAD_A_PATH;
  QAction* _CLEAR_VIDEOS_LIST;

  QAction* _PLAY_CURRENT_PATH;
  QAction* _PLAY_SELECTION;
  QActionGroup* _BATCH_VIDEO_ACTIONS;

  QAction* _VIDEOS_LIST_MENU;

  QAction* _MARK_HOT_SCENE;
  QAction* _GRAB_FRAME;

  QAction* _RENAME_VIDEO;
  QAction* _MOD_PERFORMERS;

  QActionGroup* _RATE_AG;
  const int _RATE_LEVEL_COUNT;

  QAction* _REVEAL_IN_EXPLORER;
};

VideoPlayerActions& g_videoPlayerActions();
#endif // VIDEOPLAYERACTIONS_H
