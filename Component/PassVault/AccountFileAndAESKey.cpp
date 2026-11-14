#include "AccountFileAndAESKey.h"
#include "NotificatorMacro.h"
#include "CredentialUtil.h"

#include <QFileInfo>
#include <QTimer>

QString GetPasswordBookCredTargetName() {
#ifdef RUNNING_UNIT_TESTS
  return "PASSWORD_MANAGER_AES_KEY_TEST";
#else
  return "PASSWORD_MANAGER_AES_KEY";
#endif
}

AccountFileAndAESKey::AccountFileAndAESKey(const QString& specifyEncFileFullPath, const QString& specifyAesKey, QWidget* parent) : QWidget{parent} {
  mEncFileFullPathLE = new QLineEdit{this};

  mAesKeyLE = new QLineEdit{this};
  mAesKeyLE->setEchoMode(QLineEdit::PasswordEchoOnEdit);
  mAesKeyConfirmLE = new QLineEdit{this};
  mAesKeyConfirmLE->setEchoMode(QLineEdit::PasswordEchoOnEdit);

  mSaveCurrentAesKeyButton = new QPushButton{tr("Save Key"), this};
  mSaveCurrentAesKeyButton->setToolTip("Save the current AES key text to the credential manager immediately. The input is not validated at all.");

  mApplyButton = new QPushButton{tr("Apply"), this};

  mLayout = new QFormLayout(this);
  mLayout->addRow(tr("Encrypt File:"), mEncFileFullPathLE);
  mLayout->addRow(tr("AES Key:"), mAesKeyLE);
  mLayout->addRow(tr("Confirm Key:"), mAesKeyConfirmLE);
  mLayout->addRow(mSaveCurrentAesKeyButton, mApplyButton);
  mLayout->setSpacing(1);
  mLayout->setContentsMargins(0, 0, 0, 0);

  InitAccountFullPath(specifyEncFileFullPath);
  InitAesKey(specifyAesKey);

  subscribe();
}

bool AccountFileAndAESKey::IsNeedRegisterCore(const QString& path) {
  const QFileInfo fi{path};
  return !fi.exists() || fi.size() == 0;
}

bool AccountFileAndAESKey::IsNeedRegister() const {
  return IsNeedRegisterCore(GetAccountFileFullPath());
}

void AccountFileAndAESKey::SetReEnterVisible(bool visible) {
  mAesKeyConfirmLE->setHidden(!visible);
  if (QWidget* label = mLayout->labelForField(mAesKeyConfirmLE)) {
    label->setHidden(!visible);
  }
}

void AccountFileAndAESKey::subscribe() {
  connect(mEncFileFullPathLE, &QLineEdit::returnPressed, this, &AccountFileAndAESKey::onEncFileFullPathLineEditChanged);
  connect(mAesKeyLE, &QLineEdit::returnPressed, this, &AccountFileAndAESKey::TryCommitAesKey);
  connect(mAesKeyConfirmLE, &QLineEdit::returnPressed, this, &AccountFileAndAESKey::TryCommitAesKey);
  connect(mApplyButton, &QPushButton::clicked, this, &AccountFileAndAESKey::onApply);
  connect(mSaveCurrentAesKeyButton, &QPushButton::clicked, this, &AccountFileAndAESKey::onSaveCurrentInputKey);
}

void AccountFileAndAESKey::onEncFileFullPathLineEditChanged() {
  const bool bNeedReg{IsNeedRegister()};
  SetReEnterVisible(bNeedReg);
  emit encFileFullPathChanged(mEncFileFullPathLE->text());
}

bool AccountFileAndAESKey::ValidateAesKeyPair() {
  const bool bNeedReg = IsNeedRegister();
  if (!bNeedReg) {
    return true;
  }
  const QString aesKey = mAesKeyLE->text();
  if (aesKey != mAesKeyConfirmLE->text()) {
    const QString oldStyle = mAesKeyLE->styleSheet();
    const QString oldReEnterStyle = mAesKeyConfirmLE->styleSheet();

    mAesKeyLE->setStyleSheet("QLineEdit { border: 1px solid red; }");
    mAesKeyConfirmLE->setStyleSheet("QLineEdit { border: 1px solid red; }");

    QTimer::singleShot(2000, this, [this, oldStyle, oldReEnterStyle]() {
      mAesKeyLE->setStyleSheet(oldStyle);
      mAesKeyConfirmLE->setStyleSheet(oldReEnterStyle);
    });
    return false;
  }
  return true;
}

bool AccountFileAndAESKey::TryCommitAesKey() {
  if (!ValidateAesKeyPair()) {
    return false;
  }

  emit aesKeyChanged(mAesKeyLE->text());
  return true;
}

bool AccountFileAndAESKey::onSaveCurrentInputKey() {
  const QString aesKey = GetAesKey();
  bool saveResult = CredUtilHelper::savePassword(GetPasswordBookCredTargetName(), aesKey);
  LOG_OE_P(saveResult, "Save Aes key", "%d char to Credential Manager", aesKey.size());
  return saveResult;
}

bool AccountFileAndAESKey::onApply() {
  if (!ValidateAesKeyPair()) {
    return false;
  }
  emit encFileFullPathAndAesKeyChanged(GetAccountFileFullPath(), GetAesKey());
  return true;
}