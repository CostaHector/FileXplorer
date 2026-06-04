#ifndef INPUTDIALOGHELPER_H
#define INPUTDIALOGHELPER_H

#include <QFont>
#include <QMessageBox>

namespace InputDialogHelper {
std::pair<bool, QFont> GetFontWithInitial(const QFont &initialFont, QWidget *parent = nullptr, const QString &title = "");
std::pair<bool, int> GetIntWithInitial(QWidget *parent, const QString &title, const QString &label, int value, int minValue, int maxValue, int step);

bool YesOrCancelBox(QMessageBox::Icon iconType, const QIcon& winIcon, const QString& title, const QString& text, const QString& informativeText);
} // namespace InputDialogHelper

#endif // INPUTDIALOGHELPER_H
