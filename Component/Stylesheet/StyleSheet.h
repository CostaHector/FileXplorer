#ifndef STYLESHEET_H
#define STYLESHEET_H
#include <QString>
#include <QRect>
#include <QSize>
#include <QFont>

class QWidget;
class QLayout;

namespace StyleSheet {
void UpdateTitleBar(QWidget* widget);
void setDarkTitleBar(QWidget* widget, bool enable);
void setGlobalDarkMode(bool enable);

QIcon GetSystemDirOpenIcon();
}  // namespace StyleSheet

constexpr QRect DEFAULT_GEOMETRY{100, 100, 1024, 768};
constexpr QSize DOCKER_DEFAULT_SIZE{DEFAULT_GEOMETRY.width() / 2, DEFAULT_GEOMETRY.height()};

void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align);
#endif  // STYLESHEET_H
