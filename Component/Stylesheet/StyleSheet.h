#ifndef STYLESHEET_H
#define STYLESHEET_H
#include <QString>
#include <QIcon>
#include <QStyle>

class QWidget;
class QLayout;

namespace StyleSheet {
void UpdateTitleBar(QWidget* widget);
void setDarkTitleBar(QWidget* widget, bool enable);
void setGlobalDarkMode(bool enable);
void UpdateApplyPushButton(QWidget* pushButton);
}  // namespace StyleSheet

void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align);
#endif  // STYLESHEET_H
