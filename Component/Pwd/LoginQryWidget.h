#ifndef LOGIN_QRY_WIDGET_H
#define LOGIN_QRY_WIDGET_H
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QStackedLayout>
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

class LoginQryWidget : public QDialog {
public:
  explicit LoginQryWidget(QWidget *parent = nullptr);
  QString getAESKey() const { return mKey; }
  void Subscribe();

  enum TAB_TYPE {
    LOGIN = 0,
    REGISTER = 1,
  };

private:
  QLineEdit *CreateKeyLineEdit() const;
  QLabel *CreateMessageLabel() const;

  QWidget *CreateLoginPage();
  QWidget *CreateRegisterPage();

  QString mKey;

  QWidget *mLoginWid{nullptr}, *mRegisterWid{nullptr};
  QTabBar *mLoginRegisterTab{nullptr};
  QStackedLayout *mLoginRegisterStkLo{nullptr};

  QVBoxLayout *mMainLayout{nullptr};

  std::function<void(void)> mAutoLoginTimeoutFunc{nullptr};
};
#endif
