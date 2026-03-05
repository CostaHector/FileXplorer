#ifndef LOGIN_QRY_WIDGET_H
#define LOGIN_QRY_WIDGET_H
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QTimer>

#ifdef RUNNING_UNIT_TESTS
namespace LoginQryWidgetMock {
inline bool& beforeTimeOutIsAutoLoginCheckedMock() {
  static bool isAutoLoginChecked = false;
  return isAutoLoginChecked;
}
inline void clear() {
  beforeTimeOutIsAutoLoginCheckedMock() = false;
}
}
#endif

QString GetCredTargetName();
bool QueryWhenPasswordBookFileNotExist(QWidget* parent);

class LoginWid : public QWidget {
  Q_OBJECT
 public:
  explicit LoginWid(QWidget* parent = nullptr);

  QString GetKey() const { return inputKeyLe->text(); }

  void subscribe();

  bool onRemeberKeyStateChanged(int rememberState);
  void onAutoLoginSwitchChanged(int autoLoginState);

  void InitState();

  void AutoLoginTimeoutCallback();

  bool isAutoLoginEnabled() const {
    return autoLogin->isChecked();
  }
  bool isRememberEnabled() const {
    return remeberKey->isChecked();
  }
  void startTimer();
  void stopTimer();
 signals:
  void timeoutAccepted();

 private:
  QLineEdit* inputKeyLe{nullptr};
  QCheckBox* remeberKey{nullptr};
  QCheckBox* autoLogin{nullptr};
  QLabel* mMessage{nullptr};
  QFormLayout* loginLo{nullptr};
  QTimer autoLoginTimer;
};

class RegisterWid : public QWidget {
  Q_OBJECT
 public:
  explicit RegisterWid(QWidget* parent = nullptr);
  void InitState();

  QString GetKey() const { return inputKeyLe->text(); }

 signals:
  void registerAccepted();

 public slots:
  void onTryRegisterButtonClicked();

 private:
  QLineEdit* inputKeyLe{nullptr};
  QLineEdit* inputKeyAgainLe{nullptr};
  QFormLayout* registerLo{nullptr};
};

class LoginQryWidget : public QDialog {
  Q_OBJECT
public:
  explicit LoginQryWidget(QWidget *parent = nullptr);
  void hideEvent(QHideEvent* event) override;
  QString getAESKey() const;
  void Subscribe();
  enum TAB_TYPE {
    LOGIN = 0,
    REGISTER = 1,
  };

 public slots:
  void onStartTimer();
  void onOkButtonClicked();
  void onCancelButtonClicked();

private:
  QTabBar *mLoginRegisterTab{nullptr};

  LoginWid *mLoginWid{nullptr};
  RegisterWid *mRegisterWid{nullptr};
  QStackedWidget *mLoginRegisterStk{nullptr};

  QDialogButtonBox* mDlgBtnBox{nullptr};

  QVBoxLayout *mMainLayout{nullptr};
};
#endif
