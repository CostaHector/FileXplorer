#include "ImageTestPrecoditionTools.h"
#include "Logger.h"
#include <QBuffer>
#include <QImage>

namespace ImageTestPrecoditionTools {
const QString& GetSvgContentTemplate() {
  static const QString svg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="540" height="360" viewBox="0 0 540 360">
  <rect width="540" height="360" fill="%1"/>
  <text x="0" y="300" font-size="360" fill="white">%2</text>
  </svg>)";  // WARNING: don't use R"" inside class when the class need .moc
  return svg;
}

bool CreateAndSaveAWhitePng(const QString& filePath, int width, int height) {
  QImage image{width, height, QImage::Format_ARGB32};  // ARGB32
  image.fill(Qt::white);                               // RGBA：255,255,255,255
  if (!image.save(filePath)) {
    LOG_W("Failed to save image to file[%s]", qPrintable(filePath));
    return false;
  }
  return true;
}

QByteArray GetPNGImage(int width, int height, const QString& imgType) {
  QByteArray imageData;
  QImage img(width, height, QImage::Format_RGB32);
  img.fill(Qt::green);
  QBuffer buffer(&imageData);
  buffer.open(QIODevice::WriteOnly);
  img.save(&buffer, imgType.toUtf8().constData());
  return imageData;
}

QByteArray GetTestGif() {
  // 预制: 简单的 2x2 像素 GIF 动画的 Base64 编码数据, 包含两帧：红色和蓝色交替
  static const char* simpleGifBase64 =
      "R0lGODlhAgACAPABAAAAAP///yH5BAAAAAAALAAAAAACAAIAAAICRA5WKQUAOw==";
  QByteArray gifData = QByteArray::fromBase64(simpleGifBase64);
  return gifData;
}

quint32 GetPixelColorFromImage(const QImage& image, int x, int y) {
  if (image.isNull()) {
    LOG_W("image is null");
    return -1;
  }
  if (x < 0 || x >= image.width() || y < 0 || y >= image.height()) {
    LOG_W("pnt(%d, %d) out of range", x, y);
    return -1;
  }
  QColor color = image.pixelColor(x, y);

  // 转换为0xRRGGBB格式
  return (color.red() << 16) | (color.green() << 8) | color.blue();
}

quint32 GetPixelColorFromData(const QByteArray& imageData, const char* format, int x, int y) {
  QImage image;
  if (!image.loadFromData(imageData, format)) {
    return -1;
  }
  return GetPixelColorFromImage(image, x, y);
}

quint32 GetPixelColorFromImagePath(const QString& filePath, int x, int y) {
  QImage image(filePath);
  return GetPixelColorFromImage(image, x, y);
}

}  // namespace ImageTestPrecoditionTools
