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

constexpr QRect DEFAULT_GEOMETRY{0, 0, 1024, 768};
constexpr QSize DOCKER_DEFAULT_SIZE{DEFAULT_GEOMETRY.width() / 2, DEFAULT_GEOMETRY.height()};

struct IMAGE_SIZE {
  static constexpr int TABS_ICON_IN_MENU_16 = 16;
  static constexpr int TABS_ICON_IN_MENU_24 = 24;
  static constexpr int TABS_ICON_IN_MENU_48 = 48;
  static constexpr QSize ICON_SIZE_CANDIDATES[]//
      {
          QSize(25, 16),
          QSize(40, 25),
          QSize(65, 40),
          QSize(105, 65),
          QSize(170, 105),
          QSize(275, 170),
          QSize(323, 200),
          QSize(445, 275),
          QSize(485, 300),
          QSize(566, 350),
          QSize(648, 400),
          QSize(720, 445),
          QSize(809, 500),
          QSize(970, 600),
          QSize(1165, 720),
          QSize(1885, 1165),
      };
  static constexpr int ICON_SIZE_CANDIDATES_N = sizeof(ICON_SIZE_CANDIDATES) / sizeof(*ICON_SIZE_CANDIDATES);
  static QString HumanReadFriendlySize(int scaleIndex, bool* isValidScaledIndex=nullptr);
};
void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align);
#endif  // STYLESHEET_H
