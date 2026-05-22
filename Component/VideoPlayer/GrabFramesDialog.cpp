#include "GrabFramesDialog.h"
#include "ImageTool.h"
#include <QFormLayout>
#include <QDialogButtonBox>

GrabFramesDialog::GrabFramesDialog(QWidget *parent)
  : QDialog(parent) {
  m_startSpin = new QSpinBox(this);
  m_startSpin->setRange(0, 3600);
  m_startSpin->setValue(ImageTool::DEFAULT_START_POSITION_SECOND);
  m_startSpin->setSuffix(tr(" s"));

  m_intervalSpin = new QSpinBox(this);
  m_intervalSpin->setRange(1, 600);
  m_intervalSpin->setValue(ImageTool::DEFAULT_INTERVAL_SECOND);
  m_intervalSpin->setSuffix(tr(" s"));

  m_countSpin = new QSpinBox(this);
  m_countSpin->setRange(1, 100);
  m_countSpin->setValue(ImageTool::DEFAULT_FRAMES_COUNT);
  m_countSpin->setSuffix(tr(" cnt"));

  // 布局
  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow(tr("Start Position:"), m_startSpin);
  formLayout->addRow(tr("Grab Interval:"), m_intervalSpin);
  formLayout->addRow(tr("Grab Count:"), m_countSpin);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout *mainLayout = new QVBoxLayout{this};
  mainLayout->addLayout(formLayout);
  mainLayout->addStretch();
  mainLayout->addWidget(buttonBox);

  setLayout(mainLayout);
  setWindowIcon(QIcon{":/VideoPlayer/GRAB_FRAMES"});
  setWindowTitle(tr("Grab Frames Settings"));
}

int GrabFramesDialog::startPositionSecond() const {
  return m_startSpin == nullptr ? ImageTool::DEFAULT_START_POSITION_SECOND : m_startSpin->value();
}
int GrabFramesDialog::intervalSecond() const {
  return m_intervalSpin == nullptr ? ImageTool::DEFAULT_INTERVAL_SECOND : m_intervalSpin->value();
}
int GrabFramesDialog::framesCount() const {
  return m_countSpin == nullptr ? ImageTool::DEFAULT_FRAMES_COUNT : m_countSpin->value();
}
