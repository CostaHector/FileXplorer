#ifndef VIEWACTIONS_H
#define VIEWACTIONS_H

#include <QAction>
#include <QActionGroup>
#include "PublicVariable.h"

class ViewActions : public QObject {
 public:
  explicit ViewActions(QObject* parent = nullptr);

  auto Get_NAVIGATION_PANE_Actions() -> QActionGroup* {
    auto* NAVIGATION_PANE = new QAction(QIcon(":/themes/NAVIGATION_PANE"), "Navigate pane");
    NAVIGATION_PANE->setToolTip(
        QString("<b>%1 (%2)</b><br/> Show or hide the navigation pane.").arg(NAVIGATION_PANE->text(), NAVIGATION_PANE->shortcut().toString()));
    NAVIGATION_PANE->setCheckable(true);

    auto* PREVIEW_PANE_HTML = new QAction(QIcon(":/themes/SHOW_FOLDER_PREVIEW_HTML"), "HTML preview");
    PREVIEW_PANE_HTML->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_P));
    PREVIEW_PANE_HTML->setToolTip(
        QString("<b>%1 (%2)</b><br/> Show or hide the preview pane.").arg(PREVIEW_PANE_HTML->text(), PREVIEW_PANE_HTML->shortcut().toString()));
    PREVIEW_PANE_HTML->setCheckable(true);

    auto* JSON_EDITOR_PANE = new QAction(QIcon(":/themes/SHOW_FOLDER_PREVIEW_JSON_EDITOR"), "Json editor");
    JSON_EDITOR_PANE->setChecked(PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_JSON_EDITOR.name).toBool());
    JSON_EDITOR_PANE->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_J));
    JSON_EDITOR_PANE->setToolTip(
        QString("<b>%1 (%2)</b><br/>Show Json Edit Pane.").arg(JSON_EDITOR_PANE->text(), JSON_EDITOR_PANE->shortcut().toString()));
    JSON_EDITOR_PANE->setCheckable(true);


    auto* actionGroup = new QActionGroup(this);
    actionGroup->addAction(NAVIGATION_PANE);
    actionGroup->addAction(PREVIEW_PANE_HTML);
    actionGroup->addAction(JSON_EDITOR_PANE);
    actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

    NAVIGATION_PANE->setChecked(PreferenceSettings().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name).toBool());
    PREVIEW_PANE_HTML->setChecked(PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name).toBool());
    JSON_EDITOR_PANE->setChecked(PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_JSON_EDITOR.name).toBool());
    return actionGroup;
  }

  QActionGroup* PANES_RIBBONS;
};

ViewActions& g_viewActions();

#endif  // VIEWACTIONS_H
