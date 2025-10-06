#include "LoginQryWidget.h"
#include "AccountStorage.h"
#include "CredentialUtil.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include "MemoryKey.h"
#include <QLabel>
#include <QPushButton>
#include <QTimer>

LoginQryWidget::LoginQryWidget(QWidget* parent) : QDialog{parent} {
  mLoginRegisterTab = new (std::nothrow) QTabBar{this};
  CHECK_NULLPTR_RETURN_VOID(mLoginRegisterTab);
  mLoginRegisterTab->addTab(QIcon(":/LOGIN"), ENUM_2_STR(LOGIN));
  mLoginRegisterTab->addTab(QIcon(":/REGISTER"), ENUM_2_STR(REGISTER));
  mLoginRegisterTab->setShape(QTabBar::RoundedNorth);

  mLoginWid = CreateLoginPage();
  CHECK_NULLPTR_RETURN_VOID(mLoginWid);
  mRegisterWid = CreateRegisterPage();
  CHECK_NULLPTR_RETURN_VOID(mRegisterWid);

  mLoginRegisterStkLo = new (std::nothrow) QStackedWidget{this};
  CHECK_NULLPTR_RETURN_VOID(mLoginRegisterStkLo);
  mLoginRegisterStkLo->insertWidget(LOGIN, mLoginWid);
  mLoginRegisterStkLo->insertWidget(REGISTER, mRegisterWid);

  mMainLayout = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(mMainLayout);
  mMainLayout->addWidget(mLoginRegisterTab);
  mMainLayout->addWidget(mLoginRegisterStkLo);

  Subscribe();
  setWindowIcon(QIcon(":/AES_KEY"));
  setWindowTitle("Pre-Login Security Check");
}

QLineEdit* LoginQryWidget::CreateKeyLineEdit() const {
  QLineEdit* keyLe = new (std::nothrow) QLineEdit;
  CHECK_NULLPTR_RETURN_NULLPTR(keyLe);
  keyLe->setEchoMode(QLineEdit::Password);
  keyLe->setClearButtonEnabled(false);
  keyLe->setToolTip("Maximum 16 printable ASCII characters(AES-256 key 16 bytes)");
  QRegularExpressionValidator* validator = new QRegularExpressionValidator{QRegularExpression("[\\x20-\\x7E]{0,16}")};
  keyLe->setValidator(validator);
  // printable ASCII char, count <=16
  return keyLe;
}

QLabel* LoginQryWidget::CreateMessageLabel() const {
  QLabel* messageLabel = new (std::nothrow) QLabel;
  CHECK_NULLPTR_RETURN_NULLPTR(messageLabel);
  messageLabel->setStyleSheet("color: red;");
  messageLabel->setFont(StyleSheet::TEXT_EDIT_FONT);
  return messageLabel;
}

