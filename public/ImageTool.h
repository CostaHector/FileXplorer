#ifndef IMAGETOOL_H
#define IMAGETOOL_H

#include <QFileIconProvider>

namespace ImageTool {
bool IsFileAbsPathImage(const QString& fileAbsPath);
bool IsGifFile(const QString& fileAbsPath);


const QFileIconProvider& GetIconProvider();

QIcon GetIconFromCachedByFullPath(const QString& fullPath);
QIcon GetIconFromCached(const QString& starDotExt);

QPixmap GetPixmapFromCached(const QString& fileAbsPath, int expectWidth, int expectHeight, bool bSmooth=false);
QString GetBase64PixmapForHtml(const QString& starDotExtensionLowerCase);
}

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
          QSize(3050, 1885),
          QSize(4935, 3050),
          QSize(7985, 4935),
          QSize(12920, 7985),
      };
  static constexpr int DEFAULT_IMAGE_SCALED_SIZE = 5, DEFAULT_NON_IMAGE_SCALED_SIZE = 0;
  static constexpr int ICON_SIZE_CANDIDATES_N = sizeof(ICON_SIZE_CANDIDATES) / sizeof(*ICON_SIZE_CANDIDATES);
  static QString HumanReadFriendlySize(int scaleIndex, bool* isValidScaledIndex=nullptr);
  static int clampScaledIndex(int newScaledIndex);
  static int GetInitialScaledSize(const QString& name);
  static void SaveInitialScaledSize(const QString& name, int scaledIndex);
};

#endif  // IMAGETOOL_H
