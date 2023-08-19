#include "FileLeafAction.h"


QAction* FileLeafAction::_REVEAL_IN_EXPLORER(nullptr);
QAction* FileLeafAction::_PREFERENCE_SETTING(nullptr);
QAction* FileLeafAction::_ABOUT_FILE_EXPLORER(nullptr);
QActionGroup* FileLeafAction::LEAF_FILE(nullptr);

FileLeafAction::FileLeafAction() {
}

QActionGroup* FileLeafAction::ag(){
    if (LEAF_FILE != nullptr){
        return LEAF_FILE;
    }

    _REVEAL_IN_EXPLORER = new QAction(QIcon(":/themes/REVEAL_IN_EXPLORER"), "&Reveal");
    _PREFERENCE_SETTING = new QAction(QIcon(":/themes/SETTINGS"), "&Settings");
    _ABOUT_FILE_EXPLORER = new QAction(QIcon(":/themes/ABOUT"), "&About");
    LEAF_FILE = new QActionGroup(nullptr);

    _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
    _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
    _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%0 (%1)</b><br/> Reveal items in system file explorer.").arg
                                    (_REVEAL_IN_EXPLORER->text(), _REVEAL_IN_EXPLORER->shortcut().toString()));
    _REVEAL_IN_EXPLORER->setCheckable(false);

    _PREFERENCE_SETTING->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_P));
    _PREFERENCE_SETTING->setShortcutVisibleInContextMenu(true);
    _PREFERENCE_SETTING->setToolTip(QString("<b>%0 (%1)</b><br/> Show Preference Settings Window.").arg
                                    (_PREFERENCE_SETTING->text(), _PREFERENCE_SETTING->shortcut().toString()));
    _PREFERENCE_SETTING->setCheckable(false);



    LEAF_FILE->addAction(_REVEAL_IN_EXPLORER);
    LEAF_FILE->addAction(_PREFERENCE_SETTING);
    LEAF_FILE->addAction(_ABOUT_FILE_EXPLORER);
    LEAF_FILE->setExclusive(false);
    return LEAF_FILE;
}
