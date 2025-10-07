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

 signals:
  void timeoutAccepted();

 private:
  QLineEdit* inputKeyLe{nullptr};
  QCheckBox* remeberKey{nullptr};
  QCheckBox* autoLogin{nullptr};
  QFormLayout* loginLo{nullptr};
  QTimer* autoLoginTimer{nullptr};
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
  QString getAESKey() const;
  void Subscribe();

  enum TAB_TYPE {
    LOGIN = 0,
    REGISTER = 1,
  };

 public slots:
  void onOkButtonClicked();

private:
  QTabBar *mLoginRegisterTab{nullptr};

  LoginWid *mLoginWid{nullptr};
  RegisterWid *mRegisterWid{nullptr};
  QStackedWidget *mLoginRegisterStk{nullptr};

  QDialogButtonBox* mDlgBtnBox{nullptr};

  QVBoxLayout *mMainLayout{nullptr};
};
#endif
