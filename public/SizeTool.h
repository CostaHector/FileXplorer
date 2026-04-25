#ifndef SIZETOOL_H
#define SIZETOOL_H

#include <QRect>
#include <QSize>

namespace SizeTool {
constexpr int TABLE_DEFAULT_ROW_SECTION_SIZE = 30;
constexpr int TABLE_DEFAULT_COLUMN_SECTION_SIZE = 200;
constexpr int TABLE_MAX_ROW_SECTION_SIZE = 9999;
constexpr int TABLE_MAX_COLUMN_SECTION_SIZE = 9999;
constexpr QRect DEFAULT_GEOMETRY{100, 100, 1024, 768};
constexpr QSize DOCKER_DEFAULT_SIZE{SizeTool::DEFAULT_GEOMETRY.width() / 2, SizeTool::DEFAULT_GEOMETRY.height()};
}  // namespace SizeTool

#endif  // SIZETOOL_H
