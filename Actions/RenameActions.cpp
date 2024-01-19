#include "RenameActions.h"

RenameActions::RenameActions(QObject* parent) : QObject(parent), RENAME_RIBBONS(Get_Rename_Actions()), NAME_CASE(Get_CASE_Actions()) {}

RenameActions& g_renameAg() {
  static RenameActions renameActions;
  return renameActions;
}

#include <QToolBar>

class RenameActionIllustration : public QToolBar {
 public:
  explicit RenameActionIllustration(const QString& title, QWidget* parent = nullptr) : QToolBar(title, parent) {
    addAction("start");
    addActions(g_renameAg().RENAME_RIBBONS->actions());
    addSeparator();
    addAction("seperator");
    addSeparator();
    addActions(g_renameAg().NAME_CASE->actions());
    addAction("end");
  }
};

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  RenameActionIllustration renameIllustration("Rename Items", nullptr);
  renameIllustration.show();
  return a.exec();
}
#endif
