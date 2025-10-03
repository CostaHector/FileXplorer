#include "ImageTestPrecoditionTools.h"
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

QByteArray GetPNGImage(int width, int height, const QString& imgType) {
  QByteArray imageData;
  QImage img(width, height, QImage::Format_RGB32);
  img.fill(Qt::green);
  QBuffer buffer(&imageData);
  buffer.open(QIODevice::WriteOnly);
  img.save(&buffer, imgType.toUtf8().constData());
  return imageData;
}
}  // namespace ImageTestPrecoditionTools
