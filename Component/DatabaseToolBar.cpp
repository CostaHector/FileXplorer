#include "DatabaseToolBar.h"

DatabaseToolBar::DatabaseToolBar(const QString& title, QWidget* parent)
    : QToolBar(parent),
      sqlSearchLE(new QLineEdit),
      dbControlTB(GetDatabaseControlTB()),
      driveSelectionTB(GetTableSelectionTB()),
      sqlSearchTB(GetDBSearchTB()),
      functionsTB(GetFunctionsTB()),
      dbViewHideShow(GetHideShowToolButton()){
  addWidget(dbControlTB);
  addSeparator();
  addWidget(driveSelectionTB);
  addSeparator();
  addWidget(sqlSearchTB);
  addSeparator();
  addWidget(functionsTB);
  addSeparator();
  addWidget(dbViewHideShow);
}


//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  DatabaseToolBar dbTB("Database Toolbar");
  dbTB.show();
  return a.exec();
}
#endif
