#include "ViewActions.h"
#include "ViewTypeTool.h"
#include "MemoryKey.h"

ViewActions::ViewActions(QObject* parent) : QObject{parent} {
  using namespace ViewTypeTool;

  _NAVIGATION_PANE = new (std::nothrow) QAction(QIcon(":img/NAVIGATION_PANE"), tr("Navigation Pane"), this);
  _NAVIGATION_PANE->setCheckable(true);
  _NAVIGATION_PANE->setChecked(Configuration().value(CompoVisKey::SHOW_NAVIGATION_SIDEBAR.name, CompoVisKey::SHOW_NAVIGATION_SIDEBAR.v).toBool());
  _NAVIGATION_PANE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Show or hide the navigation pane.").arg(_NAVIGATION_PANE->text(), _NAVIGATION_PANE->shortcut().toString()));

  _LIST_VIEW = new (std::nothrow) QAction(QIcon(":img/DISPLAY_LARGE_THUMBNAILS"), tr("LIST"), this);
  _TABLE_VIEW = new (std::nothrow) QAction(QIcon(":img/DISPLAY_DETAIL_INFOMATIONS"), tr("TABLE"), this);
  _TREE_VIEW = new (std::nothrow) QAction(QIcon(":img/DISPLAY_TREE_VIEW"), tr("TREE"), this);
  _ADVANCE_SEARCH_VIEW = new (std::nothrow) QAction(QIcon(":img/SEARCH"), tr("SEARCH"), this);
  _MOVIE_VIEW = new (std::nothrow) QAction(QIcon(":img/MOVIES_VIEW"), tr("MOVIE"), this);
  _SCENE_VIEW = new (std::nothrow) QAction(QIcon(":img/SCENES_VIEW"), tr("SCENE"), this);
  _CAST_VIEW = new (std::nothrow) QAction(QIcon(":img/CAST_VIEW"), tr("CAST"), this);
  _JSON_VIEW = new (std::nothrow) QAction(QIcon(":img/JSON_EDITOR"), tr("JSON"), this);
  _ALL_VIEWS += _LIST_VIEW;
  _ALL_VIEWS += _TABLE_VIEW;
  _ALL_VIEWS += _TREE_VIEW;
  _ALL_VIEWS += _MOVIE_VIEW;
  _ALL_VIEWS += _CAST_VIEW;
  _ALL_VIEWS += _SCENE_VIEW;
  _ALL_VIEWS += _JSON_VIEW;
  _ALL_VIEWS += _ADVANCE_SEARCH_VIEW;

  _LIST_VIEW->setShortcutVisibleInContextMenu(true);
  _LIST_VIEW->setToolTip(QString("Displays items by using large thumbnails. (%1)").arg(_LIST_VIEW->shortcut().toString()));
  _LIST_VIEW->setCheckable(true);

  _TABLE_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_6));
  _TABLE_VIEW->setShortcutVisibleInContextMenu(true);
  _TABLE_VIEW->setToolTip(QString("Displays information about each in the window. (%1)").arg(_TABLE_VIEW->shortcut().toString()));
  _TABLE_VIEW->setCheckable(true);
  _TABLE_VIEW->setChecked(true);

  _TREE_VIEW->setShortcutVisibleInContextMenu(true);
  _TREE_VIEW->setToolTip(QString("Display files and folders achitecures. (%1)").arg(_TREE_VIEW->shortcut().toString()));
  _TREE_VIEW->setCheckable(true);

  _ADVANCE_SEARCH_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F));
  _ADVANCE_SEARCH_VIEW->setShortcutVisibleInContextMenu(true);
  _ADVANCE_SEARCH_VIEW->setToolTip(QString("Show advanced search window. (%1)").arg(_ADVANCE_SEARCH_VIEW->shortcut().toString()));
  _ADVANCE_SEARCH_VIEW->setCheckable(true);

  _MOVIE_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_7));
  _MOVIE_VIEW->setShortcutVisibleInContextMenu(true);
  _MOVIE_VIEW->setToolTip(QString("Movie dictionary view aka Database view. (%1)").arg(_MOVIE_VIEW->shortcut().toString()));
  _MOVIE_VIEW->setCheckable(true);

  _CAST_VIEW->setShortcutVisibleInContextMenu(true);
  _CAST_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_8));
  _CAST_VIEW->setToolTip(QString("Show Cast database. (%1)").arg(_CAST_VIEW->shortcut().toString()));
  _CAST_VIEW->setCheckable(true);

  _SCENE_VIEW->setShortcutVisibleInContextMenu(true);
  _SCENE_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_9));
  _SCENE_VIEW->setToolTip(QString("Show video scenes in page table. (%1)").arg(_SCENE_VIEW->shortcut().toString()));
  _SCENE_VIEW->setCheckable(true);

  _JSON_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_0));
  _JSON_VIEW->setShortcutVisibleInContextMenu(true);
  _JSON_VIEW->setToolTip(QString("Show Json editor tableview. (%1)").arg(_JSON_VIEW->shortcut().toString()));
  _JSON_VIEW->setCheckable(true);

  _VIEW_BACK_TO = new (std::nothrow) QAction(QIcon{":img/_VIEW_BACK_TO"}, tr("View Back"), this);
  _VIEW_BACK_TO->setToolTip(QString("<b>%1 (Ctrl+Mouse BackButton)</b><br/> back to last view type.").arg(_VIEW_BACK_TO->text()));
  _VIEW_FORWARD_TO = new (std::nothrow) QAction(QIcon{":img/_VIEW_FORWARD_TO"}, tr("View Forward"), this);
  _VIEW_FORWARD_TO->setToolTip(QString("<b>%1 (Ctrl+Mouse ForwardButton)</b><br/> forward to next view type.").arg(_VIEW_FORWARD_TO->text()));
  _VIEWS_NAVIGATE += _VIEW_BACK_TO;
  _VIEWS_NAVIGATE += _VIEW_FORWARD_TO;

  const bool bShowPreviewPanel{Configuration().value(CompoVisKey::SHOW_PREVIEW_DOCKER.name, CompoVisKey::SHOW_PREVIEW_DOCKER.v).toBool()};
  _PREVIEW_PANEL = new (std::nothrow) QAction{QIcon{":img/SHOW_FOLDER_PREVIEW"}, tr("Preview Panel"), this};
  _PREVIEW_PANEL->setCheckable(true);
  _PREVIEW_PANEL->setChecked(bShowPreviewPanel);
  _PREVIEW_PANEL->setToolTip(
      QString("<b>%1 (%2)</b><br/> Show or hide the preview pane.").arg(_PREVIEW_PANEL->text(), _PREVIEW_PANEL->shortcut().toString()));

  _SYS_VIDEO_PLAYERS = new (std::nothrow) QAction(QIcon(":img/SYSTEM_APPLICATION_VIDEO"), tr("Play in System Application"), this);
  _SYS_VIDEO_PLAYERS->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Return));
  _SYS_VIDEO_PLAYERS->setShortcutVisibleInContextMenu(true);
  _SYS_VIDEO_PLAYERS->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                         "Play the selected item(s) in default system player.")
                                     .arg(_SYS_VIDEO_PLAYERS->text(), _SYS_VIDEO_PLAYERS->shortcut().toString()));

  _HAR_VIEW = new (std::nothrow) QAction{QIcon(":img/HAR_VIEW"), tr("Har View"), this};
  _HAR_VIEW->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                "Double click to open an har file")
                            .arg(_HAR_VIEW->text(), _HAR_VIEW->shortcut().toString()));

  _FONT_TYPE_AND_SIZE = new (std::nothrow) QAction{QIcon{":img/FONT_SIZE"}, tr("Font"), this};
  _FONT_TYPE_AND_SIZE->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                          "Configure text font settings")
                                      .arg(_FONT_TYPE_AND_SIZE->text(), _FONT_TYPE_AND_SIZE->shortcut().toString()));

  _ROW_HEIGHT = new (std::nothrow) QAction{QIcon{":img/DEFAULT_ROW_HEIGHT"}, tr("Row Height"), this};
  _ROW_HEIGHT->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                  "Configure tableview/treeview row height")
                              .arg(_ROW_HEIGHT->text(), _ROW_HEIGHT->shortcut().toString()));
}

ViewActions::~ViewActions() {
  Configuration().setValue(CompoVisKey::SHOW_NAVIGATION_SIDEBAR.name, _NAVIGATION_PANE->isChecked());
  Configuration().setValue(CompoVisKey::SHOW_PREVIEW_DOCKER.name, _PREVIEW_PANEL->isChecked());
}

ViewActions& g_viewActions() {
  static ViewActions ins;
  return ins;
}
