#include "SceneInPageActions.h"
#include "MenuToolButton.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

SceneInPageActions& g_SceneInPageActions() {
  static SceneInPageActions ins;
  return ins;
}

SceneInPageActions::SceneInPageActions(QObject* parent) : QObject{parent} {
  _UPDATE_JSON = new (std::nothrow) QAction(QIcon(":img/UPDATE_JSON"), "Update K-V", this);
  CHECK_NULLPTR_RETURN_VOID(_UPDATE_JSON);
  _UPDATE_JSON->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                   "Insert or update key-value pairs in JSON files recursively. "
                                   "Supports modifying video and image metadata fields. "
                                   "This operation will update the actual JSON file contents.")
                               .arg(_UPDATE_JSON->text(), _UPDATE_JSON->shortcut().toString()));

  _UPDATE_SCN = new (std::nothrow) QAction(QIcon(":img/UPDATE_SCN_FILE"), "Update scn", this);
  CHECK_NULLPTR_RETURN_VOID(_UPDATE_SCN);
  _UPDATE_SCN->setShortcut(QKeySequence(Qt::Key_F5));
  _UPDATE_SCN->setShortcutVisibleInContextMenu(true);
  _UPDATE_SCN->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                  "Generate scene cache files (.scn) from JSON metadata files. "
                                  "Processes all JSON files recursively within the selected directory.")
                              .arg(_UPDATE_SCN->text(), _UPDATE_SCN->shortcut().toString()));

  _CLEAR_SCN_FILE = new (std::nothrow) QAction(QIcon(":img/CLEAR_SCN_FILES"), "Clear scn", this);
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

  _REVERSE_SORT = new (std::nothrow) QAction("Reverse", this);
  CHECK_NULLPTR_RETURN_VOID(_REVERSE_SORT);
  _REVERSE_SORT->setCheckable(true);
  _REVERSE_SORT->setChecked(Configuration().value(MemoryKey::SCENE_SORT_ORDER.name, MemoryKey::SCENE_SORT_ORDER.v).toInt() ==
                            Qt::SortOrder::DescendingOrder);

  mSortOrderIntAction.init({{_BY_MOVIE_PATH, SortDimE::MOVIE_PATH},
                            {_BY_MOVIE_SIZE, SortDimE::MOVIE_SIZE},
                            {_BY_RATE, SortDimE::RATE},
                            {_BY_UPLOADED_TIME, SortDimE::UPLOADED_TIME}},
                           DEFAULT_SCENE_SORT_ORDER, QActionGroup::ExclusionPolicy::Exclusive);
  const int sortDim = Configuration().value(MemoryKey::SCENE_SORT_BY_DIMENSION.name, MemoryKey::SCENE_SORT_BY_DIMENSION.v).toInt();
  mSortOrderIntAction.setCheckedIfActionExist(sortDim);

  subscribe();
}

SceneInPageActions::~SceneInPageActions() {
  SceneSortOrderHelper::SortDimE sortDimension = SceneSortOrderHelper::DEFAULT_SCENE_SORT_ORDER;
  Qt::SortOrder sortOrder = Qt::SortOrder::AscendingOrder;
  std::tie(sortDimension, sortOrder) = GetSortSetting();

  Configuration().setValue(MemoryKey::SCENE_SORT_ORDER.name, (int)sortOrder);
  Configuration().setValue(MemoryKey::SCENE_SORT_BY_DIMENSION.name, (int)sortDimension);
}

void SceneInPageActions::subscribe() {
  connect(_REVERSE_SORT, &QAction::toggled, this, &SceneInPageActions::EmitScenesSortPolicyChangedSignal);
  connect(mSortOrderIntAction.getActionGroup(), &QActionGroup::triggered, this, &SceneInPageActions::EmitScenesSortPolicyChangedSignal);
}

std::pair<SceneSortOrderHelper::SortDimE, Qt::SortOrder> SceneInPageActions::GetSortSetting() const {
  SceneSortOrderHelper::SortDimE sortDimension = mSortOrderIntAction.curVal();
  Qt::SortOrder sortOrder = _REVERSE_SORT->isChecked() ? Qt::SortOrder::DescendingOrder : Qt::SortOrder::AscendingOrder;
  return {sortDimension, sortOrder};
}

void SceneInPageActions::EmitScenesSortPolicyChangedSignal() {
  SceneSortOrderHelper::SortDimE sortDimension = SceneSortOrderHelper::DEFAULT_SCENE_SORT_ORDER;
  Qt::SortOrder sortOrder = Qt::SortOrder::AscendingOrder;
  std::tie(sortDimension, sortOrder) = GetSortSetting();
  LOG_D("Signal emit with parms sortDim[%s] order:%d", SceneSortOrderHelper::c_str(sortDimension), (int)sortOrder);
  emit scenesSortPolicyChanged(sortDimension, sortOrder);
}

QToolBar* SceneInPageActions::GetOrderToolBar(QWidget* parent) {
  auto* orderToolButton = new (std::nothrow) MenuToolButton(mSortOrderIntAction.getActionEnumAscendingList(),  //
                                                            QToolButton::InstantPopup,                         //
                                                            Qt::ToolButtonStyle::ToolButtonTextBesideIcon,     //
                                                            IMAGE_SIZE::TABS_ICON_IN_MENU_48,                  //
                                                            parent);
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  orderToolButton->SetCaption(QIcon{":img/SORTING_FILE_FOLDER"}, "Sort Dimension");

  auto* orderTB = new (std::nothrow) QToolBar{"Scene Order", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(orderTB);

  orderTB->addWidget(orderToolButton);
  orderTB->addAction(_REVERSE_SORT);
  orderTB->setOrientation(Qt::Orientation::Vertical);
  orderTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);

  SetLayoutAlightment(orderTB->layout(), Qt::AlignmentFlag::AlignLeft);
  return orderTB;
}
