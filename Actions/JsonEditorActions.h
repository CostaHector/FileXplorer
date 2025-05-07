#ifndef JSONEDITORACTIONS_H
#define JSONEDITORACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QToolBar>

class JsonEditorActions : public QObject {
 public:
  explicit JsonEditorActions(QObject* parent = nullptr);

  QMenuBar* GetJsonMenuBar(QWidget* parent);
  QToolBar* GetJsonToolBar(QWidget* parent);
  QMenu* GetJsonToBeEdittedListMenu(QWidget* parent);

  QAction* _CLR_TO_BE_EDITED_LIST{nullptr};
  QActionGroup* _FILE_LOAD_ACTIONS{nullptr};

  QAction* _LAST_FILE{nullptr};
  QAction* _NEXT_FILE{nullptr};
  QAction* _DONE_AND_NEXT{nullptr};
  QAction* _AUTO_SKIP{nullptr};
  QAction* _COMPLETE_PERFS_COUNT{nullptr};
  QActionGroup* _QUICK_EDIT_ACTIONS{nullptr};

  QAction* _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD{nullptr};
  QAction* _LOWER_ALL_WORDS{nullptr};
  QAction* _FORMATTER{nullptr};
  QAction* _RELOAD_JSON_FROM_FROM_DISK{nullptr};
  QAction* _ADD_SELECTED_PERFORMER{nullptr};
  QAction* _EXTRACT_CAPITALIZED_PERFORMER{nullptr};
  QActionGroup* _TEXT_EDIT_ACTIONS{nullptr};

  QAction* _SAVE{nullptr};
  QActionGroup* _FILE_SAVE_ACTIONS{nullptr};

  QActionGroup* GetSystemActions();
  QAction* _REVEAL_IN_EXPLORER{nullptr};
  QAction* _OPEN_THIS_FILE{nullptr};
  QAction* _RENAME_THIS_FILE{nullptr};
  QActionGroup* _SYSTEM_ACTIONS{nullptr};

  QAction* _AI_HINT{nullptr};
  QAction* _LEARN_PERFORMERS_FROM_JSON{nullptr};
  QActionGroup* _AI_ACTIONS{nullptr};

  QAction* _SELECT_CURRENT_FOLDER{nullptr};
  QAction* _CONSTRUCT_JSONS_FOR_VIDS{nullptr};
  QAction* _APPEND_PERFORMERS{nullptr};
  QAction* _SET_STUDIO{nullptr};
  QAction* _CONSTRUCT_STUDIO_CAST{nullptr};
  QAction* _CONSTRUCT_STUDIO_KVP{nullptr};
  QAction* _CLR_PERFORMERS_STUDIO_VALUE{nullptr};
  QAction* _STANDARDLIZE_JSON_KEY{nullptr};
  QActionGroup* _BATCH_EDIT_TOOL_ACTIONS{nullptr};
};

JsonEditorActions& g_jsonEditorActions();

#endif  // JSONEDITORACTIONS_H
