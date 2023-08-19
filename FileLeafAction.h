#ifndef FILELEAFACTION_H
#define FILELEAFACTION_H
#include <QAction>
#include <QActionGroup>

class FileLeafAction
{
public:
    FileLeafAction();
    static QActionGroup* ag();
    static QAction* _REVEAL_IN_EXPLORER;
    static QAction* _PREFERENCE_SETTING;
    static QAction* _ABOUT_FILE_EXPLORER;
    static QActionGroup* LEAF_FILE;
};
#endif // FILELEAFACTION_H
