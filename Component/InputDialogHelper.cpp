#include "InputDialogHelper.h"
#include <QFontDialog>
#include <QInputDialog>
#include <QAbstractButton>

namespace InputDialogHelper {

std::pair<bool, QFont> GetFontWithInitial(const QFont& initialFont, QWidget* parent, const QString& title) {
  bool bAccept{false};
  const QFont& newFont = QFontDialog::getFont(&bAccept, initialFont, parent, title);
  return {bAccept, newFont};
}
std::pair<bool, int> GetIntWithInitial(QWidget *parent, const QString &title, const QString &label, int initialValue,
                                       int minValue, int maxValue,
                                       int step) {
  bool bAccept{false};
  int newValue = QInputDialog::getInt(parent, title, label, initialValue, minValue, maxValue, step, &bAccept);
  return {bAccept, newValue};
}

bool YesOrCancelBox(QMessageBox::Icon iconType, const QIcon& winIcon,
                    const QString& title, const QString& text,
                    const QString& informativeText, const QString& detailedText) {
  QMessageBox msgBox;
  msgBox.setIcon(iconType);
  msgBox.setWindowIcon(winIcon);
  msgBox.setWindowTitle(title);
  msgBox.setText(text);
  msgBox.setInformativeText(informativeText);
  msgBox.setDetailedText(detailedText);
  msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  return msgBox.exec() == QMessageBox::Yes;
}

int YesNoCancelBox(QMessageBox::Icon iconType, const QIcon& winIcon,
                   const QString& title, const QString& text,
                   const QString& informativeText, const QString& detailedText,
                   const QString& yesText, const QString& noText) {
  QMessageBox msgBox;
  msgBox.setIcon(iconType);
  msgBox.setWindowIcon(winIcon);
  msgBox.setWindowTitle(title);
  msgBox.setText(text);
  msgBox.setInformativeText(informativeText);
  msgBox.setDetailedText(detailedText);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

  // 回车触发 No，避免误覆盖
  msgBox.setDefaultButton(QMessageBox::No);
  // Esc 和标题栏 X 都等价于点击 Cancel
  msgBox.setEscapeButton(QMessageBox::Cancel);

  // 自定义 Yes 按钮文字（可选）
  if (!yesText.isEmpty()) {
    if (QAbstractButton* yesBtn = msgBox.button(QMessageBox::Yes)) {
      yesBtn->setText(yesText);
    }
  }
  if (!noText.isEmpty()) {
    if (QAbstractButton* noBtn = msgBox.button(QMessageBox::Yes)) {
      noBtn->setText(noText);
    }
  }

  const int ret = msgBox.exec();
  // 兜底：平台差异下 X 关闭可能返回 QDialog::Rejected / NoButton，统一归为 Cancel
  return (ret == QMessageBox::Yes || ret == QMessageBox::No || ret == QMessageBox::Cancel) ? ret : QMessageBox::Cancel;
}

}
