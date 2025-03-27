#include "StateLabel.h"
#include <QPixmap>

QPixmap GetLabelStatusPixmap(StateLabel::LABEL_STATUS_E status) {
  if (status < StateLabel::BEGIN || status >= StateLabel::BUTT) {
    qWarning("status[%d] out of bound", status);
    return {};
  }
  static const QPixmap labelSavedStatusPxp[StateLabel::BUTT]       //
      {QPixmap(":img/SAVED").scaled(24, 24, Qt::KeepAspectRatio),  //
       QPixmap(":img/NOT_SAVED").scaled(24, 24, Qt::KeepAspectRatio)};
  return labelSavedStatusPxp[status];
}

StateLabel::StateLabel(const QString& text, QWidget* parent)  //
    : QLabel{text, parent} {
  setPixmap(GetLabelStatusPixmap(m_currentState));
  setAlignment(Qt::AlignRight);
}

void StateLabel::ToSaved() {
  m_currentState = SAVED;
  setPixmap(GetLabelStatusPixmap(m_currentState));
}
void StateLabel::ToNotSaved() {
  m_currentState = NOT_SAVED;
  setPixmap(GetLabelStatusPixmap(m_currentState));
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

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

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  StateLabelWidget wid;
  wid.show();
  return a.exec();
}
#endif
