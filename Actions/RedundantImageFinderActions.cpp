#include "RedundantImageFinderActions.h"
#include "PublicMacro.h"
#include "MemoryKey.h"

RedundantImageFinderActions::RedundantImageFinderActions(QObject* parent)  //
    : QObject{parent} {
  FIND_DUPLICATE_IMGS_BY_LIBRARY = new (std::nothrow) QAction{QIcon{":img/DUPLICATE_IMAGES_BY_BENCHMARK_DICTIONARY"}, "By benchmark library", this};
  CHECK_NULLPTR_RETURN_VOID(FIND_DUPLICATE_IMGS_BY_LIBRARY);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setCheckable(true);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setChecked(true);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setShortcutVisibleInContextMenu(true);
  FIND_DUPLICATE_IMGS_BY_LIBRARY->setToolTip(
      "Active: Search duplicates via benchmark library."
      "Inactive: Compare MD5 checksums in current directory.");

  FIND_DUPLICATE_IMGS_IN_A_PATH = new (std::nothrow) QAction{QIcon{":img/DUPLICATE_IMAGES_BY_MD5"}, "By MD5", this};
  CHECK_NULLPTR_RETURN_VOID(FIND_DUPLICATE_IMGS_IN_A_PATH);
  FIND_DUPLICATE_IMGS_IN_A_PATH->setCheckable(true);
  FIND_DUPLICATE_IMGS_IN_A_PATH->setShortcutVisibleInContextMenu(true);
  FIND_DUPLICATE_IMGS_IN_A_PATH->setToolTip("Search duplicates in a folder");

  using namespace DuplicateImageDetectionCriteria;
  mDecideByIntAction.init({{FIND_DUPLICATE_IMGS_BY_LIBRARY, DICriteriaE::LIBRARY},  //
                           {FIND_DUPLICATE_IMGS_IN_A_PATH, DICriteriaE::MD5}},      //
                          DEFAULT_DI_CRITERIA_E, QActionGroup::ExclusionPolicy::Exclusive);
  mDecideByIntAction.setCheckedIfActionExist(DEFAULT_DI_CRITERIA_E);

  RECYLE_NOW = new (std::nothrow) QAction{QIcon{":img/MOVE_TO_TRASH_BIN"}, "Recycle", this};
  CHECK_NULLPTR_RETURN_VOID(RECYLE_NOW);
  RECYLE_NOW->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_Delete));
  RECYLE_NOW->setToolTip(QString("<b>%1 (%2)</b><br/> Move the selected item(s) to the Recyle Bin.")  //
                             .arg(RECYLE_NOW->text(), RECYLE_NOW->shortcut().toString()));

  INCLUDING_EMPTY_IMAGES = new (std::nothrow) QAction{QIcon{":img/FILE"}, "Include empty", this};
  CHECK_NULLPTR_RETURN_VOID(INCLUDING_EMPTY_IMAGES);
  INCLUDING_EMPTY_IMAGES->setCheckable(true);
  INCLUDING_EMPTY_IMAGES->setChecked(
      Configuration().value(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.v).toBool());
  INCLUDING_EMPTY_IMAGES->setToolTip(
      QString("<b>%1 (%2)</b><br/> Blank images (with a file size of 0Byte) will also be considered redundant images.")  //
          .arg(INCLUDING_EMPTY_IMAGES->text(), INCLUDING_EMPTY_IMAGES->shortcut().toString()));

  OPEN_BENCHMARK_FOLDER = new (std::nothrow) QAction{QIcon(":img/FOLDER_OPEN"), "Open library", this};
  CHECK_NULLPTR_RETURN_VOID(OPEN_BENCHMARK_FOLDER);
  OPEN_BENCHMARK_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/> Open the path where the redundant image library(benchmark) is located.")  //
                                        .arg(OPEN_BENCHMARK_FOLDER->text(), OPEN_BENCHMARK_FOLDER->shortcut().toString()));

  RELOAD_BENCHMARK_LIB = new (std::nothrow) QAction{QIcon{":img/RELOAD_FROM_DISK"}, "Reload library", this};
  CHECK_NULLPTR_RETURN_VOID(RELOAD_BENCHMARK_LIB);
  RELOAD_BENCHMARK_LIB->setToolTip(QString("<b>%1 (%2)</b><br/> Reload the path where the redundant image library(benchmark) is located again")  //
                                       .arg(RELOAD_BENCHMARK_LIB->text(), RELOAD_BENCHMARK_LIB->shortcut().toString()));
}

RedundantImageFinderActions& g_redunImgFinderAg() {
  static RedundantImageFinderActions ins;
  return ins;
}
