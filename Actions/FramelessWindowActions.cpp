#include "FramelessWindowActions.h"
#include <QStyle>
#include "PublicVariable.h"

FramelessWindowActions::FramelessWindowActions(QObject* parent)
    : QObject{parent},
      _EXPAND_RIBBONS{new QAction(QIcon(":/themes/EXPAND_RIBBON"), tr("Expand or Hide Commands"), this)},
      _FRAMELESS{new QAction(QIcon(":/themes/FRAMELESS_WINDOW"), tr("Frameless Window"), this)},
      _MINIMIZE{new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_TitleBarMinButton), "Minimize Window", this)},
      _MAXIMIZE{new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_TitleBarMaxButton), "Maximize/Normalize window", this)},
      _CLOSE{new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_TitleBarCloseButton), "X", this)},
      FRAMELESS_MENU_RIBBONS(Get_FRAMELESS_MENU_Actions()) {}

auto FramelessWindowActions::Get_FRAMELESS_MENU_Actions() -> QActionGroup* {
  _EXPAND_RIBBONS->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_1));
  _EXPAND_RIBBONS->setToolTip(QString("<b>%1 (%2)</b><br/> Show tabs with visible commands when enabled, or show tabs with hidden commands")
                                  .arg(_EXPAND_RIBBONS->text(), _EXPAND_RIBBONS->shortcut().toString()));
  _EXPAND_RIBBONS->setCheckable(true);
  _EXPAND_RIBBONS->setChecked(
      PreferenceSettings().value(MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.name, MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.v).toBool());

  _FRAMELESS->setCheckable(true);
  _FRAMELESS->setShortcut(QKeySequence(Qt::Key::Key_F10));
  _FRAMELESS->setToolTip(QString("<b>%1 (%2)</b><br/> Set window framless or not.").arg(_FRAMELESS->text(), _FRAMELESS->shortcut().toString()));
  _FRAMELESS->setChecked(PreferenceSettings().value(MemoryKey::SHOW_FRAMELESS_WINDOW.name, MemoryKey::SHOW_FRAMELESS_WINDOW.v).toBool());

  _MINIMIZE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Shows the widget minimized, as an icon.").arg(_MINIMIZE->text(), _MINIMIZE->shortcut().toString()));

  _MAXIMIZE->setCheckable(true);
  _MAXIMIZE->setToolTip(QString("<b>%1 (%2)</b><br/> Shows the widget maximized or Restores the widget after it has been maximized or minimized.")
                            .arg(_MAXIMIZE->text(), _MAXIMIZE->shortcut().toString()));

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(_EXPAND_RIBBONS);
  actionGroup->addAction(_FRAMELESS);
  actionGroup->addAction(_MINIMIZE);
  actionGroup->addAction(_MAXIMIZE);
  actionGroup->addAction(_CLOSE);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  return actionGroup;
}

FramelessWindowActions& g_framelessWindowAg() {
  static FramelessWindowActions framelessWindowsActIns;
  return framelessWindowsActIns;
}
