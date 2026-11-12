#ifndef ACCOUNTFILEANDAESKEY_H
#define ACCOUNTFILEANDAESKEY_H

#include <QWidget>
#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>

class AccountFileAndAESKey: public QWidget {
  Q_OBJECT
public:
  explicit AccountFileAndAESKey(QWidget* parent = nullptr);

  // 外部初始化
  void InitAccountFullPath(const QString& newAccountFullPath) {
    mEncFileFullPathLE->setText(newAccountFullPath);
    const bool bNeedReg{IsNeedRegister()};
    SetReEnterVisible(bNeedReg);
  }
  void InitAesKey(const QString& aesKey) {
    mAesKeyLE->setText(aesKey);
    mAesKeyConfirmLE->setText(aesKey);
    mAESKey = aesKey;
  }

  QString GetAccountFileFullPath() const { return mEncFileFullPathLE->text(); }

  const QString& GetAesKey() const { return mAESKey; }

signals:
  void encFileFullPathChanged(QString);
  void aesKeyChanged(QString);

private:
  bool IsNeedRegister() const;
  void SetReEnterVisible(bool visible);
  void subscribe();

  void onEncFileFullPathLineEditChanged();
  bool TryCommitAesKey();
  void onResetInput();
  bool onSaveCurrentInputKey();

  QString mAESKey;
  QString mEncFileFullPath;

  QLineEdit* mEncFileFullPathLE{nullptr};
  QLineEdit* mAesKeyLE{nullptr};
  QLineEdit* mAesKeyConfirmLE{nullptr};

  QPushButton* mResetInputButton{nullptr};
  QPushButton* mSaveCurrentAesKeyButton{nullptr};
  QFormLayout* mLayout{nullptr};
};
#endif // ACCOUNTFILEANDAESKEY_H
