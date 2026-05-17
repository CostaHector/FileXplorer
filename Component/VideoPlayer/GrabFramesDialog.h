#ifndef GRABFRAMESDIALOG_H
#define GRABFRAMESDIALOG_H

#include <QDialog>
#include <QSpinBox>

class GrabFramesDialog : public QDialog {
  Q_OBJECT
public:
  explicit GrabFramesDialog(QWidget *parent = nullptr);
  int startPositionSecond() const;
  int intervalSecond() const;
  int framesCount() const;

private:
  QSpinBox *m_startSpin{nullptr};
  QSpinBox *m_intervalSpin{nullptr};
  QSpinBox *m_countSpin{nullptr};
};

#endif // GRABFRAMESDIALOG_H
