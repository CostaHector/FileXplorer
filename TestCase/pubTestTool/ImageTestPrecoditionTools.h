#ifndef IMAGETESTPRECODITIONTOOLS_H
#define IMAGETESTPRECODITIONTOOLS_H
#include <QString>
#include <QByteArray>
#include <QImage>

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
constexpr int SVG_FILL_COLORS_COUNT = sizeof(SVG_FILL_COLORS)/sizeof(*SVG_FILL_COLORS);

const QString& GetSvgContentTemplate();
bool CreateAndSaveAWhitePng(const QString& filePath, int width = 1440, int height = 1080);

QByteArray GetPNGImage(int width = 100, int height = 80, const QString& imgType = "PNG");

quint32 GetPixelColorFromImage(const QImage& image, int x, int y);
quint32 GetPixelColorFromData(const QByteArray& imageData, const char* format, int x, int y);
quint32 GetPixelColorFromImagePath(const QString& filePath, int x, int y);

}
#endif  // IMAGETESTPRECODITIONTOOLS_H
