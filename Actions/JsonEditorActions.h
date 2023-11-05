#ifndef JSONEDITORACTIONS_H
#define JSONEDITORACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class JsonEditorActions : public QObject {
 public:
  explicit JsonEditorActions(QObject* parent = nullptr);

 signals:
  QAction* _FORMATTER;
  QAction* _LAST;
  QAction* _NEXT;
  QAction* _AUTO_SKIP;
  QAction* _CONDITION_NOT;
  QAction* _SUBMIT;
  QAction* _SAVE;
  QAction* _CANCEL;
  QAction* _LOAD;
  QAction* _EMPTY;
  QAction* _RELOAD_FROM_JSON_FILE;

  QActionGroup* JSON_EDITOR_ACTIONS;

  QAction* _REVEAL_IN_EXPLORER;
  QAction* _OPEN_THIS_FILE;

  QAction* _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD;
  QAction* _LOWER_ALL_WORDS;
  QAction* _HINT;
  QAction* _LEARN_PERFORMERS_FROM_JSON;
};

JsonEditorActions& g_jsonEditorActions();

#endif  // JSONEDITORACTIONS_H