QWidget* LoginQryWidget::CreateLoginPage() {
  QLineEdit* inputKeyLe = CreateKeyLineEdit();
  inputKeyLe->setPlaceholderText("Enter AES decryption key");

  QCheckBox* remeberKey = new (std::nothrow) QCheckBox{"Remember key", this};
  CHECK_NULLPTR_RETURN_NULLPTR(remeberKey);
  remeberKey->setTristate(false);
  const int rememberState = Configuration().value("REMEMBER_KEY", Qt::CheckState::Unchecked).toInt();
  remeberKey->setCheckState(rememberState == Qt::CheckState::Checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

  QCheckBox* autoLogin = new (std::nothrow) QCheckBox{"Log in automatically", this};
  CHECK_NULLPTR_RETURN_NULLPTR(autoLogin);
  autoLogin->setTristate(false);
  const int autoLoginState = Configuration().value("LOG_IN_AUTOMATICALLY", Qt::CheckState::Unchecked).toInt();
  autoLogin->setCheckState(autoLoginState == Qt::CheckState::Checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
  QLabel* messageLabel = CreateMessageLabel();

  QDialogButtonBox* loginButtonBox = new (std::nothrow) QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  CHECK_NULLPTR_RETURN_NULLPTR(loginButtonBox);
  loginButtonBox->button(QDialogButtonBox::Ok)->setText("Login");

  QFormLayout* loginLo = new (std::nothrow) QFormLayout;
  CHECK_NULLPTR_RETURN_NULLPTR(loginLo);
  loginLo->addRow("Decryption Key:", inputKeyLe);
  loginLo->addRow(remeberKey, autoLogin);
  loginLo->addWidget(messageLabel);
  loginLo->addWidget(loginButtonBox);

  QWidget* loginWid = new (std::nothrow) QWidget{this};
  CHECK_NULLPTR_RETURN_NULLPTR(loginWid);
  loginWid->setLayout(loginLo);

  connect(loginButtonBox, &QDialogButtonBox::accepted, this, [this, inputKeyLe]() {
    mKey = inputKeyLe->text();
    accept();
  });

  const CredentialUtil& credUtil = CredentialUtil::GetInst();
  connect(remeberKey, &QCheckBox::stateChanged, this, [inputKeyLe, credUtil](int state) {
    Configuration().setValue("REMEMBER_KEY", state);
    if (state == Qt::Checked && !inputKeyLe->text().isEmpty()) {
      if (!credUtil.savePassword("PASSWORD_MANAGER_AES_KEY", inputKeyLe->text())) {
        LOG_WARN_P("Failed to save password", "Credential Manager[%s]", qPrintable(inputKeyLe->text()));
      }
    } else {
      credUtil.deletePassword("PASSWORD_MANAGER_AES_KEY");
    }
  });
  connect(autoLogin, &QCheckBox::stateChanged, this, [messageLabel](int state) {
    Configuration().setValue("LOG_IN_AUTOMATICALLY", state);
    messageLabel->setText(state == Qt::Checked ? "Auto login switch on" : "Auto login switch off");
  });

  connect(loginButtonBox, &QDialogButtonBox::rejected, this, &QDialog::close);

  if (AccountStorage::IsAccountCSVFileInexistOrEmpty()) {
    // login widget disable, must register at first, invalidate password in credential needed/remember key/AutoLogin
    credUtil.deletePassword("PASSWORD_MANAGER_AES_KEY");
    remeberKey->setChecked(false);
    autoLogin->setChecked(false);

    loginWid->setEnabled(false);
    static const QString msg{"File [" + AccountStorage::GetFullEncCsvFilePath() + "] not exists."};
    messageLabel->setText("Register first! " + msg);
    LOG_WARN_NP("Register first!", qPrintable(msg));
  } else {
    // allowed login
    if (rememberState == Qt::CheckState::Checked) {
      const QString aesKey = credUtil.readPassword("PASSWORD_MANAGER_AES_KEY");
      if (!aesKey.isEmpty()) {
        inputKeyLe->setText(aesKey);
        mKey = aesKey;
      }
    }
    if (autoLoginState == Qt::CheckState::Checked && !mKey.isEmpty()) { // empty key. skip right now
      static constexpr int TIMER_LENGTH_MS = 2000;  // time count down
      loginButtonBox->setEnabled(false);
      messageLabel->setText("Auto login in " + QString::number(TIMER_LENGTH_MS) + " ms");

      QTimer* autoLoginTimer = new QTimer(this);
      autoLoginTimer->setInterval(TIMER_LENGTH_MS);
      autoLoginTimer->setSingleShot(true);

      mAutoLoginTimeoutFunc = [loginButtonBox, autoLogin, messageLabel]() {
        loginButtonBox->setEnabled(true);
        messageLabel->clear();
#ifdef RUNNING_UNIT_TESTS
        bool isStillAutoLogin = LoginQryWidgetMock::beforeTimeOutIsAutoLoginCheckedMock();
        if (autoLogin->isChecked() != isStillAutoLogin) {
          autoLogin->setChecked(isStillAutoLogin);
        }
#endif
        if (autoLogin->isChecked()) {  // user may uncheck autoLogin before timeout, so check again here
          emit loginButtonBox->accepted();
        }
      };
      connect(autoLoginTimer, &QTimer::timeout, this, mAutoLoginTimeoutFunc);
#ifndef RUNNING_UNIT_TESTS
      autoLoginTimer->start();
#endif
    }
  }
  return loginWid;
}

QWidget* LoginQryWidget::CreateRegisterPage() {
  QLineEdit* inputKeyLe = CreateKeyLineEdit();
  CHECK_NULLPTR_RETURN_NULLPTR(inputKeyLe);
  inputKeyLe->setPlaceholderText("Enter AES encryption key");

  QLineEdit* inputKeyAgainLe = CreateKeyLineEdit();
  CHECK_NULLPTR_RETURN_NULLPTR(inputKeyAgainLe);
  inputKeyAgainLe->setPlaceholderText("Confirm AES encryption key");
  QLabel* messageLabel = CreateMessageLabel();
  CHECK_NULLPTR_RETURN_NULLPTR(messageLabel);

  QDialogButtonBox* registerButtonBox = new (std::nothrow) QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  CHECK_NULLPTR_RETURN_NULLPTR(registerButtonBox);
  registerButtonBox->button(QDialogButtonBox::Ok)->setText("Register");

  QFormLayout* registerLo = new (std::nothrow) QFormLayout;
  CHECK_NULLPTR_RETURN_NULLPTR(registerLo);
  registerLo->addRow("Encryption Key:", inputKeyLe);
  registerLo->addRow("Confirm Key:", inputKeyAgainLe);
  registerLo->addWidget(messageLabel);
  registerLo->addWidget(registerButtonBox);
  QWidget* registerWid = new (std::nothrow) QWidget;
  CHECK_NULLPTR_RETURN_NULLPTR(registerWid);
  registerWid->setLayout(registerLo);

  connect(registerButtonBox, &QDialogButtonBox::accepted, this, [this, inputKeyLe, inputKeyAgainLe, messageLabel]() {
    if (inputKeyLe->text() != inputKeyAgainLe->text()) {
      qDebug("Key mismatch - first length: %d, second length: %d", inputKeyLe->text().size(), inputKeyAgainLe->text().size());
      messageLabel->setText("The encryption keys do not match");
      return;
    }
    mKey = inputKeyLe->text();
    accept();
  });
  connect(registerButtonBox, &QDialogButtonBox::rejected, this, &QDialog::close);

  if (!AccountStorage::IsAccountCSVFileInexistOrEmpty()) {
    // register disabled, login allowed
    registerWid->setEnabled(false);
    static const QString msg{"File [" + AccountStorage::GetFullEncCsvFilePath() + "] already exists."};
    messageLabel->setText("Override danger! " + msg);
  }
  return registerWid;
}

void LoginQryWidget::Subscribe() {
  connect(mLoginRegisterTab, &QTabBar::currentChanged, mLoginRegisterStkLo, &QStackedWidget::setCurrentIndex);
}
