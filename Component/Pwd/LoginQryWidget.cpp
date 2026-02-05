#include "LoginQryWidget.h"
#include "AccountStorage.h"
#include "CredentialUtil.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include "FileLeafAction.h"
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

QString GetCredTargetName() {
#ifdef RUNNING_UNIT_TESTS
  return "PASSWORD_MANAGER_AES_KEY_TEST";
#else
  return "PASSWORD_MANAGER_AES_KEY";
#endif
}

QLineEdit* CreateKeyLineEdit(QWidget* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  QLineEdit* keyLe = new (std::nothrow) QLineEdit{parent};
  CHECK_NULLPTR_RETURN_NULLPTR(keyLe);
  keyLe->setEchoMode(QLineEdit::Password);
  keyLe->setClearButtonEnabled(false);
  keyLe->setToolTip("Maximum 16 printable ASCII characters(AES-256 key 16 bytes)");
  QRegularExpressionValidator* validator = new (std::nothrow)
      QRegularExpressionValidator{QRegularExpression("[\\x20-\\x7E]{0,16}"), parent};
  CHECK_NULLPTR_RETURN_NULLPTR(validator);
  keyLe->setValidator(validator);
  // printable ASCII char, count <=16
  return keyLe;
}

LoginWid::LoginWid(QWidget* parent)
  : QWidget{parent} {
  inputKeyLe = CreateKeyLineEdit(this);
  CHECK_NULLPTR_RETURN_VOID(inputKeyLe);
  inputKeyLe->setPlaceholderText("Enter AES decryption key");
  remeberKey = new (std::nothrow) QCheckBox{tr("Remember key"), this};
  CHECK_NULLPTR_RETURN_VOID(remeberKey);
  remeberKey->setTristate(false);
  const int rememberState = Configuration().value("REMEMBER_KEY", Qt::CheckState::Unchecked).toInt();
  remeberKey->setCheckState(rememberState == Qt::CheckState::Checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

  autoLogin = new (std::nothrow) QCheckBox{tr("Log in automatically"), this};
  CHECK_NULLPTR_RETURN_VOID(autoLogin);
  autoLogin->setTristate(false);
  const int autoLoginState = Configuration().value("LOG_IN_AUTOMATICALLY", Qt::CheckState::Unchecked).toInt();
  autoLogin->setCheckState(autoLoginState == Qt::CheckState::Checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

  mMessage = new (std::nothrow) QLabel{this};

  loginLo = new (std::nothrow) QFormLayout{this};
  CHECK_NULLPTR_RETURN_VOID(loginLo);
  loginLo->addRow(tr("Decryption Key"), inputKeyLe);
  loginLo->addRow(remeberKey, autoLogin);
  loginLo->addRow(mMessage);

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
    case Qt::CheckState::Checked: { // update into credential
      bool saveResult = credUtil.savePassword(GetCredTargetName(), keyNow);
      if (!saveResult) {
        LOG_WARN_P("Failed to save password", "Credential Manager[%s]", qPrintable(keyNow));
        return false;
      }
      mMessage->setText("Password <b>saved</b> to system credential succeed");
      break;
    }
    default: { // delete from credential
      bool deleteResult = credUtil.deletePassword(GetCredTargetName());
      if (!deleteResult) {
        LOG_WARN_P("Failed to save password", "Credential Manager[%s]", qPrintable(keyNow));
        return false;
      }
      mMessage->setText("Password <b>delete</b> from system credential");
      break;
    }
  }
  return true;
}

void LoginWid::onAutoLoginSwitchChanged(int autoLoginState) {
  if (autoLoginState == Qt::Checked) {
    remeberKey->setChecked(true);
  }
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
    credUtil.deletePassword(GetCredTargetName());

    Configuration().setValue("REMEMBER_KEY", Qt::CheckState::Unchecked);
    Configuration().setValue("LOG_IN_AUTOMATICALLY", Qt::CheckState::Unchecked);
    remeberKey->setChecked(false);
    autoLogin->setChecked(false);

    LOG_WARN_P("Register need!", "File[%s] not exists.", qPrintable(AccountStorage::GetFullEncCsvFilePath()));
    return;
  }

  // allowed login below
  if (remeberKey->checkState() == Qt::CheckState::Checked) {
    const QString& aesKey = credUtil.readPassword(GetCredTargetName());
    // key may not in system credential
    if (!aesKey.isEmpty()) {
      inputKeyLe->setText(aesKey);
    }
  }

  if (autoLogin->checkState() == Qt::CheckState::Checked && !GetKey().isEmpty()) { // empty key. skip right now
    static constexpr int TIMER_LENGTH_MS = 2000;                                   // time count down
    mMessage->setText(QString("Will auto Login in %1(ms)").arg(TIMER_LENGTH_MS));

    autoLoginTimer.setInterval(TIMER_LENGTH_MS);
    autoLoginTimer.setSingleShot(true);
    connect(&autoLoginTimer, &QTimer::timeout, this, &LoginWid::AutoLoginTimeoutCallback);
#ifndef RUNNING_UNIT_TESTS
    autoLoginTimer.start();
#endif
  }
}

