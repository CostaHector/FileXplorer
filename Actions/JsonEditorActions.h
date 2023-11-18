#ifndef JSONEDITORACTIONS_H
#define JSONEDITORACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class JsonEditorActions : public QObject {
 public:
  explicit JsonEditorActions(QObject* parent = nullptr);

 signals:
  QAction* _SELECT_A_FOLDER_AND_LOAD_JSON;
  QAction* _EMPTY_JSONS_LISTWIDGET;
  QAction* _LAST;
  QAction* _NEXT;
  QAction* _AUTO_SKIP;
  QAction* _CONDITION_NOT;
  QActionGroup* FILES_ACTIONS;

  QAction* _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD;
  QAction* _LOWER_ALL_WORDS;
  QAction* _FORMATTER;
  QAction* _RELOAD_JSON_FROM_FROM_DISK;
  QAction* _SAVE;
  QAction* _CANCEL;
  QAction* _SUBMIT;
  QActionGroup* EDIT_ACTIONS;

  QAction* _REVEAL_IN_EXPLORER;
  QAction* _OPEN_THIS_FILE;
  QAction* _HINT;
  QAction* _LEARN_PERFORMERS_FROM_JSON;
};

JsonEditorActions& g_jsonEditorActions();

#endif  // JSONEDITORACTIONS_H
