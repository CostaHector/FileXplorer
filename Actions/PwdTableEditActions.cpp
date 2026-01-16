#include "PwdTableEditActions.h"

PwdTableEditActions::PwdTableEditActions(QObject* parent)
  : QObject{parent} {
  INSERT_A_ROW = new QAction{QIcon{":/edit/INSERT_ROW_BEFORE"}, tr("Insert a row"), this};
  INSERT_A_ROW->setToolTip("Insert a row before current line");
  INSERT_ROWS = new QAction{QIcon{":/edit/INSERT_ROWS"}, tr("Insert rows"), this};
  INSERT_ROWS->setToolTip("Insert row(s) before current line");
  APPEND_ROWS = new QAction{QIcon{":/edit/APPEND_ROWS"}, tr("Append rows"), this};
  APPEND_ROWS->setToolTip("Append row(s) behind last row");
  DELETE_ROWS = new QAction{QIcon{":/edit/DELETE_ROWS"}, tr("Delete rows"), this};
  DELETE_ROWS->setToolTip("Delete row(s) selected currently");

  LOAD_FROM_INPUT = new QAction{QIcon{":/edit/LOAD_FROM_INPUT"}, tr("Load from input"), this};
  LOAD_FROM_INPUT->setToolTip("Load records from contents in CSV");

  OPEN_DIRECTORY = new QAction{QIcon{":/OPEN_DIRECTORY"}, tr("Open directory"), this};
  OPEN_DIRECTORY->setToolTip("Launches the system file explorer at the location of the data file(s).");
  SHOW_PLAIN_CSV_CONTENT = new QAction{QIcon{":/edit/SHOW_CSV_CONTENTS"}, tr("Show CSV contents"), this};
  SHOW_PLAIN_CSV_CONTENT->setToolTip(
      "Show CSV contents in message box");

  EXPORT_TO_PLAIN_CSV = new QAction{QIcon{":/edit/EXPORT"}, tr("Export to plain CSV"), this};
  EXPORT_TO_PLAIN_CSV->setToolTip(
      "Export account table records to CSV file in plaintext (<b>unencrypted, human-readable</b>)");

  SEARCH_BY = new QAction{QIcon{":/edit/SEARCH_BY"}, tr("Search by"), this};

  SAVE_CHANGES = new QAction{QIcon{":/edit/SAVE_CHANGES"}, tr("Save changes"), this};
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
