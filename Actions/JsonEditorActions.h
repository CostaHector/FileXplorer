#ifndef JSONEDITORACTIONS_H
#define JSONEDITORACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class JsonEditorActions : public QObject {
  Q_OBJECT
 public:
  explicit JsonEditorActions(QObject* parent = nullptr);

  QAction* _EDIT_STUDIOS;
  QAction* _RELOAD_STUDIOS;

  QAction* _RELOAD_PERF_AKA;
  QAction* _EDIT_PERF_AKA;

  QAction* _SELECT_A_FOLDER_AND_LOAD_JSON;
  QAction* _EMPTY_JSONS_LISTWIDGET;
  QAction* _LAST_JSON;
  QAction* _NEXT_JSON;
  QAction* _AUTO_SKIP;
  QAction* _CONDITION_NOT;
  QActionGroup* FILES_ACTIONS;

  QAction* _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD;
  QAction* _LOWER_ALL_WORDS;
  QAction* _FORMATTER;
  QAction* _RELOAD_JSON_FROM_FROM_DISK;
  QAction* _ADD_SELECTED_PERFORMER;
  QAction* _SAVE;
  QAction* _CANCEL;
  QAction* _SUBMIT;
  QActionGroup* EDIT_ACTIONS;

  QAction* _REVEAL_IN_EXPLORER;
  QAction* _OPEN_THIS_FILE;
  QAction* _HINT;
  QAction* _LEARN_PERFORMERS_FROM_JSON;

  QAction* LOAD_JSONS_FROM_CURRENT_FOLDER;
  QAction* CONSTRUCT_JSONS_FOR_VIDS;
  QAction* JSON_ADD_PERFORMERS;
  QAction* JSON_SET_PRODUCTION_STUDIO;
  QAction* JSON_ADD_PERFORMER_KEY_VALUE_PAIR;
  QAction* JSON_ADD_PRODUCTION_STUDIO_KEY_VALUE_PAIR;
  QAction* JSON_CLEAR_PERFORMERS_PROD_STUDIO_NAME;
  QActionGroup* BATCH_FILES_ACTIONS;
};

JsonEditorActions& g_jsonEditorActions();

#endif  // JSONEDITORACTIONS_H
