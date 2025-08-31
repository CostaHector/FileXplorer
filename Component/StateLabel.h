#ifndef STATELABEL_H
#define STATELABEL_H

#include <QLabel>

class StateLabel : public QLabel {
 public:
  typedef enum tagLABEL_STATUS_E {
    BEGIN = 0,      //
    SAVED = BEGIN,  //
    NOT_SAVED = 1,  //
    BUTT = 2
  } LABEL_STATUS_E;
  explicit StateLabel(const QString& text, QWidget* parent = nullptr);
  LABEL_STATUS_E state() const {return m_currentState;}
 public slots:
  void ToSaved();
  void ToNotSaved();

 private:
  LABEL_STATUS_E m_currentState{SAVED};
};

#endif  // STATELABEL_H
