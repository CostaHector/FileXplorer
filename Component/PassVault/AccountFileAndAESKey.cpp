#include "AccountFileAndAESKey.h"
#include "AccountFileHelper.h"
#include "CredentialUtil.h"
#include "NotificatorMacro.h"

#include <QFileInfo>
#include <QTimer>

QString GetCredTargetName() {
#ifdef RUNNING_UNIT_TESTS
  return "PASSWORD_MANAGER_AES_KEY_TEST";
#else
  return "PASSWORD_MANAGER_AES_KEY";
#endif
}

AccountFileAndAESKey::AccountFileAndAESKey(QWidget *parent) : QWidget{parent} {
  mEncFileFullPathLE = new QLineEdit{this};

  mAesKeyLE = new QLineEdit{this};
  mAesKeyLE->setEchoMode(QLineEdit::PasswordEchoOnEdit);
  mAesKeyConfirmLE = new QLineEdit{this};
  mAesKeyConfirmLE->setEchoMode(QLineEdit::PasswordEchoOnEdit);

  mResetInputButton = new QPushButton{tr("Reset Input"), this};
  mResetInputButton->setToolTip("Restore the default file path and clear the AES key fields. Press Enter in QLineEdit to apply.");

  mSaveCurrentAesKeyButton = new QPushButton{tr("Save Key"), this};
  mSaveCurrentAesKeyButton->setToolTip("Save the current AES key text to the credential manager immediately. The input is not validated at all.");

  mLayout = new QFormLayout(this);
  mLayout->addRow(tr("Encrypt File:"), mEncFileFullPathLE);
  mLayout->addRow(tr("AES Key:"), mAesKeyLE);
  mLayout->addRow(tr("Confirm Key:"), mAesKeyConfirmLE);
  mLayout->addRow(mResetInputButton, mSaveCurrentAesKeyButton);
  mLayout->setSpacing(1);
  mLayout->setContentsMargins(0, 0, 0, 0);

  QString encFileFullPath = AccountFileHelper::GetSaveToFullPath(true, "");
  InitAccountFullPath(encFileFullPath);

  const QString aesKey = CredUtilHelper::readPassword(GetCredTargetName());
  if (!aesKey.isEmpty()) {
    InitAesKey(aesKey);
  }

  subscribe();
}

bool AccountFileAndAESKey::IsNeedRegister() const {
  const QFileInfo fi{GetAccountFileFullPath()};
  return !fi.exists() || fi.size() == 0;
}

void AccountFileAndAESKey::SetReEnterVisible(bool visible) {
  mAesKeyConfirmLE->setVisible(visible);
  if (QWidget* label = mLayout->labelForField(mAesKeyConfirmLE)) {
    label->setVisible(visible);
  }
}

void AccountFileAndAESKey::subscribe() {
  connect(mEncFileFullPathLE, &QLineEdit::returnPressed, this, &AccountFileAndAESKey::onEncFileFullPathLineEditChanged);
  connect(mAesKeyLE, &QLineEdit::returnPressed, this, &AccountFileAndAESKey::TryCommitAesKey);
  connect(mAesKeyConfirmLE, &QLineEdit::returnPressed, this, &AccountFileAndAESKey::TryCommitAesKey);
  connect(mResetInputButton, &QPushButton::clicked, this, &AccountFileAndAESKey::onResetInput);
  connect(mSaveCurrentAesKeyButton, &QPushButton::clicked, this, &AccountFileAndAESKey::onSaveCurrentInputKey);
}

void AccountFileAndAESKey::onEncFileFullPathLineEditChanged() {
  const bool bNeedReg{IsNeedRegister()};
  SetReEnterVisible(bNeedReg);

  if (mEncFileFullPath == mEncFileFullPathLE->text()) {
    return;
  }
  mEncFileFullPath = mEncFileFullPathLE->text();
  emit encFileFullPathChanged(mEncFileFullPath);
}

bool AccountFileAndAESKey::TryCommitAesKey() {
  const QString current = mAesKeyLE->text();
  const bool bNeedReg = IsNeedRegister();

  if (bNeedReg) {
    if (current != mAesKeyConfirmLE->text()) {
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
  }

  if (current == mAESKey) {
    return true;            // 值没变，不重复 emit
  }
  mAESKey = std::move(current);
  emit aesKeyChanged(mAESKey);
  return true;
}

void AccountFileAndAESKey::onResetInput() {
  QString encFileFullPath = AccountFileHelper::GetSaveToFullPath(true, "");
  InitAccountFullPath(encFileFullPath);
  InitAesKey("");
}

bool AccountFileAndAESKey::onSaveCurrentInputKey() {
  bool saveResult = CredUtilHelper::savePassword(GetCredTargetName(), mAesKeyLE->text());
  LOG_OE_P(saveResult, "Save Aes key", "%d char to Credential Manager", mAesKeyLE->text().size());
  return saveResult;
}