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
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QTableView {
    show-decoration-selected: 1;
    alternate-background-color: %1;
    gridline-color: %2;
}
QTableView::item {
    border: none;
}
QTableView::item:alternate {
}
QTableView::item:selected {
    border-bottom: 1px inherit %3;
}
QTableView::item:selected:active {
    background: %4;
    border-bottom: 2px solid %3;
}
QTableView::item:selected:!active {
    background: %5;
}
QTableView::item:focus {
    border: none;
}
QTableView::item:hover {
    background: %6;
}
QTableView {
    background-image: url("%7");
    background-position: center top;
    background-repeat: no-repeat;
    background-attachment: fixed;
}
        )";
      break;
    default:
      return "";
  }
  const auto& inst = GetInst();
  QString bgAlternateRow{inst.GetColorValue("Background/View/AlternateRow", styleE)};
  QString gridline{inst.GetColorValue("GridLine", styleE)};
  QString borderGeneral{inst.GetColorValue("Border/General", styleE)};
  QString bgSelectedActive{inst.GetColorValue("Background/View/SelectedActive", styleE)};
  QString bgSelectedInactive{inst.GetColorValue("Background/View/SelectedInactive", styleE)};
  QString bgHover{inst.GetColorValue("Background/View/Hover", styleE)};
  QString bgImage{inst.GetNoColorValue("BackgroundImage/TableView")};

  QString styleSheetTableCornerButton{R"(
QTableView > QTableCornerButton::section {
    background-color: %1;
}
)"};
  QString tableCornerButtonSection{inst.GetColorValue("Background/View/TableCornerButton/Section", styleE)};
  return styleSheet.arg(bgAlternateRow, gridline, borderGeneral, bgSelectedActive, bgSelectedInactive, bgHover, bgImage) //
         + styleSheetTableCornerButton.arg(tableCornerButtonSection);
}

QString ListViewStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;

  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QListView {
    show-decoration-selected: 1;
    alternate-background-color: %1;
}
QListView:icon-mode::item {
    border: none;
}
QListView:list-mode::item:selected {
    border-bottom: 1px inherit %2;
}
QListView:list-mode::item:selected:active {
    background: %3;
    border-bottom: 2px solid %3;
}
QListView::item:selected:!active {
    background: %4;
}
QListView::item:hover {
    background: %5;
}
        )";
      break;
    default:
      return "";
  }
  const auto& inst = GetInst();
  QString bgAlternateRow{inst.GetColorValue("Background/View/AlternateRow", styleE)};         // %1
  QString borderGeneral{inst.GetColorValue("Border/General", styleE)};                        // %2
  QString bgSelectedActive{inst.GetColorValue("Background/View/SelectedActive", styleE)};     // %3
  QString bgSelectedInactive{inst.GetColorValue("Background/View/SelectedInactive", styleE)}; // %4
  QString bgHover{inst.GetColorValue("Background/View/Hover", styleE)};                       // %5
  styleSheet = styleSheet.arg(bgAlternateRow, borderGeneral, bgSelectedActive, bgSelectedInactive, bgHover);
  return styleSheet;
}

QString TreeViewStyleSheet::GetStyleSheet(Style::StyleSheetE styleE) const {
  QString styleSheet;
  switch (styleE) {
    case Style::StyleSheetE::STYLESHEET_LIGHT:
    case Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      styleSheet = R"(
QTreeView {
    show-decoration-selected: 0;
    alternate-background-color: %1;
}
QTreeView::item:alternate {
}
QTreeView::item:selected {
    border-bottom: 1px inherit %2;
}
QTreeView::item:selected:active {
    background: %3;
    border-bottom: 2px solid %2;
}
QTreeView::item:selected:!active {
    background: %4;
}
QTreeView::item:hover {
    background: %5;
}
        )";
      break;
    default:
      return "";
  }
  const auto& inst = GetInst();
  QString bgAlternateRow{inst.GetColorValue("Background/View/AlternateRow", styleE)}; // %1
  QString borderGeneral{inst.GetColorValue("Border/General", styleE)};                // %2

  QString bgSelectedActive{inst.GetColorValue("Background/View/SelectedActive", styleE)};     // %3
  QString bgSelectedInactive{inst.GetColorValue("Background/View/SelectedInactive", styleE)}; // %4
  QString bgHover{inst.GetColorValue("Background/View/Hover", styleE)};                       // %5

  styleSheet = styleSheet.arg(bgAlternateRow, borderGeneral, bgSelectedActive, bgSelectedInactive, bgHover);
  return styleSheet;
}
