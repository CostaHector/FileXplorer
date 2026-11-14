#ifndef ACCOUNTFILEANDAESKEY_H
#define ACCOUNTFILEANDAESKEY_H

#include <QWidget>
#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>

QString GetPasswordBookCredTargetName();
class AccountFileAndAESKey: public QWidget {
  Q_OBJECT
public:
  explicit AccountFileAndAESKey(const QString& specifyEncFileFullPath, const QString& specifyAesKey, QWidget* parent = nullptr);

  // 外部初始化
  void InitAccountFullPath(const QString& newAccountFullPath) {
    mEncFileFullPathLE->setText(newAccountFullPath);
    const bool bNeedReg{IsNeedRegister()};
    SetReEnterVisible(bNeedReg);
  }
  void InitAesKey(const QString& aesKey) {
    mAesKeyLE->setText(aesKey);
    mAesKeyConfirmLE->setText(aesKey);
  }

  QString GetAccountFileFullPath() const { return mEncFileFullPathLE->text(); }
  QString GetAesKey() const { return mAesKeyLE->text(); }

signals:
  void encFileFullPathChanged(QString encFullpath);
  void aesKeyChanged(QString aesKey);
  void encFileFullPathAndAesKeyChanged(QString encFullpath, QString aesKey);

private:
  bool ValidateAesKeyPair();
  static bool IsNeedRegisterCore(const QString& path);
  bool IsNeedRegister() const;
  void SetReEnterVisible(bool visible);
  void subscribe();

  void onEncFileFullPathLineEditChanged();
  bool TryCommitAesKey();
  bool onApply();
  bool onSaveCurrentInputKey();

  QLineEdit* mEncFileFullPathLE{nullptr};
  QLineEdit* mAesKeyLE{nullptr};
  QLineEdit* mAesKeyConfirmLE{nullptr};

  QPushButton* mApplyButton{nullptr};
  QPushButton* mSaveCurrentAesKeyButton{nullptr};
  QFormLayout* mLayout{nullptr};
};
#endif // ACCOUNTFILEANDAESKEY_H
