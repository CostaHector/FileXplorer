#ifndef VIEWACTIONS_H
#define VIEWACTIONS_H

#include <QAction>
#include <QActionGroup>
#include "PublicVariable.h"

class ViewActions : public QObject {
  Q_OBJECT
 public:
  explicit ViewActions(QObject* parent = nullptr)
      : QObject{parent},
        NAVIGATION_PANE{new QAction(QIcon(":/themes/NAVIGATION_PANE"), tr("Navigate pane"))},
        PREVIEW_PANE_HTML{new QAction(QIcon(":/themes/SHOW_FOLDER_PREVIEW_HTML"), tr("HTML preview"))},
        JSON_EDITOR_PANE{new QAction(QIcon(":/themes/SHOW_FOLDER_PREVIEW_JSON_EDITOR"), tr("Json editor"))},
        _VIDEO_PLAYER_EMBEDDED{new QAction(QIcon(":/themes/VIDEO_PLAYER"), tr("Embedded player"))},
        _VIEW_ACRIONS(Get_NAVIGATION_PANE_Actions()),
        _SYS_VIDEO_PLAYERS(new QAction(QIcon(":/themes/PLAY_BUTTON_TRIANGLE"), tr("Play"))),
        _VIDEO_PLAYERS(GetPlayersActions()) {}

  auto Get_NAVIGATION_PANE_Actions() -> QActionGroup* {
    NAVIGATION_PANE->setToolTip(
        QString("<b>%1 (%2)</b><br/> Show or hide the navigation pane.").arg(NAVIGATION_PANE->text(), NAVIGATION_PANE->shortcut().toString()));
    NAVIGATION_PANE->setCheckable(true);

    PREVIEW_PANE_HTML->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_P));
    PREVIEW_PANE_HTML->setToolTip(
        QString("<b>%1 (%2)</b><br/> Show or hide the preview pane.").arg(PREVIEW_PANE_HTML->text(), PREVIEW_PANE_HTML->shortcut().toString()));
    PREVIEW_PANE_HTML->setCheckable(true);

    JSON_EDITOR_PANE->setToolTip(
        QString("<b>%1 (%2)</b><br/>Show Json Edit Pane.").arg(JSON_EDITOR_PANE->text(), JSON_EDITOR_PANE->shortcut().toString()));

    _VIDEO_PLAYER_EMBEDDED->setShortcutVisibleInContextMenu(true);
    _VIDEO_PLAYER_EMBEDDED->setToolTip(QString("<b>%1 (%2)</b><br/> Open the selected item in embedded video player.")
                                           .arg(_VIDEO_PLAYER_EMBEDDED->text(), _VIDEO_PLAYER_EMBEDDED->shortcut().toString()));

    auto* actionGroup = new QActionGroup(this);
    actionGroup->addAction(NAVIGATION_PANE);
    actionGroup->addAction(PREVIEW_PANE_HTML);
    actionGroup->addAction(JSON_EDITOR_PANE);
    actionGroup->addAction(_VIDEO_PLAYER_EMBEDDED);
    actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

    NAVIGATION_PANE->setChecked(PreferenceSettings().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name).toBool());
    PREVIEW_PANE_HTML->setChecked(PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name).toBool());
    return actionGroup;
  }

  QActionGroup* GetPlayersActions() {
    _SYS_VIDEO_PLAYERS->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Return));
    _SYS_VIDEO_PLAYERS->setShortcutVisibleInContextMenu(true);
    _SYS_VIDEO_PLAYERS->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                           "Play the selected item(s) in default system player.")
                                       .arg(_SYS_VIDEO_PLAYERS->text(), _SYS_VIDEO_PLAYERS->shortcut().toString()));
    QActionGroup* actionGroup = new QActionGroup(this);
    actionGroup->addAction(_SYS_VIDEO_PLAYERS);
    actionGroup->addAction(_VIDEO_PLAYER_EMBEDDED);
    actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

    for (QAction* act : actionGroup->actions()) {
      act->setCheckable(false);
    }
    return actionGroup;
  }

  QAction* NAVIGATION_PANE;
  QAction* PREVIEW_PANE_HTML;
  QAction* JSON_EDITOR_PANE;
  QAction* _VIDEO_PLAYER_EMBEDDED;
  QActionGroup* _VIEW_ACRIONS;

  QAction* _SYS_VIDEO_PLAYERS = nullptr;
  QActionGroup* _VIDEO_PLAYERS;
};

ViewActions& g_viewActions();

#endif  // VIEWACTIONS_H
