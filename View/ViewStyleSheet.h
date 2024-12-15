#ifndef VIEWSTYLESHEET_H
#define VIEWSTYLESHEET_H

#include <QString>

namespace ViewStyleSheet {
constexpr int ROW_SECTION_HEIGHT = 10;
QString GetDefaultListViewStyleSheet();
QString GetDefaultTableViewStyleSheet();
QString GetDefaultTreeViewStyleSheet();
}  // namespace ViewStyleSheet

#endif  // VIEWSTYLESHEET_H
