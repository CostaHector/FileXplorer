#ifndef IMAGETOOL_H
#define IMAGETOOL_H

#include <QFileIconProvider>
#include <QStyle>
class QBuffer;

namespace ImageTool {
bool IsFileAbsPathImage(const QString& fileAbsPath);
bool IsGifFile(const QString& fileAbsPath);

const QFileIconProvider& GetIconProvider();

QIcon GetIconFromCachedByFullPath(const QString& fullPath);
QIcon GetIconFromCached(const QString& starDotExt);

QPixmap GetPixmapFromCached(const QString& fileAbsPath, int expectWidth, int expectHeight, bool bSmooth = false);
QString GetBase64PixmapForHtml(const QString& starDotExtensionLowerCase);

QIcon GetBuiltInIcon(QStyle::StandardPixmap spE);

const QIcon& GetCheckResultIcon(bool bPass);

QSize GetImageDimensionPixel(const QString& imgFilePath);
QSize GetImageDimensionPixel(QBuffer* pBuff, const QString& noDotFormat);

int CreateThumbnailForAllDirectFoldersUnder(const QString& folderPath, bool bSkipIfExist = true);
bool CreateThumbnailCore(const QString& srcImgAbsPath, const QString& dstThumbnailAbsPath, bool bSkipIfExist = true);
constexpr int EXPECT_THUMBNAIL_SIDE = 256;

constexpr int DEFAULT_START_POSITION_SECOND = 5, DEFAULT_INTERVAL_SECOND = 5, DEFAULT_FRAMES_COUNT = 1;
int GrabFramesFromVideos(const QStringList& videosAbsPath, int startPositionMS, int intervalSecond, int framesCount, bool bSkipIfExist = true);
} // namespace ImageTool

struct IMAGE_SIZE {
  static constexpr int TABS_ICON_IN_MENU_16 = 16;
  static constexpr int TABS_ICON_IN_MENU_24 = 24;
  static constexpr int TABS_ICON_IN_MENU_36 = 36;
  static constexpr int TABS_ICON_IN_MENU_48 = 48;
  static constexpr QSize ICON_SIZE_CANDIDATES[] { //
      // width ascending; same width sorted by height ascending
      QSize(25, 16),     QSize(40, 25),     QSize(65, 40),     QSize(105, 65),    QSize(170, 105),//
      QSize(275, 170),   QSize(320, 240),   QSize(323, 200),   QSize(445, 275),   QSize(485, 300),//
      QSize(566, 350),   QSize(640, 480),   QSize(648, 400),   QSize(720, 445),   QSize(800, 600),//
      QSize(809, 500),   QSize(970, 600),   QSize(1024, 768),  QSize(1165, 720),  QSize(1280, 720),//
      QSize(1885, 1165), QSize(1920, 800),  QSize(1920, 1080), QSize(2048, 1080), QSize(2560, 1080),//
      QSize(2560, 1440), QSize(3050, 1885), QSize(3440, 1440), QSize(3840, 1600), QSize(3840, 2160),//
      QSize(4096, 2160), QSize(4935, 3050), QSize(7680, 4320), QSize(7985, 4935), QSize(12920, 7985),//
  };
  static constexpr int DEFAULT_IMAGE_SCALED_SIZE = 5, DEFAULT_NON_IMAGE_SCALED_SIZE = 0;
  static constexpr int ICON_SIZE_CANDIDATES_N = sizeof(ICON_SIZE_CANDIDATES) / sizeof(*ICON_SIZE_CANDIDATES);
  static QString HumanReadFriendlySize(int scaleIndex, bool* isValidScaledIndex = nullptr);
  static int clampScaledIndex(int newScaledIndex);
  static int GetInitialScaledSize(const QString& name);
  static void SaveInitialScaledSize(const QString& name, int scaledIndex);

  static bool GetInitialVerticalLongerMode(const QString& name);
  static void SaveInitialVerticalLongerMode(const QString& name, bool bIconVerticalLongerMode);

  static QSize CandidateSize(int scaledIndex, bool bIconVerticalLongerMode);
};

#endif // IMAGETOOL_H
