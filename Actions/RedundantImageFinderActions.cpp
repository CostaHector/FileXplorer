#include "RedundantImageFinderActions.h"
#include "PublicMacro.h"
#include "RedunImgFinderKey.h"
#include "Configuration.h"
#include "MenuToolButton.h"
#include "ImagesInfoManager.h"
#include "FileTool.h"
#include <QToolBar>

using namespace DuplicateImageDetectionCriteria;

RedundantImageFinderActions::RedundantImageFinderActions(QObject* parent) //
  : QObject{parent} {
  FIND_DUPLICATE_IMGS_BY_LIBRARY = new (std::nothrow) QAction{QIcon{":img/DUPLICATE_IMAGES_BY_BENCHMARK_DICTIONARY"}, tr("Find by benchmark library"), this};
  CHECK_NULLPTR_RETURN_VOID(FIND_DUPLICATE_IMGS_BY_LIBRARY);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setCheckable(true);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setChecked(true);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setShortcutVisibleInContextMenu(true);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setToolTip("When active: Find duplicate images by comparing against a benchmark library.\n"
                                             "When inactive: Compare MD5 checksums within the current directory only.");

  FIND_DUPLICATE_IMGS_IN_A_PATH = new (std::nothrow) QAction{QIcon{":img/DUPLICATE_IMAGES_BY_MD5"}, tr("Find by MD5 checksum"), this};
  CHECK_NULLPTR_RETURN_VOID(FIND_DUPLICATE_IMGS_IN_A_PATH);
  FIND_DUPLICATE_IMGS_IN_A_PATH->setCheckable(true);
  FIND_DUPLICATE_IMGS_IN_A_PATH->setShortcutVisibleInContextMenu(true);
  FIND_DUPLICATE_IMGS_IN_A_PATH->setToolTip("Find duplicate images in a folder by calculating MD5 checksums");

  using namespace DuplicateImageDetectionCriteria;
  mDecideByIntAction.init({{FIND_DUPLICATE_IMGS_BY_LIBRARY, DICriteriaE::LIBRARY}, //
                           {FIND_DUPLICATE_IMGS_IN_A_PATH, DICriteriaE::MD5}},     //
                          DEFAULT_DI_CRITERIA_E,
                          QActionGroup::ExclusionPolicy::Exclusive);
  mDecideByIntAction.setCheckedIfActionExist(DEFAULT_DI_CRITERIA_E);

  RECYLE_NOW = new (std::nothrow) QAction{QIcon{":img/MOVE_TO_TRASH_BIN"}, tr("Move to Recycle Bin"), this};
  CHECK_NULLPTR_RETURN_VOID(RECYLE_NOW);
  RECYLE_NOW->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_Delete));
  RECYLE_NOW->setToolTip(QString("<b>%1 (%2)</b><br/>Move selected items to the Recycle Bin.") //
                             .arg(RECYLE_NOW->text(), RECYLE_NOW->shortcut().toString()));

  INCLUDING_EMPTY_IMAGES = new (std::nothrow) QAction{QIcon{":img/FILE"}, tr("Include zero-byte images"), this};
  CHECK_NULLPTR_RETURN_VOID(INCLUDING_EMPTY_IMAGES);
  INCLUDING_EMPTY_IMAGES->setCheckable(true);
  INCLUDING_EMPTY_IMAGES->setChecked(getConfig(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE).toBool());
  INCLUDING_EMPTY_IMAGES->setToolTip(QString("<b>%1</b><br/>Include zero-byte images when searching for duplicates.") //
                                         .arg(INCLUDING_EMPTY_IMAGES->text()));

  OPEN_BENCHMARK_FOLDER = new (std::nothrow) QAction{QIcon(":img/FOLDER_OPEN"), tr("Open benchmark folder"), this};
  CHECK_NULLPTR_RETURN_VOID(OPEN_BENCHMARK_FOLDER);
  OPEN_BENCHMARK_FOLDER->setToolTip(QString("<b>%1</b><br/>Open the folder containing the benchmark image library.") //
                                        .arg(OPEN_BENCHMARK_FOLDER->text()));

  RELOAD_BENCHMARK_LIB = new (std::nothrow) QAction{QIcon{":/JsonEditor/RELOAD_FROM_DISK"}, tr("Reload benchmark library"), this};
  CHECK_NULLPTR_RETURN_VOID(RELOAD_BENCHMARK_LIB);
  RELOAD_BENCHMARK_LIB->setToolTip(QString("<b>%1</b><br/>Reload the benchmark image library from disk.") //
                                       .arg(RELOAD_BENCHMARK_LIB->text()));

  _DISABLE_IMAGE_DECORATION = new (std::nothrow) QAction(QIcon(":img/DISABLE_IMAGE_DECORATION"), tr("Performance mode"), this);
  CHECK_NULLPTR_RETURN_VOID(_DISABLE_IMAGE_DECORATION);
  _DISABLE_IMAGE_DECORATION->setCheckable(true);
  const bool disableImage{getConfig(RedunImgFinderKey::DISABLE_IMAGE_DECORATION).toBool()};
  _DISABLE_IMAGE_DECORATION->setChecked(disableImage);
  _DISABLE_IMAGE_DECORATION->setShortcutVisibleInContextMenu(true);
  _DISABLE_IMAGE_DECORATION->setToolTip(QString("<b>%1</b><br/>"
                                                "Enable performance mode to hide image previews for faster operation.\n"
                                                "Disable to show image thumbnails.")
                                            .arg(_DISABLE_IMAGE_DECORATION->text()));

  subscribe();
}

