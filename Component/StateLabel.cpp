#include "StateLabel.h"

const QString StateLabel::SAVED_STR = "saved";
const QString StateLabel::NOT_SAVED_STR = "notSaved";

#include <QHBoxLayout>
#include <QPushButton>
class StateLabelWidget : public QWidget {
 public:
  QPushButton* save;
  QPushButton* change;
  explicit StateLabelWidget(QWidget* parent = nullptr) : QWidget(parent), save(new QPushButton("Save")), change(new QPushButton("Change")) {
    StateLabel* textEdit = new StateLabel("none");
    QHBoxLayout* toasterLayout = new QHBoxLayout;
    toasterLayout->addWidget(textEdit);
    toasterLayout->addWidget(save);
    toasterLayout->addWidget(change);
    connect(save, &QPushButton::clicked, this, [textEdit]() -> void { textEdit->ToSaved(); });
    connect(change, &QPushButton::clicked, this, [textEdit]() -> void { textEdit->ToNotSaved(); });
    setLayout(toasterLayout);
  }
};

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  StateLabelWidget wid;
  wid.show();
  return a.exec();
}
#endif
