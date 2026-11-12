#ifndef ACCOUNTCARDEDITOR_H
#define ACCOUNTCARDEDITOR_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
class QPushButton;
class AccountCardEditor : public QWidget {
  Q_OBJECT
public:
  explicit AccountCardEditor(QWidget* parent = nullptr);

  QLineEdit*   typeEdit() const     { return mTypeEdit; }
  QLineEdit*   nameEdit() const     { return mNameEdit; }
  QLineEdit*   accountEdit() const  { return mAccountEdit; }
  QLineEdit*   pwdEdit() const      { return mPwdEdit; }
  QTextEdit*   othersEdit() const   { return mOthersEdit; }
  QPushButton* saveButton() const   { return mStageButton; }
  QPushButton* cancelButton() const { return mCancelButton; }

private:
  QLineEdit*   mTypeEdit{nullptr};
  QLineEdit*   mNameEdit{nullptr};
  QLineEdit*   mAccountEdit{nullptr};
  QLineEdit*   mPwdEdit{nullptr};
  QTextEdit*   mOthersEdit{nullptr};
  QPushButton* mStageButton{nullptr};
  QPushButton* mCancelButton{nullptr};
};
#endif // ACCOUNTCARDEDITOR_H
