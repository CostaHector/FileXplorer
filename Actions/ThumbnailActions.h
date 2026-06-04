#ifndef THUMBNAILACTIONS_H
#define THUMBNAILACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>

class ThumbnailActions : public QObject {
  Q_OBJECT
public:
  static ThumbnailActions& GetInst();

  QWidget* GetThumbnailCreateTools(QWidget* parent);
  QWidget* GetThumbnailFrameTools(QWidget* parent);
  QWidget* GetStoryBoardToolbar(QWidget* parent);

  bool isSkipIfAlreadyExist() const { return _SKIP_IF_ALREADY_EXIST != nullptr && _SKIP_IF_ALREADY_EXIST->isChecked(); }

  QAction* _SKIP_IF_ALREADY_EXIST{nullptr};
  QAction* CREATE_THUMBNAIL_FOR_A_PATH{nullptr};
  QAction* _RENAME_STORYBOARD_FROM_POT_PLAYER{nullptr};

signals:
  void crtVideoStoryBoard(int rowCnt, int colCnt, int widthPixel, bool bSkipIfExist);
  void extractImages(int startIndex, int endIndex, bool bSkipIfExist);
  void grabFrames(int startPositionSecond, int intervalSecond, int framesCount, bool bSkipIfExist);

private:
  void subscribe();
  void emitCustomFramesRange();

  QAction* GRAB_A_FRAME{nullptr}, *GRAB_FRAMES_ADVANCE{nullptr};

  QAction* _CREATE_1_X_1_STORYBOARD{nullptr};
  QAction* _CREATE_2_X_2_STORYBOARD{nullptr};
  QAction* _CREATE_3_X_3_STORYBOARD{nullptr};
  QAction* _CREATE_4_X_4_STORYBOARD{nullptr};
  QActionGroup* _CREATE_THUMBNAIL_AG{nullptr};

  QAction* _EXTRACT_1ST_IMG{nullptr};
  QAction* _EXTRACT_2ND_IMGS{nullptr};
  QAction* _EXTRACT_4TH_IMGS{nullptr};
  QActionGroup* _EXTRACT_THUMBNAIL_AG{nullptr};
  QAction* _CUSTOM_RANGE_IMGS{nullptr};

  explicit ThumbnailActions(QObject* parent = nullptr);
};

#endif // THUMBNAILACTIONS_H
