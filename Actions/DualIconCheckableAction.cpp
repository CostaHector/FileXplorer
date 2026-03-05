#include "DualIconCheckableAction.h"

DualIconCheckableAction::DualIconCheckableAction(const QIcon& whenUnchecked,
                                                 const QIcon& whenChecked,
                                                 const QString& whenUncheckedtext,
                                                 const QString& whenCheckedtext,
                                                 bool bIsCheckedWhenInit,
                                                 QObject* parent)
  : QAction{parent}
  , m_iconUnchecked{whenUnchecked}
  , m_iconChecked{whenChecked}
  , m_textUnchecked{whenUncheckedtext}
  , m_textChecked{whenCheckedtext} {
  setCheckable(true);
  setChecked(bIsCheckedWhenInit);
  updateActionProperty(bIsCheckedWhenInit);

  connect(this, &QAction::toggled, this, &DualIconCheckableAction::onToggled);
}

void DualIconCheckableAction::onToggled(bool checked) {
  updateActionProperty(checked);
}

void DualIconCheckableAction::updateActionProperty(bool checked) {
  setIcon(checked ? m_iconChecked : m_iconUnchecked);
  setText(checked ? m_textChecked : m_textUnchecked);
}

DualIconCheckableAction* DualIconCheckableAction::CreateMuteAction(QObject* parent, const bool bMute) {
  DualIconCheckableAction* muteAction = new DualIconCheckableAction{QIcon{":/VideoPlayer/VOLUME_UNMUTE"},
                                                                    QIcon{":/VideoPlayer/VOLUME_MUTE"},
                                                                    tr("Mute"),
                                                                    tr("Mute"),
                                                                    bMute,
                                                                    parent};
  muteAction->setToolTip(QString{"<b>%1 (%2)</b><br/>Mute current player when enabled."} //
                             .arg(muteAction->text(), muteAction->shortcut().toString()));
  return muteAction;
}
DualIconCheckableAction* DualIconCheckableAction::CreatePauseAction(QObject* parent, const bool bPause) {
  DualIconCheckableAction* pauseAction = new DualIconCheckableAction{QIcon{":/VideoPlayer/PLAY_VIDEO"},
                                                                     QIcon{":/VideoPlayer/PAUSE_VIDEO"},
                                                                     tr("Pause"),
                                                                     tr("Pause"),
                                                                     bPause,
                                                                     parent};
  pauseAction->setToolTip(QString{"<b>%1 (%2)</b><br/>Pause current player when enabled."} //
                              .arg(pauseAction->text(), pauseAction->shortcut().toString()));
  return pauseAction;
}

DualIconCheckableAction* DualIconCheckableAction::CreateFullSceenAction(QObject* parent, const bool bFullScreen) {
  DualIconCheckableAction* fullScreenAction = new DualIconCheckableAction{QIcon{":/VideoPlayer/FULL_SCREEN_OFF"},
                                                                          QIcon{":/VideoPlayer/FULL_SCREEN_ON"},
                                                                          tr("Full screen"),
                                                                          tr("Full screen"),
                                                                          bFullScreen,
                                                                          parent};
  fullScreenAction->setShortcut(QKeySequence(Qt::Key::Key_F12));
  fullScreenAction->setToolTip(QString{"<b>%1 (%2)</b><br/>Show view widget in fullscreen when enabled."} //
                                   .arg(fullScreenAction->text(), fullScreenAction->shortcut().toString()));
  return fullScreenAction;
}
