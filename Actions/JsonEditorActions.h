#ifndef JSONEDITORACTIONS_H
#define JSONEDITORACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QMenuBar>
#include <QMenu>
#include <QObject>
#include <QToolBar>

class JsonEditorActions : public QObject {
  Q_OBJECT
 public:
  explicit JsonEditorActions(QObject* parent = nullptr);

  QMenuBar* GetJsonMenuBar(QWidget* parent);
  QToolBar* GetJsonToolBar(QWidget* parent);
  QMenu* GetJsonToBeEdittedListMenu(QWidget* parent);

  QAction* _STUDIO_INFORMATION;
  QAction* _EDIT_STUDIOS;
  QAction* _RELOAD_STUDIOS;
  QActionGroup* _STUDIO_ACTIONS;

  QAction* _PERFORMERS_INFORMATION;
  QAction* _EDIT_PERFS;
  QAction* _RELOAD_PERFS;
  QActionGroup* _PERFS_ACTIONS;

  QAction* _AKA_PERFORMERS_INFORMATION;
  QAction* _EDIT_PERF_AKA;
  QAction* _RELOAD_PERF_AKA;
  QActionGroup* _PERFS_AKA_ACTIONS;

  QAction* _BROWSE_AND_SELECT_THE_FOLDER;
  QAction* _CLR_TO_BE_EDITED_LIST;
  QActionGroup* _FILE_LOAD_ACTIONS;

  QAction* _LAST_FILE;
  QAction* _NEXT_FILE;
  QAction* _AUTO_SKIP;
  QAction* _COMPLETE_PERFS_COUNT;
  QActionGroup* _QUICK_EDIT_ACTIONS;

  QAction* _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD;
  QAction* _LOWER_ALL_WORDS;
  QAction* _FORMATTER;
  QAction* _RELOAD_JSON_FROM_FROM_DISK;
  QAction* _ADD_SELECTED_PERFORMER;
  QActionGroup* _TEXT_EDIT_ACTIONS;

  QAction* _SAVE;
  QAction* _CANCEL;
  QAction* _SUBMIT;
  QActionGroup* _FILE_SAVE_ACTIONS;

  QAction* _REVEAL_IN_EXPLORER;
  QAction* _OPEN_THIS_FILE;
  QActionGroup* _SYSTEM_ACTIONS;

  QAction* _AI_HINT;
  QAction* _LEARN_PERFORMERS_FROM_JSON;
  QActionGroup* _AI_ACTIONS;

  QAction* _SELECT_CURRENT_FOLDER;
  QAction* _CONSTRUCT_JSONS_FOR_VIDS;
  QAction* _APPEND_PERFORMERS;
  QAction* _SET_STUDIO;
  QAction* _CONSTRUCT_PERFORMER_KVP;
  QAction* _CONSTRUCT_STUDIO_KVP;
  QAction* _CLR_PERFORMERS_STUDIO_VALUE;
  QActionGroup* _BATCH_EDIT_TOOL_ACTIONS;
};

JsonEditorActions& g_jsonEditorActions();

#endif  // JSONEDITORACTIONS_H
