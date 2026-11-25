#ifndef CASTDBCARDEDITOR_H
#define CASTDBCARDEDITOR_H

#include <QWidget>

class QPushButton;
class QLineEdit;
class QSpinBox;
class QPlainTextEdit;

class CastDBCardEditor : public QWidget {
  Q_OBJECT
public:
  explicit CastDBCardEditor(QWidget* parent = nullptr);

  QLineEdit*      mActorNameEdit{nullptr};
  QSpinBox*       mRateEdit{nullptr};
  QLineEdit*      mAliasEdit{nullptr};
  QLineEdit*      mTagsEdit{nullptr};
  QLineEdit*      mOriBelongToEdit{nullptr};
  QSpinBox*       mHeightEdit{nullptr};
  QSpinBox*       mSizeEdit{nullptr};
  QPlainTextEdit* mDetailEdit{nullptr};

#ifdef RUNNING_UNIT_TESTS
  QPushButton* GetStageButton()      { return mStageButton; }
  QPushButton* GetRevertButton()     { return mRevertButton; }
#endif

signals:
  void stageCurrentChanges();
  void revertCurrentChanges();

private:
  QPushButton* mRevertButton{nullptr};
  QPushButton* mStageButton{nullptr};
};

#endif // CASTDBCARDEDITOR_H
