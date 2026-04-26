#include "ImageTool.h"
#include "StringTool.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "Configuration.h"
#include <QApplication>
#include <QPixmapCache>
#include <QBuffer>

namespace ImageTool {
bool IsFileAbsPathImage(const QString& fileAbsPath) {
  // .jpg, .webp, .avif
  return TYPE_FILTER::isDotExtImage(PathTool::GetDotFileExtension(fileAbsPath));
}

bool IsGifFile(const QString& fileAbsPath) {
  return fileAbsPath.endsWith(".gif", Qt::CaseInsensitive);
}

const QFileIconProvider& GetIconProvider() {
  static const QFileIconProvider iconProv;
  return iconProv;
}

QIcon GetIconFromCachedByFullPath(const QString& fullPath) {
  return GetIconFromCached(PathTool::GetAsteriskDotFileExtension(fullPath));
}

QIcon GetIconFromCached(const QString& starDotExt) {
  static const QFileIconProvider& iconProv = GetIconProvider();
  static QHash<QString, QIcon> starDotExt2Icon{{"", iconProv.icon(QFileIconProvider::IconType::Folder)}};
  auto it = starDotExt2Icon.constFind(starDotExt);
  if (it == starDotExt2Icon.constEnd()) {
    return starDotExt2Icon[starDotExt] = iconProv.icon(starDotExt);
  }
  return it.value();
}

QPixmap GetPixmapFromCached(const QString& fileAbsPath, int expectWidth, int expectHeight, bool bSmooth) {
  QPixmap pm;
  const QString imgKey{StringTool::PathJoinPixmapSize(fileAbsPath, expectWidth, expectHeight, bSmooth)};
  if (QPixmapCache::find(imgKey, &pm)) {
    return pm;
  }
  if (fileAbsPath.startsWith(':') || IsFileAbsPathImage(fileAbsPath)) {
    if (QFile{fileAbsPath}.size() > 10 * 1024 * 1024) { // 10MB
      return {};                                        // image files too large
    }
    if (!pm.load(fileAbsPath)) {
      return {}; // load failed
    }
  } else {
    const QString& starDotExt{PathTool::GetAsteriskDotFileExtension(fileAbsPath)};
    if (!QPixmapCache::find(starDotExt, &pm)) {
      const QIcon& ic = GetIconFromCached(starDotExt);
      pm = ic.pixmap(64, 64);
      QPixmapCache::insert(starDotExt, pm);
    }
  }
  const Qt::TransformationMode transformMode{bSmooth ? Qt::SmoothTransformation : Qt::FastTransformation};
  if (pm.width() * expectHeight >= pm.height() * expectWidth) {
    pm = pm.scaledToWidth(expectWidth, transformMode);
  } else {
    pm = pm.scaledToHeight(expectHeight, transformMode);
  }
  QPixmapCache::insert(imgKey, pm);
  return pm;
}

QString GetBase64PixmapForHtml(const QString& starDotExtensionLowerCase) {
  QString imgStr;
  static QHash<QString, QString> fileTypeImgIcons;
  auto it = fileTypeImgIcons.find(starDotExtensionLowerCase);
  if (it == fileTypeImgIcons.end()) {
    static QFileIconProvider iconProv;
    const QIcon& ic = iconProv.icon(starDotExtensionLowerCase);
    const QPixmap pm{ic.pixmap(64, 64)};
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    pm.save(&buffer, "PNG");
    imgStr = R"(<img src="data:image/png;base64,)" + bArray.toBase64() + QString(R"(" width="64">)");
    fileTypeImgIcons[starDotExtensionLowerCase] = imgStr;
  } else {
    imgStr = it.value();
  }
  return imgStr;
}

QIcon GetBuiltInIcon(QStyle::StandardPixmap spE) {
#ifdef RUNNING_UNIT_TESTS
  return {};
#endif
  static const QStyle* pStyle = QApplication::style();
  return pStyle->standardIcon(spE);
}

const QIcon& GetCheckResultIcon(bool bPass) {
  static const QIcon PASS_OR_NOT_ICONS_ARR[]{QIcon{":img/WRONG"}, QIcon{":img/CORRECT"}};
  return PASS_OR_NOT_ICONS_ARR[(int) bPass];
}

const QPixmap& GetLabelStatusPixmap(int labelStatus) {
  static const QPixmap labelSavedStatusPxp[]                      //
      {QPixmap(":img/SAVED").scaled(24, 24, Qt::KeepAspectRatio), //
       QPixmap(":img/NOT_SAVED").scaled(24, 24, Qt::KeepAspectRatio)};
  const int clampResult = std::max(0, std::min(1, labelStatus));
  return labelSavedStatusPxp[clampResult];
}

} // namespace ImageTool

constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_16;
constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_24;
constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_48;
constexpr QSize IMAGE_SIZE::ICON_SIZE_CANDIDATES[];
constexpr int IMAGE_SIZE::ICON_SIZE_CANDIDATES_N;
constexpr int IMAGE_SIZE::DEFAULT_IMAGE_SCALED_SIZE, IMAGE_SIZE::DEFAULT_NON_IMAGE_SCALED_SIZE;

QString IMAGE_SIZE::HumanReadFriendlySize(int scaleIndex, bool* isValidScaledIndex) {
  if (scaleIndex < 0 || scaleIndex >= IMAGE_SIZE::ICON_SIZE_CANDIDATES_N) {
    if (isValidScaledIndex != nullptr) {
      *isValidScaledIndex = false;
    }
    return QString::asprintf("[%d] out of range[0, %d)", scaleIndex, ICON_SIZE_CANDIDATES_N);
  }
  if (isValidScaledIndex != nullptr) {
    *isValidScaledIndex = true;
  }
  return QString::asprintf("[%d] %d-by-%d",
                           scaleIndex, //
                           ICON_SIZE_CANDIDATES[scaleIndex].width(),
                           ICON_SIZE_CANDIDATES[scaleIndex].height());
}

int IMAGE_SIZE::clampScaledIndex(int newScaledIndex) {
  if (newScaledIndex < 0) {
    return 0;
  }
  if (newScaledIndex >= IMAGE_SIZE::ICON_SIZE_CANDIDATES_N) {
    return IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1;
  }
  return newScaledIndex;
}

int IMAGE_SIZE::GetInitialScaledSize(const QString& name) {
  const bool isImageRelated{
      name.contains("img", Qt::CaseSensitivity::CaseInsensitive)        //
      || name.contains("image", Qt::CaseSensitivity::CaseInsensitive)   //
      || name.contains("scene", Qt::CaseSensitivity::CaseInsensitive)   //
      || name.contains("browser", Qt::CaseSensitivity::CaseInsensitive) //
  }; //
  const int defaultScaledSize{isImageRelated ? DEFAULT_IMAGE_SCALED_SIZE : DEFAULT_NON_IMAGE_SCALED_SIZE};
  int iconSizeIndexHint = Configuration().value(name + "/ICON_SIZE_INDEX", defaultScaledSize).toInt();
  return clampScaledIndex(iconSizeIndexHint); // [0, WHEEL_CANDIDATES_N)
}

void IMAGE_SIZE::SaveInitialScaledSize(const QString& name, int scaledIndex) {
  Configuration().setValue(name + "/ICON_SIZE_INDEX", scaledIndex);
}
