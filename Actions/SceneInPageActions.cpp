#include "SceneInPageActions.h"
#include "MenuToolButton.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

SceneInPageActions& g_SceneInPageActions() {
  static SceneInPageActions ins;
  return ins;
}

SceneInPageActions::SceneInPageActions(QObject* parent) : QObject{parent} {
  _UPDATE_JSON = new (std::nothrow) QAction(QIcon(":img/UPDATE_JSON"), tr("Update K-V"), this);
  CHECK_NULLPTR_RETURN_VOID(_UPDATE_JSON);
  _UPDATE_JSON->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                   "Insert or update key-value pairs in JSON files recursively. "
                                   "Supports modifying video and image metadata fields. "
                                   "This operation will update the actual JSON file contents.")
                               .arg(_UPDATE_JSON->text(), _UPDATE_JSON->shortcut().toString()));

  _UPDATE_SCN = new (std::nothrow) QAction(QIcon(":img/UPDATE_SCN_FILE"), tr("Update scn"), this);
  CHECK_NULLPTR_RETURN_VOID(_UPDATE_SCN);
  _UPDATE_SCN->setShortcut(QKeySequence(Qt::Key_F5));
  _UPDATE_SCN->setShortcutVisibleInContextMenu(true);
  _UPDATE_SCN->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                  "Generate scene cache files (.scn) from JSON metadata files. "
                                  "Processes all JSON files recursively within the selected directory.")
                              .arg(_UPDATE_SCN->text(), _UPDATE_SCN->shortcut().toString()));

  bool disableImage{Configuration().value(SceneKey::SCENE_DISABLE_IMAGE_DECORATION.name, SceneKey::SCENE_DISABLE_IMAGE_DECORATION.v).toBool()};
  _DISABLE_IMAGE_DECORATION = new (std::nothrow) QAction(QIcon(":img/DISABLE_IMAGE_DECORATION"), tr("Performance mode(disable image)"), this);
  CHECK_NULLPTR_RETURN_VOID(_DISABLE_IMAGE_DECORATION);
  _DISABLE_IMAGE_DECORATION->setCheckable(true);
  _DISABLE_IMAGE_DECORATION->setChecked(disableImage);
  _DISABLE_IMAGE_DECORATION->setShortcutVisibleInContextMenu(true);
  _DISABLE_IMAGE_DECORATION->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                                "Hide images for faster performance. Otherwise show movie posters by default.\n"
                                                "(<i>Useful for faster sorting and filtering</i>)")
                                            .arg(_DISABLE_IMAGE_DECORATION->text(), _DISABLE_IMAGE_DECORATION->shortcut().toString()));

  _CLEAR_SCN_FILE = new (std::nothrow) QAction(QIcon(":img/CLEAR_SCN_FILES"), tr("Clear scn"), this);
  CHECK_NULLPTR_RETURN_VOID(_CLEAR_SCN_FILE);
  _CLEAR_SCN_FILE->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                      "Remove all scene cache files (.scn) recursively from the selected directory. "
                                      "This does not affect the original JSON source files.")
                                  .arg(_CLEAR_SCN_FILE->text(), _CLEAR_SCN_FILE->shortcut().toString()));

  using namespace SceneSortOrderHelper;
  _BY_MOVIE_PATH = new (std::nothrow) QAction(c_str(SortDimE::MOVIE_PATH), this);
  CHECK_NULLPTR_RETURN_VOID(_BY_MOVIE_PATH);
  _BY_MOVIE_PATH->setCheckable(true);

  _BY_MOVIE_SIZE = new (std::nothrow) QAction(c_str(SortDimE::MOVIE_SIZE), this);
  CHECK_NULLPTR_RETURN_VOID(_BY_MOVIE_SIZE);
  _BY_MOVIE_SIZE->setCheckable(true);

  _BY_RATE = new (std::nothrow) QAction(c_str(SortDimE::RATE), this);
  CHECK_NULLPTR_RETURN_VOID(_BY_RATE);
  _BY_RATE->setCheckable(true);

  _BY_UPLOADED_TIME = new (std::nothrow) QAction(c_str(SortDimE::UPLOADED_TIME), this);
  CHECK_NULLPTR_RETURN_VOID(_BY_UPLOADED_TIME);
  _BY_UPLOADED_TIME->setCheckable(true);

  bool reverseOrder{Configuration().value(SceneKey::SCENE_SORT_ORDER.name, SceneKey::SCENE_SORT_ORDER.v).toBool()};
  _REVERSE_RESULT = new (std::nothrow) QAction(QIcon{":img/ORDER_DESCENDING"}, tr("Reverse result"), this);
  CHECK_NULLPTR_RETURN_VOID(_REVERSE_RESULT);
  _REVERSE_RESULT->setCheckable(true);
  _REVERSE_RESULT->setChecked(reverseOrder);
  _REVERSE_RESULT->setToolTip("Toggle between ascending and descending order");

  _SORT_RANGE_PAGE_BY_PAGE = new (std::nothrow) QAction(tr("Sort Per Page"), this);
  CHECK_NULLPTR_RETURN_VOID(_SORT_RANGE_PAGE_BY_PAGE);
  _SORT_RANGE_PAGE_BY_PAGE->setCheckable(true);
  _SORT_RANGE_PAGE_BY_PAGE->setChecked(false);
  _SORT_RANGE_PAGE_BY_PAGE->setToolTip("Sort each page independently when enabled. Otherwise sort entire list by default");

  mSortOrderIntAction.init({{_BY_MOVIE_PATH, SortDimE::MOVIE_PATH},
                            {_BY_MOVIE_SIZE, SortDimE::MOVIE_SIZE},
                            {_BY_RATE, SortDimE::RATE},
                            {_BY_UPLOADED_TIME, SortDimE::UPLOADED_TIME}},
                           DEFAULT_SCENE_SORT_ORDER, QActionGroup::ExclusionPolicy::Exclusive);
  const int sortDim = Configuration().value(SceneKey::SCENE_SORT_BY_DIMENSION.name, SceneKey::SCENE_SORT_BY_DIMENSION.v).toInt();
  mSortOrderIntAction.setCheckedIfActionExist(sortDim);

  subscribe();
}

