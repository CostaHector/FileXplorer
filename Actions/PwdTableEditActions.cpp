#include "PwdTableEditActions.h"

PwdTableEditActions::PwdTableEditActions(QObject* parent)
  : QObject{parent} {
  INSERT_A_ROW = new QAction{QIcon{":/edit/INSERT_ROW_BEFORE"}, "Insert a row"};
  INSERT_A_ROW->setToolTip("Insert a row before current line");
  INSERT_ROWS = new QAction{QIcon{":/edit/INSERT_ROWS"}, "Insert rows"};
  INSERT_ROWS->setToolTip("Insert row(s) before current line");
  APPEND_ROWS = new QAction{QIcon{":/edit/APPEND_ROWS"}, "Append rows"};
  APPEND_ROWS->setToolTip("Append row(s) behind last row");
  DELETE_ROWS = new QAction{QIcon{":/edit/DELETE_ROWS"}, "Delete rows"};
  DELETE_ROWS->setToolTip("Delete row(s) selected currently");

  LOAD_FROM_INPUT = new QAction{QIcon{":/edit/LOAD_FROM_INPUT"}, "Load from input"};
  LOAD_FROM_INPUT->setToolTip("Load records from contents in CSV");

  OPEN_DIRECTORY = new QAction{QIcon{":/OPEN_DIRECTORY"}, "Open directory"};
  OPEN_DIRECTORY->setToolTip("Launches the system file explorer at the location of the data file(s).");
  SHOW_PLAIN_CSV_CONTENT = new QAction{QIcon{":/edit/SHOW_CSV_CONTENTS"}, "Show CSV contents"};
  SHOW_PLAIN_CSV_CONTENT->setToolTip(
      "Show CSV contents in message box");

  EXPORT_TO_PLAIN_CSV = new QAction{QIcon{":/edit/EXPORT"}, "Export to plain CSV"};
  EXPORT_TO_PLAIN_CSV->setToolTip(
      "Export account table records to CSV file in plaintext (<b>unencrypted, human-readable</b>)");

  SEARCH_BY = new QAction{QIcon{":/edit/SEARCH_BY"}, "Search by"};

  SAVE_CHANGES = new QAction{QIcon{":/edit/SAVE_CHANGES"}, "Save changes"};
  SAVE_CHANGES->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  SAVE_CHANGES->setToolTip(QString("<b>%1 (%2)</b><br/>Save All changes to local file.")
                               .arg(SAVE_CHANGES->text(), SAVE_CHANGES->shortcut().toString()));

  ROW_EDIT_AG = new QActionGroup{this};
  ROW_EDIT_AG->addAction(INSERT_A_ROW);
  ROW_EDIT_AG->addAction(INSERT_ROWS);
  ROW_EDIT_AG->addAction(APPEND_ROWS);
  ROW_EDIT_AG->addAction(DELETE_ROWS);
}

PwdTableEditActions& GetTableEditActionsInst() {
  static PwdTableEditActions ins;
  return ins;
}
