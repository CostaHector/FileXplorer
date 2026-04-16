#include "ViewStyleSheet.h"
#include "Logger.h"
#include <QColor>

bool TableViewStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new TableViewStyleSheet));
bool ListViewStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new ListViewStyleSheet));
bool TreeViewStyleSheet::mRegistered = StyleSheetGetter::Register(std::unique_ptr<StyleSheetGetter>(new TreeViewStyleSheet));

QString TableViewStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
      styleSheet = R"(
QTableView QTableCornerButton::section {
    border-right: none;
    border-bottom: none;
}
QTableView QTableCornerButton::section:hover {
}
QTableView QTableCornerButton::section:pressed {
}

QTableView {
    show-decoration-selected: 1;
    %1
    background-color: %2;
    alternate-background-color: %3;
    gridline-color: %8;
}
QTableView::item {
    border-left: 2px solid transparent; /* preserve 2 px for focus */
    border-right: 2px solid transparent; /* preserve 2 px for focus */
}
QTableView::item:alternate {
}
QTableView::item:selected {
    border-bottom: 1px inherit %4;
}
QTableView::item:selected:active {
    background: %5;
    color: #000000;
    border-bottom: 2px solid %4;
}
QTableView::item:selected:!active {
    background: %6;
    color: #000000;
}
QTableView::item:focus {
    border-left: 2px solid %4;
    border-right: 2px solid %4;
}
QTableView::item:hover {
    background: %7;
    color: #000000;
}

        )";
      break;
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QTableView QTableCornerButton::section {
    background-color: #3F3B39;
    border-right: none;
    border-bottom: none;
}

QTableView QTableCornerButton::section:hover {
    background-color: #252525;
}

QTableView QTableCornerButton::section:pressed {
    background-color: #252525;
}

QTableView {
    show-decoration-selected: 1;
    %1
    background-color: %2;
    alternate-background-color: %3;
    gridline-color: %8;
}
QTableView::item {
    border-left: 2px solid transparent; /* preserve 2 px for focus */
    border-right: 2px solid transparent; /* preserve 2 px for focus */
}
QTableView::item:alternate {
}
QTableView::item:selected {
    border-bottom: 1px inherit %4;
}
QTableView::item:selected:active {
    background: %5;
    color: #000000;
    border-bottom: 2px solid %4;
}
QTableView::item:selected:!active {
    background: %6;
    color: #000000;
}
QTableView::item:focus {
    border-left: 2px solid %4;
    border-right: 2px solid %4;
}
QTableView::item:hover {
    background: %7;
    color: #FFFFFF;
}
        )";
      break;
    default:
      return "";
  }

  QString fontStr{FontCfg::ReadFontString()}; // %1
  QString bgGeneral{ColorCfg::GetColorBackgroundGeneral(styleE)};// %2
  QString bgAlternateRow{ColorCfg::GetColorBackgroundAlternateRow(styleE)}; // %3
  QString borderGeneral{ColorCfg::GetColorBorderGeneral(styleE)}; // %4 
  QString bgSelectedActive{ColorCfg::GetColorBackgroundSelectedActive(styleE)}; // %5
  QString bgSelectedInactive{ColorCfg::GetColorBackgroundSelectedInactive(styleE)}; // %6
  QString bgHover{ColorCfg::GetColorBackgroundHover(styleE)}; // %7
  QString gridline{ColorCfg::GetColorGridLine(styleE)}; // %8
  QString forgroundGeneral{ColorCfg::GetColorForgroundGeneral(styleE)};

  styleSheet = styleSheet.arg(fontStr, bgGeneral, bgAlternateRow, borderGeneral, bgSelectedActive, bgSelectedInactive, bgHover, gridline);
  return styleSheet;
}

QString ListViewStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;

  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
      styleSheet = R"(
