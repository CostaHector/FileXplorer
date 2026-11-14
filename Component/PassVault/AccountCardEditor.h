#ifndef ACCOUNTCARDEDITOR_H
#define ACCOUNTCARDEDITOR_H

#include <QWidget>

class QPushButton;
class QLineEdit;
class QPlainTextEdit;

class AccountCardEditor : public QWidget {
  Q_OBJECT
public:
  explicit AccountCardEditor(QWidget* parent = nullptr);

  QLineEdit*   typeEdit() const      { return mTypeEdit; }
  QLineEdit*   nameEdit() const      { return mNameEdit; }
  QLineEdit*   accountEdit() const   { return mAccountEdit; }
  QLineEdit*   pwdEdit() const       { return mPwdEdit; }
  QPlainTextEdit* othersEdit() const { return mOthersEdit; }

signals:
  void stageCurrentChanges();
  void revertCurrentChanges();

private:
  QLineEdit*   mTypeEdit{nullptr};
  QLineEdit*   mNameEdit{nullptr};
  QLineEdit*   mAccountEdit{nullptr};
  QLineEdit*   mPwdEdit{nullptr};
  QPlainTextEdit* mOthersEdit{nullptr};

  QPushButton* mRevertButton{nullptr};
  QPushButton* mStageButton{nullptr};
};
#endif // ACCOUNTCARDEDITOR_H
