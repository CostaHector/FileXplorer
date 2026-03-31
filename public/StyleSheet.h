#ifndef STYLESHEET_H
#define STYLESHEET_H
#include <QString>
#include <QRect>
#include <QSize>
#include <QFont>

class QWidget;
class QLayout;

namespace StyleSheet {
constexpr int ROW_SECTION_HEIGHT = 10;
static const QFont TEXT_EDIT_FONT{"Consolas", 15};

void UpdateTitleBar(QWidget* widget);
void setDarkTitleBar(QWidget* widget, bool enable);
void setGlobalDarkMode(bool enable);

constexpr char SUBMIT_BTN_STYLE[] =
    "QPushButton{"
    "    color: #fff;"
    "    background-color: DodgerBlue;"
    "    border-color: DodgerBlue;"
    "}"
    "QPushButton:hover {"
    "    color: #fff;"
    "    background-color: rgb(36, 118, 199);"
    "    border-color: rgb(36, 118, 199);"
    "}";
}  // namespace StyleSheet

constexpr QRect DEFAULT_GEOMETRY{100, 100, 1024, 768};
constexpr QSize DOCKER_DEFAULT_SIZE{DEFAULT_GEOMETRY.width() / 2, DEFAULT_GEOMETRY.height()};

void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align);
#endif  // STYLESHEET_H
