#ifndef THUMBNAILPROCESSACTIONS_H
#define THUMBNAILPROCESSACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>

struct ThumbnailDimension {
  int x;
  int y;
  int width;
};

struct ThumbnailExtractIndexRange {
  int startIndex;
  int endIndex;
};

class ThumbnailProcessActions : public QObject {
 public:
  explicit ThumbnailProcessActions(QObject* parent = nullptr);

  QAction* _CREATE_1_BY_1_THUMBNAIL{nullptr};
  QAction* _CREATE_2_BY_2_THUMBNAIL{nullptr};
  QAction* _CREATE_3_BY_3_THUMBNAIL{nullptr};
  QAction* _CREATE_4_BY_4_THUMBNAIL{nullptr};
  QActionGroup* _CREATE_THUMBNAIL_AG{nullptr};
  QMap<QAction*, ThumbnailDimension> mCreateThumbnailDimension;
  QAction* _THUMBNAIL_SAMPLE_PERIOD{nullptr};

  QAction* _EXTRACT_1ST_IMG{nullptr};
  QAction* _EXTRACT_2ND_IMGS{nullptr};
  QAction* _EXTRACT_4TH_IMGS{nullptr};
  QActionGroup* _EXTRACT_THUMBNAIL_AG{nullptr};
  QMap<QAction*, ThumbnailExtractIndexRange> mExtractThumbnailRange;
  QAction* _CUSTOM_RANGE_IMGS{nullptr};
  QAction* _SKIP_IF_ALREADY_EXIST{nullptr};

  QToolBar* GetThumbnailToolbar(QWidget* parent = nullptr);
};

ThumbnailProcessActions& g_ThumbnailProcessActions();

#endif  // THUMBNAILPROCESSACTIONS_H
