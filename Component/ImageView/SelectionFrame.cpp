#include "SelectionFrame.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QShortcut>

SelectionFrame::SelectionFrame(QWidget *parent) : QFrame{parent} {
  setAttribute(Qt::WA_StyledBackground, true);

  m_faceRecognizeResult = new QLabel{"...", this};
  m_faceRecognizeResult->setProperty("needFloatingAndCodeFontFamilyFlag", true);
  m_faceRecognizeResult->setTextInteractionFlags(Qt::TextSelectableByMouse);

  mControlToolBar = new QToolBar{"SelectionFrameControlTb", this};
  mControlToolBar->setOrientation(Qt::Orientation::Horizontal);
  mControlToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);

  QAction* pFaceRecognizeAction = mControlToolBar->addAction(QIcon{":img/FACE_RECOGNIZE"}, "Recognize Face");
  pFaceRecognizeAction->setShortcut(QKeySequence(Qt::Key_F1));
  pFaceRecognizeAction->setToolTip(QString{"<b>%1 (%2)</b><br/>Identify the person in the selection."}
                                          .arg(pFaceRecognizeAction->text(), pFaceRecognizeAction->shortcut().toString()));

  QAction* pSaveImageRectSelection = mControlToolBar->addAction(QIcon{":img/SAVED"}, "Save Selection");
  pSaveImageRectSelection->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  pSaveImageRectSelection->setToolTip(QString{"<b>%1 (%2)</b><br/>Save the selection as an image file."}
                                          .arg(pSaveImageRectSelection->text(), pSaveImageRectSelection->shortcut().toString()));
  QAction* pCancelImageRectSelection = mControlToolBar->addAction(QIcon{":/FramelessWindow/TITLEBAR_CLOSE_BUTTON"}, "Cancel Selection");
  pCancelImageRectSelection->setShortcut(QKeySequence(Qt::Key_Escape));
  pCancelImageRectSelection->setToolTip(QString{"<b>%1 (%2)</b><br/>Clear the current selection."}
                                            .arg(pCancelImageRectSelection->text(), pCancelImageRectSelection->shortcut().toString()));

  auto* v = new QVBoxLayout(this);
  v->setContentsMargins(0, 0, 0, 0);
  v->setSpacing(0);
  auto* h = new QHBoxLayout;
  h->setSpacing(2);
  h->addStretch();
  h->addWidget(mControlToolBar);
  v->addLayout(h);
  v->addWidget(m_faceRecognizeResult);

  connect(pFaceRecognizeAction, &QAction::triggered, this, &SelectionFrame::faceRecognizeRequested);
  connect(pSaveImageRectSelection,  &QAction::triggered, this, &SelectionFrame::saveRequested);
  connect(pCancelImageRectSelection, &QAction::triggered, this, &SelectionFrame::closeRequested);
}