#ifndef VIEWACTIONS_H
#define VIEWACTIONS_H

#include <QAction>
#include <QActionGroup>

class ViewActions : public QObject {
  Q_OBJECT
 public:
  explicit ViewActions(QObject* parent = nullptr)
      : QObject{parent},
        _ADVANCE_SEARCH_VIEW{new QAction(QIcon(":/themes/SEARCH"), "search")},
        _MOVIE_VIEW{new QAction(QIcon(":/themes/SHOW_DATABASE"), "movie")},
        _LIST_VIEW{new QAction(QIcon(":/themes/DISPLAY_LARGE_THUMBNAILS"), "list")},
        _TABLE_VIEW{new QAction(QIcon(":/themes/DISPLAY_DETAIL_INFOMATIONS"), "table")},
        _TREE_VIEW{new QAction(QIcon(":/themes/DISPLAY_ACHITECTURE"), "tree")},
        _TRIPLE_VIEW{GetListTableTreeActions()},

        NAVIGATION_PANE{new QAction(QIcon(":/themes/NAVIGATION_PANE"), tr("Navigate pane"))},
        PREVIEW_PANE_HTML{new QAction(QIcon(":/themes/SHOW_FOLDER_PREVIEW_HTML"), tr("HTML preview"))},
        _JSON_EDITOR_PANE{new QAction(QIcon(":/themes/SHOW_FOLDER_PREVIEW_JSON_EDITOR"), tr("Json editor"))},
        _VIDEO_PLAYER_EMBEDDED{new QAction(QIcon(":/themes/VIDEO_PLAYER"), tr("Embedded player"))},
        _VIEW_ACRIONS(Get_NAVIGATION_PANE_Actions()),
        _SYS_VIDEO_PLAYERS(new QAction(QIcon(":/themes/PLAY_BUTTON_TRIANGLE"), tr("Play"))),
        _VIDEO_PLAYERS(GetPlayersActions()) {}

  QActionGroup* Get_NAVIGATION_PANE_Actions();

  QActionGroup* GetPlayersActions();

  QActionGroup* GetListTableTreeActions();
  QAction *_ADVANCE_SEARCH_VIEW, *_MOVIE_VIEW, *_LIST_VIEW, *_TABLE_VIEW, *_TREE_VIEW;
  QActionGroup* _TRIPLE_VIEW;

  QAction* NAVIGATION_PANE;
  QAction* PREVIEW_PANE_HTML;
  QAction* _JSON_EDITOR_PANE;
  QAction* _VIDEO_PLAYER_EMBEDDED;
  QActionGroup* _VIEW_ACRIONS;

  QAction* _SYS_VIDEO_PLAYERS = nullptr;
  QActionGroup* _VIDEO_PLAYERS;
};

ViewActions& g_viewActions();

#endif  // VIEWACTIONS_H