QWidget* RedundantImageFinderActions::GetSearchByToolButton(QWidget* parent) {
  MenuToolButton* mFindImgByTb = new MenuToolButton{mDecideByIntAction.getActionEnumAscendingList(), //
                                                    QToolButton::ToolButtonPopupMode::InstantPopup,            //
                                                    Qt::ToolButtonStyle::ToolButtonTextBesideIcon,             //
                                                    IMAGE_SIZE::TABS_ICON_IN_MENU_36,                          //
                                                    parent};                                                   //
  CHECK_NULLPTR_RETURN_NULLPTR(mFindImgByTb);
  mFindImgByTb->SetCaption(QIcon{":img/DUPLICATE_IMAGES_FINDER"}, tr("Find By Mode"), "");
  return mFindImgByTb;
}

void RedundantImageFinderActions::AddRelatedAction(QToolBar* tb) {
  CHECK_NULLPTR_RETURN_VOID(tb);

  auto* searchByToolButton = GetSearchByToolButton(tb);
  CHECK_NULLPTR_RETURN_VOID(searchByToolButton);

  tb->addWidget(searchByToolButton);
  tb->addAction(INCLUDING_EMPTY_IMAGES);
  tb->addSeparator();
  tb->addAction(OPEN_BENCHMARK_FOLDER);
  tb->addAction(RELOAD_BENCHMARK_LIB);
  tb->addSeparator();
  tb->addAction(_DISABLE_IMAGE_DECORATION);
  tb->addSeparator();
  tb->addAction(RECYLE_NOW);
}

void RedundantImageFinderActions::subscribe() { //
  connect(_DISABLE_IMAGE_DECORATION, &QAction::toggled, this, &RedundantImageFinderActions::onDisableImageDecorationToggled);
  connect(mDecideByIntAction.getActionGroup(), &QActionGroup::triggered, this, &RedundantImageFinderActions::onFindByActionTriggered);
  connect(RELOAD_BENCHMARK_LIB, &QAction::triggered, []() { ImagesInfoManager::getInst().ForceReloadImpl(); });
  connect(OPEN_BENCHMARK_FOLDER, &QAction::triggered, this, &RedundantImageFinderActions::onOpenBenchmarkFolder);
}

void RedundantImageFinderActions::onDisableImageDecorationToggled(bool bDisabled) {
  // user may want to disable image decoration before show scene view, so here must write changes into memory
  setConfig(RedunImgFinderKey::DISABLE_IMAGE_DECORATION, bDisabled);
  emit disableImageDecorationChanged(bDisabled);
}

void RedundantImageFinderActions::onFindByActionTriggered(const QAction* findByAct) {
  CHECK_NULLPTR_RETURN_VOID(findByAct);
  DICriteriaE findBy = mDecideByIntAction.act2Enum(findByAct);
  emit findByChanged(findBy);
}

RedundantImageFinderActions& g_redunImgFinderAg() {
  static RedundantImageFinderActions ins;
  return ins;
}

bool RedundantImageFinderActions::onOpenBenchmarkFolder() {
  const ImagesInfoManager& redunImgLibInst = ImagesInfoManager::getInst();
  const QString benchmarkPath = redunImgLibInst.GetDynRedunPath();
  return FileTool::OpenLocalFileUsingDesktopService(benchmarkPath);
}
