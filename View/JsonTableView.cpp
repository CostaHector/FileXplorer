#include "JsonTableView.h"

JsonTableView::JsonTableView(QWidget* parent)     //
    : CustomTableView{"JSON_TABLE_VIEW", parent}  //
{
  mJsonModel = new JsonTableModel{this};
  setModel(mJsonModel);
  setEditTriggers(QAbstractItemView::EditTrigger::EditKeyPressed | QAbstractItemView::EditTrigger::AnyKeyPressed);

  InitTableView();
  setWindowTitle("Json Table View");
}

int JsonTableView::ReadADirectory(const QString& path) {
  return mJsonModel->ReadADirectory(path);
}

int JsonTableView::AppendADirectory(const QString& path) {
  return mJsonModel->AppendADirectory(path);
}

#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  JsonTableView jtv;
  jtv.show();
  jtv.ReadADirectory("E:/MenPreview");
  jtv.AppendADirectory("E:/Men Preview");
  a.exec();
  return 0;
}
#endif
