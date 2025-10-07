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

QLineEdit* CreateKeyLineEdit(QWidget* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  QLineEdit* keyLe = new (std::nothrow) QLineEdit{parent};
  CHECK_NULLPTR_RETURN_NULLPTR(keyLe);
  keyLe->setEchoMode(QLineEdit::Password);
  keyLe->setClearButtonEnabled(false);
  keyLe->setToolTip("Maximum 16 printable ASCII characters(AES-256 key 16 bytes)");
  QRegularExpressionValidator* validator = new (std::nothrow) QRegularExpressionValidator{QRegularExpression("[\\x20-\\x7E]{0,16}"), parent};
  CHECK_NULLPTR_RETURN_NULLPTR(validator);
  keyLe->setValidator(validator);
  // printable ASCII char, count <=16
  return keyLe;
}

LoginWid::LoginWid(QWidget* parent) : QWidget{parent} {
  inputKeyLe = CreateKeyLineEdit(this);
  CHECK_NULLPTR_RETURN_VOID(inputKeyLe);
  inputKeyLe->setPlaceholderText("Enter AES decryption key");
  remeberKey = new (std::nothrow) QCheckBox{"Remember key", this};
  CHECK_NULLPTR_RETURN_VOID(remeberKey);
  remeberKey->setTristate(false);
  const int rememberState = Configuration().value("REMEMBER_KEY", Qt::CheckState::Unchecked).toInt();
  remeberKey->setCheckState(rememberState == Qt::CheckState::Checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

  autoLogin = new (std::nothrow) QCheckBox{"Log in automatically", this};
  CHECK_NULLPTR_RETURN_VOID(autoLogin);
  autoLogin->setTristate(false);
  const int autoLoginState = Configuration().value("LOG_IN_AUTOMATICALLY", Qt::CheckState::Unchecked).toInt();
  autoLogin->setCheckState(autoLoginState == Qt::CheckState::Checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

  loginLo = new (std::nothrow) QFormLayout{this};
  CHECK_NULLPTR_RETURN_VOID(loginLo);
  loginLo->addRow("Decryption Key:", inputKeyLe);
  loginLo->addRow(remeberKey, autoLogin);

  InitState();
  subscribe();
}

void LoginWid::subscribe() {
  connect(remeberKey, &QCheckBox::stateChanged, this, &LoginWid::onRemeberKeyStateChanged);
  connect(autoLogin, &QCheckBox::stateChanged, this, &LoginWid::onAutoLoginSwitchChanged);
}

bool LoginWid::onRemeberKeyStateChanged(int rememberState) {
  const CredentialUtil& credUtil = CredentialUtil::GetInst();
  Configuration().setValue("REMEMBER_KEY", rememberState);

  const QString& keyNow = GetKey();
  switch (rememberState) {
    case Qt::CheckState::Checked: {  // update into credential
      bool saveResult = credUtil.savePassword("PASSWORD_MANAGER_AES_KEY", keyNow);
      if (!saveResult) {
        LOG_WARN_P("Failed to save password", "Credential Manager[%s]", qPrintable(keyNow));
        return false;
      }
      break;
    }
    default: {  // delete from credential
      bool deleteResult = credUtil.deletePassword("PASSWORD_MANAGER_AES_KEY");
      if (!deleteResult) {
        LOG_WARN_P("Failed to save password", "Credential Manager[%s]", qPrintable(keyNow));
        return false;
      }
      break;
    }
  }
  return true;
}

void LoginWid::onAutoLoginSwitchChanged(int autoLoginState) {
  Configuration().setValue("LOG_IN_AUTOMATICALLY", autoLoginState);
  LOG_INFO_NP("Auto login switch", (autoLoginState == Qt::Checked ? "on" : "off"));
}

void LoginWid::InitState() {
  const CredentialUtil& credUtil = CredentialUtil::GetInst();
  if (AccountStorage::IsAccountCSVFileInexistOrEmpty()) {
    // 1.0 disabled login widget itself
    // 2.0 invalidate password in credential needed
    // 3.0 uncheck rememberKey/AutoLogin
    // 4.0 messageText: must register at first,
    this->setEnabled(false);
    credUtil.deletePassword("PASSWORD_MANAGER_AES_KEY");

    Configuration().setValue("REMEMBER_KEY", Qt::CheckState::Unchecked);
    Configuration().setValue("LOG_IN_AUTOMATICALLY", Qt::CheckState::Unchecked);
    remeberKey->setChecked(false);
    autoLogin->setChecked(false);

    LOG_WARN_P("Register need!", "File[%s] not exists.", qPrintable(AccountStorage::GetFullEncCsvFilePath()));
    return;
  }

  // allowed login below
  if (remeberKey->checkState() == Qt::CheckState::Checked) {
    const QString& aesKey = credUtil.readPassword("PASSWORD_MANAGER_AES_KEY");
    // key may not in system credential
    if (!aesKey.isEmpty()) {
      inputKeyLe->setText(aesKey);
    }
  }

  if (autoLogin->checkState() == Qt::CheckState::Checked && !GetKey().isEmpty()) {  // empty key. skip right now
    static constexpr int TIMER_LENGTH_MS = 2000;                                    // time count down
    LOG_INFO_P("Auto login...", "in %d ms", TIMER_LENGTH_MS);
    autoLoginTimer = new (std::nothrow) QTimer(this);
    CHECK_NULLPTR_RETURN_VOID(autoLoginTimer);
    autoLoginTimer->setInterval(TIMER_LENGTH_MS);
    autoLoginTimer->setSingleShot(true);
#ifndef RUNNING_UNIT_TESTS
    connect(autoLoginTimer, &QTimer::timeout, this, &LoginWid::AutoLoginTimeoutCallback);
    autoLoginTimer->start();
#endif
  }
}

void LoginWid::AutoLoginTimeoutCallback() {
  // user may uncheck autoLogin before timeout, so check again here
  bool isStillChecked = false;
#ifdef RUNNING_UNIT_TESTS
  isStillChecked = LoginQryWidgetMock::beforeTimeOutIsAutoLoginCheckedMock();
#else
  isStillChecked = autoLogin->isChecked();
#endif
  if (isStillChecked) {
    emit timeoutAccepted();
  }
}

RegisterWid::RegisterWid(QWidget* parent) : QWidget{parent} {
  CHECK_NULLPTR_RETURN_VOID(parent)

  inputKeyLe = CreateKeyLineEdit(this);
  CHECK_NULLPTR_RETURN_VOID(inputKeyLe);
  inputKeyLe->setPlaceholderText("Enter AES encryption key");

  inputKeyAgainLe = CreateKeyLineEdit(this);
  CHECK_NULLPTR_RETURN_VOID(inputKeyAgainLe);
  inputKeyAgainLe->setPlaceholderText("Confirm AES encryption key");

  registerLo = new (std::nothrow) QFormLayout{this};
  CHECK_NULLPTR_RETURN_VOID(registerLo);
  registerLo->addRow("Encryption Key:", inputKeyLe);
  registerLo->addRow("Confirm Key:", inputKeyAgainLe);

  InitState();
}

void RegisterWid::InitState() {
  if (!AccountStorage::IsAccountCSVFileInexistOrEmpty()) {
    // disabled register widget itself
    this->setEnabled(false);
  }
}

void RegisterWid::onTryRegisterButtonClicked() {
  if (inputKeyLe->text() != inputKeyAgainLe->text()) {
    LOG_WARN_P("Key mismatch", "first length: %d, second length: %d",  //
               inputKeyLe->text().size(), inputKeyAgainLe->text().size());
    return;
  }
  emit registerAccepted();
}

LoginQryWidget::LoginQryWidget(QWidget* parent) : QDialog{parent} {
  mLoginRegisterTab = new (std::nothrow) QTabBar{this};
  CHECK_NULLPTR_RETURN_VOID(mLoginRegisterTab);
  mLoginRegisterTab->addTab(QIcon(":/LOGIN"), ENUM_2_STR(LOGIN));
  mLoginRegisterTab->addTab(QIcon(":/REGISTER"), ENUM_2_STR(REGISTER));
  mLoginRegisterTab->setShape(QTabBar::RoundedNorth);

  mLoginWid = new (std::nothrow) LoginWid{this};
  CHECK_NULLPTR_RETURN_VOID(mLoginWid);
  mRegisterWid = new (std::nothrow) RegisterWid{this};
  CHECK_NULLPTR_RETURN_VOID(mRegisterWid);

  mLoginRegisterStk = new (std::nothrow) QStackedWidget{this};
  CHECK_NULLPTR_RETURN_VOID(mLoginRegisterStk);
  mLoginRegisterStk->insertWidget(LOGIN, mLoginWid);
  mLoginRegisterStk->insertWidget(REGISTER, mRegisterWid);

  mDlgBtnBox = new (std::nothrow) QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  CHECK_NULLPTR_RETURN_VOID(mDlgBtnBox);

  mMainLayout = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(mMainLayout);
  mMainLayout->addWidget(mLoginRegisterTab, 1);
  mMainLayout->addWidget(mLoginRegisterStk, 3);
  mMainLayout->addWidget(mDlgBtnBox, 1);

  Subscribe();
  setWindowIcon(QIcon(":/AES_KEY"));
  setWindowTitle("Pre-Login Security Check");
}

QString LoginQryWidget::getAESKey() const {
  const int curWidType = mLoginRegisterStk->currentIndex();
  switch (curWidType) {
    case LOGIN:
      return mLoginWid->GetKey();
    case REGISTER:
      return mRegisterWid->GetKey();
    default:
      LOG_E("Should never run into here. current Widget Type[%d] out of range", curWidType);
      return "";
  }
}

void LoginQryWidget::onOkButtonClicked() {
  const int curWidType = mLoginRegisterStk->currentIndex();
  switch (curWidType) {
    case LOGIN: {
      this->accept();
      return;
    }
    case REGISTER: {
      mRegisterWid->onTryRegisterButtonClicked();
      return;
    }
    default:
      LOG_E("Should never run into here. current Widget Type[%d] out of range", curWidType);
      return;
  }
}

void LoginQryWidget::Subscribe() {
  connect(mLoginRegisterTab, &QTabBar::currentChanged, mLoginRegisterStk, &QStackedWidget::setCurrentIndex);

  connect(mLoginWid, &LoginWid::timeoutAccepted, this, &QDialog::accept);

  connect(mRegisterWid, &RegisterWid::registerAccepted, this, &QDialog::accept);

  connect(mDlgBtnBox, &QDialogButtonBox::accepted, this, &LoginQryWidget::onOkButtonClicked);
  connect(mDlgBtnBox, &QDialogButtonBox::rejected, this, &QDialog::close);
}
