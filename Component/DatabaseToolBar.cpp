#include "DatabaseToolBar.h"

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
