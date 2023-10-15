#ifndef FILELEAFACTION_H
#define FILELEAFACTION_H
#include <QAction>
#include <QActionGroup>

class FileLeafActions: public QObject{
public:
    explicit FileLeafActions(QObject* parent=nullptr);
    QActionGroup* GetLeafTabActions();
    QAction* _PREFERENCE_SETTING;
    QAction* _ABOUT_FILE_EXPLORER;

    QActionGroup* LEAF_FILE;
};

FileLeafActions& g_fileLeafActions();

#endif // FILELEAFACTION_H
