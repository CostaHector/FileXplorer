#ifndef SCENEINPAGEACTIONS_H
#define SCENEINPAGEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class QLineEdit;
class QToolBar;

class SceneInPageActions : public QObject {
 public:
  explicit SceneInPageActions(QObject* parent = nullptr);

  bool InitWidget();
  QToolBar* GetOrderToolBar();
  QToolBar* GetPagesRowByColumnToolBar();
  QToolBar* GetImageSizeToolBar();

  QAction* _COMBINE_MEDIAINFOS_JSON{nullptr};
  QAction* _UPDATE_SCN_ONLY{nullptr};

  QAction* _BY_MOVIE_NAME{nullptr};
  QAction* _BY_MOVIE_SIZE{nullptr};
  QAction* _BY_RATE{nullptr};
  QAction* _BY_UPLOADED_TIME{nullptr};
  QAction* _REVERSE_SORT{nullptr};
  QActionGroup* _ORDER_AG{nullptr};

  QAction* _GROUP_BY_PAGE{nullptr};
  QAction* _THE_FIRST_PAGE{nullptr};
  QAction* _LAST_PAGE{nullptr};
  QAction* _NEXT_PAGE{nullptr};
  QAction* _THE_LAST_PAGE{nullptr};

  // order | page action
  // order | ----------
  // order | line edit
  // order | ----------
  // order | page select
  QToolBar* mOrderTB{nullptr};
  QToolBar* mEnablePageTB{nullptr};

  QLineEdit* mPageDimensionLE{nullptr};
  QLineEdit* mPageIndexInputLE{nullptr};
  QToolBar* mPagesSelectTB{nullptr};

  QLineEdit* mImageSize{nullptr};
  QToolBar* mImageSizeTB{nullptr};
};

SceneInPageActions& g_SceneInPageActions();

#endif  // SCENEINPAGEACTIONS_H
