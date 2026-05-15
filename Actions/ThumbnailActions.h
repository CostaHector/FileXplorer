#ifndef THUMBNAILACTIONS_H
#define THUMBNAILACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>

class ThumbnailActions : public QObject {
  Q_OBJECT
public:
  static ThumbnailActions& GetInst();

  QWidget* GetCreateThumbnailToolbar(QWidget* parent);

  bool isSkipIfAlreadyExist() const { return _SKIP_IF_ALREADY_EXIST != nullptr && _SKIP_IF_ALREADY_EXIST->isChecked(); }

  QAction* CREATE_THUMBNAIL_FOR_A_PATH{nullptr};
  QAction* _RENAME_THUMBNAILS_FROM_POT_PLAYER{nullptr};

signals:
  void crtVideoStoryBoard(int rowCnt, int colCnt, int widthPixel, bool bSkipIfExist);
  void extractFrames(int startIndex, int endIndex, bool bSkipIfExist);

private:
  void subscribe();
  void emitCustomFramesRange();

  QAction* _SKIP_IF_ALREADY_EXIST{nullptr};

  QAction* _CREATE_1_BY_1_THUMBNAIL{nullptr};
  QAction* _CREATE_2_BY_2_THUMBNAIL{nullptr};
  QAction* _CREATE_3_BY_3_THUMBNAIL{nullptr};
  QAction* _CREATE_4_BY_4_THUMBNAIL{nullptr};
  QActionGroup* _CREATE_THUMBNAIL_AG{nullptr};

  QAction* _EXTRACT_1ST_IMG{nullptr};
  QAction* _EXTRACT_2ND_IMGS{nullptr};
  QAction* _EXTRACT_4TH_IMGS{nullptr};
  QActionGroup* _EXTRACT_THUMBNAIL_AG{nullptr};
  QAction* _CUSTOM_RANGE_IMGS{nullptr};

  explicit ThumbnailActions(QObject* parent = nullptr);
};

#endif // THUMBNAILACTIONS_H
