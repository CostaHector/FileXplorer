﻿#include "FramelessWindowActions.h"
#include "public/MemoryKey.h"
#include "public/PublicMacro.h"
#include <QApplication>
#include <QStyle>

FramelessWindowActions::FramelessWindowActions(QObject* parent)
    : QObject{parent}  //
{
  _EXPAND_RIBBONS = new (std::nothrow) QAction{QIcon{":img/EXPAND_RIBBON"}, "Expand or Hide Ribbon Menu", this};
  CHECK_NULLPTR_RETURN_VOID(_EXPAND_RIBBONS)
  _EXPAND_RIBBONS->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_1));
  _EXPAND_RIBBONS->setToolTip(QString("<b>%1 (%2)</b><br/>Displays the Ribbon Menu when enabled, or hides it when disabled.").arg(_EXPAND_RIBBONS->text(), _EXPAND_RIBBONS->shortcut().toString()));
  _EXPAND_RIBBONS->setCheckable(true);
  _EXPAND_RIBBONS->setChecked(PreferenceSettings().value(MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.name, MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.v).toBool());

  _FRAMELESS = new (std::nothrow) QAction{QIcon{":img/FRAMELESS_WINDOW"}, "Frameless Window", this};
  CHECK_NULLPTR_RETURN_VOID(_FRAMELESS)
  _FRAMELESS->setShortcut(QKeySequence(Qt::Key::Key_F10));
  _FRAMELESS->setToolTip(QString("<b>%1 (%2)</b><br/>Controls whether the window displays with a standard title bar or in frameless mode").arg(_FRAMELESS->text(), _FRAMELESS->shortcut().toString()));
  _FRAMELESS->setCheckable(true);
  _FRAMELESS->setChecked(PreferenceSettings().value(MemoryKey::SHOW_FRAMELESS_WINDOW.name, MemoryKey::SHOW_FRAMELESS_WINDOW.v).toBool());

  _MINIMIZE = new (std::nothrow) QAction{QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_TitleBarMinButton), "Minimize Window", this};
  CHECK_NULLPTR_RETURN_VOID(_MINIMIZE)
  _MINIMIZE->setToolTip(QString("<b>%1 (%2)</b><br/> Shows the widget minimized, as an icon.").arg(_MINIMIZE->text(), _MINIMIZE->shortcut().toString()));

  _MAXIMIZE = new (std::nothrow) QAction{QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_TitleBarMaxButton), "Maximize/Normalize window", this};
  CHECK_NULLPTR_RETURN_VOID(_MAXIMIZE)
  _MAXIMIZE->setCheckable(true);
  _MAXIMIZE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Shows the widget maximized or restores the widget after it has been maximized or minimized.").arg(_MAXIMIZE->text(), _MAXIMIZE->shortcut().toString()));

  _CLOSE = new (std::nothrow) QAction{QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_TitleBarCloseButton), "X", this};
  CHECK_NULLPTR_RETURN_VOID(_CLOSE)

  FRAMELESS_MENU_RIBBONS = Get_FRAMELESS_MENU_Actions();
}

QActionGroup* FramelessWindowActions::Get_FRAMELESS_MENU_Actions() {
  QActionGroup* actionGroup = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_NULLPTR(actionGroup)
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
