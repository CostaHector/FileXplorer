#ifndef INPUTDIALOGHELPER_H
#define INPUTDIALOGHELPER_H

#include <QFont>

namespace InputDialogHelper {
std::pair<bool, QFont> GetFontWithInitial(const QFont &initialFont, QWidget *parent = nullptr, const QString &title = "");
std::pair<bool, int> GetIntWithInitial(QWidget *parent, const QString &title, const QString &label, int value, int minValue, int maxValue, int step);
} // namespace InputDialogHelper

#endif // INPUTDIALOGHELPER_H
