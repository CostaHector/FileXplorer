#include "SceneInPageActions.h"
#include "MenuToolButton.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

SceneInPageActions& g_SceneInPageActions() {
  static SceneInPageActions ins;
  return ins;
}

SceneInPageActions::SceneInPageActions(QObject* parent)
  : QObject{parent} {
  _COMBINE_MEDIAINFOS_JSON = new (std::nothrow) QAction(QIcon(":img/UPDATE_JSON_THEN_SCN"), "Update infos", this);
  CHECK_NULLPTR_RETURN_VOID(_COMBINE_MEDIAINFOS_JSON);
  _COMBINE_MEDIAINFOS_JSON->setShortcut(QKeySequence(Qt::Key_F5));
  _COMBINE_MEDIAINFOS_JSON->setShortcutVisibleInContextMenu(true);
  _COMBINE_MEDIAINFOS_JSON->setToolTip(QString("<b>%1 (%2)</b><br/> Update Value of Videos/Images Field in json files."
                                               "Generate .scn file from usefull json file(s)."
                                               "This operation may modify json file contents")
                                           .arg(_COMBINE_MEDIAINFOS_JSON->text(), _COMBINE_MEDIAINFOS_JSON->shortcut().toString()));

  _CLEAR_SCN_FILE = new (std::nothrow) QAction(QIcon(":img/CLEAR_SCN_FILES"), "Clear .scn", this);
  CHECK_NULLPTR_RETURN_VOID(_CLEAR_SCN_FILE);
  _CLEAR_SCN_FILE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Clear scn file(s) recursively").arg(_CLEAR_SCN_FILE->text(), _CLEAR_SCN_FILE->shortcut().toString()));

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
  _REVERSE_SORT->setChecked(Configuration().value(MemoryKey::SCENE_SORT_ORDER.name, MemoryKey::SCENE_SORT_ORDER.v).toInt()
                            == Qt::SortOrder::DescendingOrder);

  mSortOrderIntAction.init({{_BY_MOVIE_PATH, SortDimE::MOVIE_PATH},
                            {_BY_MOVIE_SIZE, SortDimE::MOVIE_SIZE},
                            {_BY_RATE, SortDimE::RATE},
                            {_BY_UPLOADED_TIME, SortDimE::UPLOADED_TIME}},
                           DEFAULT_SCENE_SORT_ORDER,
                           QActionGroup::ExclusionPolicy::Exclusive);
  const int sortDim = Configuration().value(MemoryKey::SCENE_SORT_BY_DIMENSION.name, MemoryKey::SCENE_SORT_BY_DIMENSION.v).toInt();
  mSortOrderIntAction.setCheckedIfActionExist(sortDim);

  subscribe();
}

SceneInPageActions::~SceneInPageActions() {
  SceneSortOrderHelper::SortDimE sortDimension = SceneSortOrderHelper::DEFAULT_SCENE_SORT_ORDER;
  Qt::SortOrder sortOrder = Qt::SortOrder::AscendingOrder;
  std::tie(sortDimension, sortOrder) = GetSortSetting();

  Configuration().setValue(MemoryKey::SCENE_SORT_ORDER.name, (int) sortOrder);
  Configuration().setValue(MemoryKey::SCENE_SORT_BY_DIMENSION.name, (int) sortDimension);
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
  LOG_D("Signal emit with parms sortDim[%s] order:%d", SceneSortOrderHelper::c_str(sortDimension), (int) sortOrder);
  emit scenesSortPolicyChanged(sortDimension, sortOrder);
}

QToolBar* SceneInPageActions::GetOrderToolBar(QWidget* parent) {
  auto* orderToolButton = new (std::nothrow) MenuToolButton(mSortOrderIntAction.getActionEnumAscendingList(), //
                                                            QToolButton::InstantPopup,                        //
                                                            Qt::ToolButtonStyle::ToolButtonTextBesideIcon,    //
                                                            IMAGE_SIZE::TABS_ICON_IN_MENU_48,                 //
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