QListView {
    show-decoration-selected: 1;
    %1
    background-color: %2;
    alternate-background-color: %3;
}
QListView::item:alternate {
}
QListView:!icon-mode::item:selected {
    border-bottom: 1px inherit %4;
}
QListView:!icon-mode::item:selected:active {
    background: %5;
    color: #000000;
    border-top: 2px solid %4;
    border-bottom: 2px solid %4;
}
QListView:!icon-mode::item:selected:!active {
    background: %6;
    color: #000000;
}
QListView::item:hover {
    background: %7;
    color: #000000;
}
        )";
      break;
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QListView {
    show-decoration-selected: 1;
    %1
    background-color: %2;
    alternate-background-color: %3;
}
QListView::item:alternate {
}
QListView:!icon-mode::item:selected {
    border-bottom: 1px inherit #FFFFFF;
}
QListView:!icon-mode::item:selected:active {
    background: %5;
    color: #000000;
    border-top: 2px solid %4;
    border-bottom: 2px solid %4;
}
QListView:!icon-mode::item:selected:!active {
    background: %6;
    color: #000000;
}
QListView::item:hover {
    background: %7;
    color: #FFFFFF;
}
        )";
      break;
    default:
      return "";
  }

  QString fontStr{FontCfg::ReadFontString()}; // %1
  QString bgGeneral{ColorCfg::GetColorBackgroundGeneral(styleE)};// %2
  QString bgAlternateRow{ColorCfg::GetColorBackgroundAlternateRow(styleE)}; // %3
  QString borderGeneral{ColorCfg::GetColorBorderGeneral(styleE)}; // %4
  QString bgSelectedActive{ColorCfg::GetColorBackgroundSelectedActive(styleE)}; // %5
  QString bgSelectedInactive{ColorCfg::GetColorBackgroundSelectedInactive(styleE)};// %6
  QString bgHover{ColorCfg::GetColorBackgroundHover(styleE)};// %7
  
  QString gridline{ColorCfg::GetColorGridLine(styleE)};
  QString forgroundGeneral{ColorCfg::GetColorForgroundGeneral(styleE)};
  styleSheet = styleSheet.arg(fontStr, bgGeneral, bgAlternateRow, borderGeneral, bgSelectedActive, bgSelectedInactive, bgHover);
  return styleSheet;
}

QString TreeViewStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
      styleSheet = R"(
QTreeView {
    show-decoration-selected: 0;
    %1
    background-color: %2;
    alternate-background-color: %3;
}
QTreeView::item:alternate {
}
QTreeView::item:selected {
    border-bottom: 1px inherit %4;
}
QTreeView::item:selected:active {
    background: %5;
    color: #000000;
    border-top: 2px solid %4;
    border-bottom: 2px solid %4;
}
QTreeView::item:selected:!active {
    background: %6;
    color: #000000;
}
QTreeView::item:hover {
    background: %7;
    color: #000000;
}
        )";
      break;
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QTreeView {
    show-decoration-selected: 0;
    %1
    background-color: %2;
    alternate-background-color: %3;
}
QTreeView::item:alternate {
}
QTreeView::item:selected {
    border-bottom: 1px inherit %4;
}
QTreeView::item:selected:active {
    background: %5;
    color: #000000;
    border-top: 2px solid %4;
    border-bottom: 2px solid %4;
}
QTreeView::item:selected:!active {
    background: %6;
    color: #000000;
}
QTreeView::item:hover {
    background: %7;
    color: #FFFFFF;
}
        )";
      break;
    default:
      return "";
  }
  QString fontStr{FontCfg::ReadFontString()}; // %1
  QString bgGeneral{ColorCfg::GetColorBackgroundGeneral(styleE)};// %2
  QString bgAlternateRow{ColorCfg::GetColorBackgroundAlternateRow(styleE)}; // %3
  QString borderGeneral{ColorCfg::GetColorBorderGeneral(styleE)}; // %4
  
  QString bgSelectedActive{ColorCfg::GetColorBackgroundSelectedActive(styleE)}; // %5
  QString bgSelectedInactive{ColorCfg::GetColorBackgroundSelectedInactive(styleE)}; // %6
  QString bgHover{ColorCfg::GetColorBackgroundHover(styleE)}; // %7
  
  QString gridline{ColorCfg::GetColorGridLine(styleE)};
  QString forgroundGeneral{ColorCfg::GetColorForgroundGeneral(styleE)};
  styleSheet = styleSheet.arg(fontStr, bgGeneral, bgAlternateRow, borderGeneral, bgSelectedActive, bgSelectedInactive, bgHover);
  return styleSheet;
}
