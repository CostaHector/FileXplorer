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
  _REVERSE_ORDER = new (std::nothrow) QAction(QIcon{":img/ORDER_DESCENDING"}, tr("Reverse"), this);
  CHECK_NULLPTR_RETURN_VOID(_REVERSE_ORDER);
  _REVERSE_ORDER->setCheckable(true);
  _REVERSE_ORDER->setChecked(reverseOrder);
  _REVERSE_ORDER->setToolTip("Toggle between ascending and descending order");

  _SORT_RANGE_CURRENT_PAGE = new (std::nothrow) QAction(tr("Sort Current Page Only"), this);
  CHECK_NULLPTR_RETURN_VOID(_REVERSE_ORDER);
  _SORT_RANGE_CURRENT_PAGE->setCheckable(true);
  _SORT_RANGE_CURRENT_PAGE->setChecked(false);
  _SORT_RANGE_CURRENT_PAGE->setToolTip("Apply sorting to current page only (unchecked by default: sort entire list)");

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
  connect(_REVERSE_ORDER, &QAction::toggled, this, &SceneInPageActions::onReverseSortOrderToggled);
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
  orderTB->addAction(_REVERSE_ORDER);
  orderTB->addAction(_SORT_RANGE_CURRENT_PAGE);
  orderTB->setOrientation(Qt::Orientation::Vertical);
  orderTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  orderTB->setIconSize(QSize{IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16});

  SetLayoutAlightment(orderTB->layout(), Qt::AlignmentFlag::AlignLeft);
  return orderTB;
}
