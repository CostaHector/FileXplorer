#ifndef IMAGETESTPRECODITIONTOOLS_H
#define IMAGETESTPRECODITIONTOOLS_H
#include <QStringList>

namespace ImageTestPrecoditionTools {
constexpr const char* SVG_FILL_COLORS[]{
    "#FF0000",  // 红色
    "#00FF00",  // 绿色
    "#0000FF",  // 蓝色
    "#FFFF00",  // 黄色
    "#FF00FF",  // 品红
    "#00FFFF",  // 青色
    "#FFA500",  // 橙色
    "#800080",  // 紫色
    "#008000",  // 深绿
    "#000080",  // 深蓝
    "#FFC0CB",  // 粉色
    "#A52A2A",  // 棕色
    "#808080",  // 灰色
    "#000000",  // 黑色
    "#FFFFFF",  // 白色
    "#40E0D0"   // 绿松石
};

inline const QString& GetSvgContentTemplate() {
  static const QString svg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="540" height="360" viewBox="0 0 540 360">
  <rect width="540" height="360" fill="%1"/>
  <text x="0" y="300" font-size="360" fill="white">%2</text>
  </svg>)";  // WARNING: don't use R"" inside class when the class need .moc
  return svg;
}
}
#endif  // IMAGETESTPRECODITIONTOOLS_H
