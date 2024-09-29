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
  QToolBar* GetSceneToolbar();
  QToolBar* GetOrderToolBar();
  QToolBar* GetPagesRowByColumnToolBar();
  QToolBar* GetPageIndexSelectionToolBar();

  QAction* _COMBINE_MEDIAINFOS_JSON{nullptr};

  QAction* _ASCENDING{nullptr};
  QAction* _DESCENDING{nullptr};
  QActionGroup* _ORDER_AG{nullptr};

  QAction* _GROUP_BY_PAGE{nullptr};
  QAction* _THE_LAST_PAGE{nullptr};
  QAction* _LAST_PAGE{nullptr};
  QAction* _NEXT_PAGE{nullptr};
  QAction* _THE_FIRST_PAGE{nullptr};

  QLineEdit* mRowsInputLE{nullptr};
  QLineEdit* mColumnsInputLE{nullptr};
  QLineEdit* mPageIndexInputLE{nullptr};

  QToolBar* mOrderTB{nullptr};
  QToolBar* mEnablePageTB{nullptr};
  QToolBar* mRowEnablePageTB{nullptr};
  QToolBar* mPagesSelectTB{nullptr};
 private:
  constexpr static int ROW_COLUMN_LINEDIT_MAX_WIDTH = 80;
};

SceneInPageActions& g_SceneInPageActions();

#endif // SCENEINPAGEACTIONS_H