void LoginWid::AutoLoginTimeoutCallback() {
  mMessage->setText("");
  // user may uncheck autoLogin before timeout, so check again here
  bool isAutoLoginStillChecked = false;
#ifdef RUNNING_UNIT_TESTS
  isAutoLoginStillChecked = LoginQryWidgetMock::beforeTimeOutIsAutoLoginCheckedMock();
#else
  isAutoLoginStillChecked = isAutoLoginEnabled();
#endif
  if (isAutoLoginStillChecked) {
    emit timeoutAccepted();
  }
}

void LoginWid::stopTimer() {
  if (autoLoginTimer.isActive()) {
    autoLoginTimer.stop();
  }
}

RegisterWid::RegisterWid(QWidget* parent)
  : QWidget{parent} {
  CHECK_NULLPTR_RETURN_VOID(parent)

  inputKeyLe = CreateKeyLineEdit(this);
  CHECK_NULLPTR_RETURN_VOID(inputKeyLe);
  inputKeyLe->setPlaceholderText("Enter AES encryption key");

  inputKeyAgainLe = CreateKeyLineEdit(this);
  CHECK_NULLPTR_RETURN_VOID(inputKeyAgainLe);
  inputKeyAgainLe->setPlaceholderText("Confirm AES encryption key");

  registerLo = new (std::nothrow) QFormLayout{this};
  CHECK_NULLPTR_RETURN_VOID(registerLo);
  registerLo->addRow(tr("Encryption Key"), inputKeyLe);
  registerLo->addRow(tr("Confirm Key"), inputKeyAgainLe);

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
    LOG_WARN_P("Key mismatch",
               "first length: %lld, second length: %lld", //
               inputKeyLe->text().size(),
               inputKeyAgainLe->text().size());
    return;
  }
  emit registerAccepted();
}

bool QueryWhenPasswordBookFileNotExist(QWidget* parent) {
  if (QMessageBox::question(parent, "Password Book not exist", "Select manually right now?") != QMessageBox::StandardButton::Yes) {
    return true;
  }
  const QString fileName = QFileDialog::getOpenFileName(parent, "Select password book", QDir::homePath(), "CSV Files (*)");
  if (!QFile::exists(fileName)) {
    LOG_ERR_P("Select abort", "file[%s] not exist", qPrintable(fileName));
    return false;
  }
  const QString dstFileName = AccountStorage::GetFullEncCsvFilePath();
  if (QFile::exists(dstFileName)) {
    LOG_WARN_P("Copy abort", "file[%s] already exist", qPrintable(dstFileName));
    return false;
  }
  bool cpyRet = QFile::copy(fileName, dstFileName);
  LOG_OE_P(cpyRet, "Select file", "from:[%s]\nto:[%s]", qPrintable(fileName), qPrintable(dstFileName));
  return cpyRet;
}

LoginQryWidget::LoginQryWidget(QWidget* parent)
  : QDialog{parent} {
  if (AccountStorage::IsAccountCSVFileInexistOrEmpty()) {
#ifndef RUNNING_UNIT_TESTS
    QueryWhenPasswordBookFileNotExist(this);
#endif
  }
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

  if (mLoginWid->isEnabled()) {
    mLoginRegisterTab->setCurrentIndex(LOGIN);
    mLoginRegisterStk->setCurrentIndex(LOGIN);
  } else if (mRegisterWid->isEnabled()) {
    mLoginRegisterTab->setCurrentIndex(REGISTER);
    mLoginRegisterStk->setCurrentIndex(REGISTER);
  }

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

void LoginQryWidget::hideEvent(QHideEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  g_fileLeafActions()._PWD_BOOK->setChecked(false);
  QDialog::hideEvent(event);
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
  mLoginWid->stopTimer();
  const int curWidType = mLoginRegisterStk->currentIndex();
  switch (curWidType) {
    case LOGIN: {
      this->accept();
      if (mLoginWid->isRememberEnabled()) {
        const CredentialUtil& credUtil = CredentialUtil::GetInst();
        credUtil.savePassword(GetCredTargetName(), getAESKey());
      }
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

void LoginQryWidget::onCancelButtonClicked() {
  mLoginWid->stopTimer();
  close();
}

void LoginQryWidget::Subscribe() {
  connect(mLoginRegisterTab, &QTabBar::currentChanged, mLoginRegisterStk, &QStackedWidget::setCurrentIndex);

  connect(mLoginWid, &LoginWid::timeoutAccepted, this, &QDialog::accept);

  connect(mRegisterWid, &RegisterWid::registerAccepted, this, &QDialog::accept);

  connect(mDlgBtnBox, &QDialogButtonBox::accepted, this, &LoginQryWidget::onOkButtonClicked);
  connect(mDlgBtnBox, &QDialogButtonBox::rejected, this, &LoginQryWidget::onCancelButtonClicked);
}