SceneInPageActions::~SceneInPageActions() {
  Configuration().setValue(SceneKey::SCENE_SORT_ORDER.name, GetSortOrderReverse());
  Configuration().setValue(SceneKey::SCENE_SORT_BY_DIMENSION.name, (int)GetSortDimension());
}

void SceneInPageActions::subscribe() {
  connect(_DISABLE_IMAGE_DECORATION, &QAction::toggled, this, &SceneInPageActions::disableImageDecorationChanged);
  connect(_REVERSE_RESULT, &QAction::toggled, this, &SceneInPageActions::onReverseSortOrderToggled);
  connect(mSortOrderIntAction.getActionGroup(), &QActionGroup::triggered, this, &SceneInPageActions::onSortDimensionTriggered);
}

void SceneInPageActions::onReverseSortOrderToggled(bool bReverseDescend) {
  if (GetSortRangeCurrentPageOnly()) {
    Qt::SortOrder newOrder = bReverseDescend ? Qt::DescendingOrder : Qt::AscendingOrder;
    emit scenesSortPolicyChanged(GetSortDimension(), newOrder);
  } else {
    emit sceneSortReverseOrderChanged(bReverseDescend);
  }
}

void SceneInPageActions::onSortDimensionTriggered(QAction* triggeredAct) {
  SceneSortOrderHelper::SortDimE sortDimension = mSortOrderIntAction.act2Enum(triggeredAct);
  if (GetSortRangeCurrentPageOnly()) {
    Qt::SortOrder newOrder = GetSortOrderReverse() ? Qt::DescendingOrder : Qt::AscendingOrder;
    emit scenesSortPolicyChanged(sortDimension, newOrder);
  } else {
    emit sceneSortDimensionChanged(sortDimension);
  }
}

QToolBar* SceneInPageActions::GetOrderToolBar(QWidget* parent) {
  auto* orderToolButton = new (std::nothrow) MenuToolButton(mSortOrderIntAction.getActionEnumAscendingList(),  //
                                                            QToolButton::InstantPopup,                         //
                                                            Qt::ToolButtonStyle::ToolButtonTextBesideIcon,     //
                                                            IMAGE_SIZE::TABS_ICON_IN_MENU_16,                  //
                                                            parent);
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  orderToolButton->SetCaption(QIcon{":img/SORTING_FILE_FOLDER"}, tr("Sort Dimension"));

  auto* orderTB = new (std::nothrow) QToolBar{"Scene Order", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(orderTB);

  orderTB->addWidget(orderToolButton);
  orderTB->addAction(_REVERSE_RESULT);
  orderTB->addAction(_SORT_RANGE_PAGE_BY_PAGE);
  orderTB->setOrientation(Qt::Orientation::Vertical);
  orderTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  orderTB->setIconSize(QSize{IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16});

  SetLayoutAlightment(orderTB->layout(), Qt::AlignmentFlag::AlignLeft);
  return orderTB;
}
