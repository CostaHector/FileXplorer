#ifndef MEMORYKEY_H
#define MEMORYKEY_H

#include "KV.h"
#include "SizeTool.h"

namespace MemoryKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV LANGUAGE_ZH_CN{"LANGUAGE_ZH_CN", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV ROW_HEIGHT{"ROW_HEIGHT", Var{SizeTool::TABLE_DEFAULT_ROW_SECTION_SIZE}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, INT_MAX>};
} // namespace MemoryKey

namespace PathKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV STARTUP_PATH{"PathKey/STARTUP_PATH", Var{"./"}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV LAST_TIME_COPY_TO{"PathKey/LAST_TIME_COPY_TO", Var{""}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV VIDEO_PLAYER_OPEN_PATH{"PathKey/VIDEO_PLAYER_OPEN_PATH", Var{"./"}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV PERFORMER_IMAGEHOST_LOCATE{"PathKey/PERFORMER_IMAGEHOST_LOCATE", Var{"./"}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV DB_INSERT_VIDS_FROM{"PathKey/DB_INSERT_VIDS_FROM", Var{"./"}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV DB_INSERT_TORRENTS_FROM{"PathKey/DB_INSERT_TORRENTS_FROM", Var{"./"}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
} // namespace PathKey

namespace BehaviorKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV NEW_CHOICE{"BehaviorKey/NEW_CHOICE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV COPY_CHOICE{"BehaviorKey/COPY_CHOICE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV RENAME_CHOICE{"BehaviorKey/RENAME_CHOICE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV THUMBNAILS_DIMENSION{"BehaviorKey/THUMBNAILS_DIMENSION", Var{"Create 2x2 thumbnail"}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV EXTRACT_CHOICE{"BehaviorKey/EXTRACT_CHOICE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV MOVE_TO_HISTORY{"BehaviorKey/MOVE_TO_HISTORY", Var{".\n..\n\\"}, GeneralDataType::Type::MULTI_LINE_STR, GeneralCStrChecker};
constexpr KV COPY_TO_HISTORY{"BehaviorKey/COPY_TO_HISTORY", Var{".\n..\n\\"}, GeneralDataType::Type::MULTI_LINE_STR, GeneralCStrChecker};
constexpr KV FILESYSTEM_STRUCTURE{"BehaviorKey/FILESYSTEM_STRUCTURE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};

constexpr KV VIDS_LAST_TABLE_NAME{"BehaviorKey/VIDS_LAST_TABLE_NAME", Var{""}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV WHERE_CLAUSE_HISTORY{"BehaviorKey/WHERE_CLAUSE_HISTORY", Var{"A\nA&B\nA|B"}, GeneralDataType::Type::MULTI_LINE_STR, GeneralCStrChecker};

constexpr KV BATCH_FILES_NAME_PATTERN{"BehaviorKey/BATCH_FILES_NAME_PATTERN", Var{"Page %03d%1$1$11.html"}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV BATCH_FOLDERS_NAME_PATTERN{"BehaviorKey/BATCH_FOLDERS_NAME_PATTERN", Var{"Page %03d%1$1$11"}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};

constexpr KV DIR_FILTER_ON_SWITCH_ENABLE{"BehaviorKey/DIR_FILTER_ON_SWITCH_ENABLE", Var{-1}, GeneralDataType::Type::PLAIN_INT, GeneralIntChecker};

constexpr KV ALL_LOG_LEVEL_AUTO_FFLUSH{"BehaviorKey/ALL_LOG_LEVEL_AUTO_FFLUSH", Var{(int) QtMsgType::QtWarningMsg}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV RETURN_ERRORCODE_UPON_ANY_FAILURE{"BehaviorKey/RETURN_ERRORCODE_UPON_ANY_FAILURE", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
} // namespace BehaviorKey

namespace SearchKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV INCLUDING_SUBDIRECTORIES{"SearchKey/INCLUDING_SUBDIRECTORIES", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV NAME_CASE_SENSITIVE{"SearchKey/NAME_CASE_SENSITIVE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV CONTENTS_CASE_SENSITIVE{"SearchKey/CONTENTS_CASE_SENSITIVE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV GRAY_ENTRIES_DONT_PASS_FILTER{"SearchKey/GRAY_ENTRIES_DONT_PASS_FILTER", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV ADVANCE_LINEEDIT_VALUE{"SearchKey/ADVANCE_LINEEDIT_VALUE", Var{""}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV ADVANCE_CONTENTS_LINEEDIT_VALUE{"SearchKey/ADVANCE_CONTENTS_LINEEDIT_VALUE", Var{""}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV ADVANCE_MODE{"SearchKey/ADVANCE_MODE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
} // namespace SearchKey

namespace CompoVisKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV MENU_RIBBON_CURRENT_TAB_INDEX{"CompoVisKey/MENU_RIBBON_CURRENT_TAB_INDEX", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV FOLDER_PREVIEW_TYPE{"CompoVisKey/FOLDER_PREVIEW_TYPE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV SHOW_PREVIEW_DOCKER{"CompoVisKey/SHOW_PREVIEW_DOCKER", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV SHOW_NAVIGATION_SIDEBAR{"CompoVisKey/SHOW_NAVIGATION_SIDEBAR", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV SHOW_HAR_IMAGE_PREVIEW{"CompoVisKey/SHOW_HAR_IMAGE_PREVIEW", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV EXPAND_NAVIGATION_SIDEBAR{"CompoVisKey/EXPAND_NAVIGATION_SIDEBAR", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV EXPAND_OFFICE_STYLE_MENUBAR{"CompoVisKey/EXPAND_OFFICE_STYLE_MENUBAR", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
} // namespace CompoVisKey

namespace SceneKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV SORT_ORDER_REVERSE{"SceneKey/SORT_ORDER_REVERSE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV SORT_BY_ROLE{"SceneKey/SORT_BY_ROLE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV DISABLE_IMAGE_DECORATION{"SceneKey/DISABLE_IMAGE_DECORATION", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV CNT_EACH_PAGE{"SceneKey/CNT_EACH_PAGE", Var{40}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, INT_MAX>};
} // namespace SceneKey

namespace BrowserKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES{"BrowserKey/CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS{"BrowserKey/CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV FLOATING_IMAGE_VIEW_SHOW{"BrowserKey/FLOATING_IMAGE_VIEW_SHOW", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV FLOATING_VIDEO_VIEW_SHOW{"BrowserKey/FLOATING_VIDEO_VIEW_SHOW", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV FLOATING_OTHER_VIEW_SHOW{"BrowserKey/FLOATING_OTHER_VIEW_SHOW", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV FLOATING_MEDIA_TYPE_SEQ{"BrowserKey/FLOATING_MEDIA_TYPE_SEQ", Var{"012"}, GeneralDataType::Type::PLAIN_STR, GeneralSequenceChecker};
} // namespace BrowserKey

namespace RedunImgFinderKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV ALSO_RECYCLE_EMPTY_IMAGE{"RedunImgFinderKey/ALSO_RECYCLE_EMPTY_IMAGE", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV RUND_IMG_PATH{"RedunImgFinderKey/RUND_IMG_PATH", Var{"."}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
} // namespace RedunImgFinderKey

namespace FavoritesNavigationKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV SORT_BY_ROLE{"FavoritesNavigationKey/SORT_BY_ROLE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV SORT_ORDER_REVERSE{"FavoritesNavigationKey/SORT_ORDER_REVERSE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
} // namespace FavoritesNavigationKey

namespace VideoPlayerKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV VOLUME{"VideoPlayerKey/VOLUME", Var{100}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};
constexpr KV MUTE{"VideoPlayerKey/MUTE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV PLAYBACK_MODE{"VideoPlayerKey/PLAYBACK_MODE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};
constexpr KV PLAYBACK_TRIGGER_MODE{"VideoPlayerKey/PLAYBACK_TRIGGER_MODE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};
constexpr KV AUTO_HIDE_TOOLBAR{"VideoPlayerKey/AUTO_HIDE_TOOLBAR", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV DUPLICATE_FINDER_DEVIATION_DURATION{"VideoPlayerKey/DUPLICATE_FINDER_DEVIATION_DURATION", Var{2 * 1000}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, INT_MAX>};   // 2s ~ 20s
constexpr KV DUPLICATE_FINDER_DEVIATION_FILESIZE{"VideoPlayerKey/DUPLICATE_FINDER_DEVIATION_FILESIZE", Var{2 * 1024}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, INT_MAX>}; // 2kB ~ 30MB

constexpr KV RATE_MOVIE_DEFAULT_VALUE{"VideoPlayerKey/RATE_MOVIE_DEFAULT_VALUE", Var{5}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 10>};
} // namespace VideoPlayerKey

namespace RenamerKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV NAME_EXT_INDEPENDENT{"RenamerKey/NAME_EXT_INDEPENDENT", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV INCLUDING_DIR{"RenamerKey/INCLUDING_DIR", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV INS_OLD_STR_LIST{"RenamerKey/INS_OLD_STR_LIST", //
                              Var{"\n"                       // insert nothing
                                  "BB\n"
                                  "- 360p\n"
                                  "- 480p\n"
                                  "- 516p\n"
                                  "- 720p\n"
                                  "- 1080p\n"
                                  "- 4K\n"
                                  "- FHD\n"
                                  // insert new items before me
                                  "- UHD"},                          //
                              GeneralDataType::Type::MULTI_LINE_STR, //
                              GeneralCStrChecker};
constexpr KV REP_OLD_STR_LIST{"RenamerKey/REP_OLD_STR_LIST",    //
                              Var{"\n"                          // replace nothing
                                  "\\s\\s+"                     // used for remove extra continous space
                                  "([a-z])([A-Z])\n"            // aA. used for add space between lowercase and uppercase
                                  "(\\d{4})(\\d{2})(\\d{2})]\n" // 19700101->1970-01-01. used for date add hypen
                                  "BB\n"
                                  "- 360p\n"
                                  "- 480p\n"
                                  "- 516p\n"
                                  "- 720p\n"
                                  "- 1080p\n"
                                  // insert new items before me
                                  "- 2160p"},                        //
                              GeneralDataType::Type::MULTI_LINE_STR, //
                              GeneralCStrChecker};
constexpr KV NEW_STR_LIST{"RenamerKey/NEW_STR_LIST", //
                          Var{"\n"                   // replaced with empty(i.e. delete)
                              " \n"                  // replaced with single space
                              "\\1 \\2\n"            // used for add space
                              "\\1 - \\2 - \\3\n"    // used for date
                              "HD\n"
                              "FHD\n"
                              // insert new items before me
                              "UHD"},
                          GeneralDataType::Type::MULTI_LINE_STR, //
                          GeneralCStrChecker};
constexpr KV INSERT_INDEXES_LIST{"RenamerKey/INSERT_INDEXES_LIST", //
                                 Var{"0\n"                         // insert at index 0
                                     "260\n"                       // insert after back
                                     // insert new items before me
                                     "100"},                            //
                                 GeneralDataType::Type::MULTI_LINE_STR, //
                                 GeneralCStrChecker};
constexpr KV ARRANGE_SECTION_INDEX{"RenamerKey/ARRANGE_SECTION_INDEX", Var{"1,2"}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV REGEX_ENABLED{"RenamerKey/REGEX_ENABLED", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};

constexpr KV NUMERIAZER_UNIQUE_EXT_COUNTER{"RenamerKey/NUMERIAZER_UNIQUE_EXT_COUNTER", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV NUMERIAZER_NO_FORMAT{"RenamerKey/NUMERIAZER_NO_FORMAT", //
                                  Var{" %1\n"                        //
                                      " - %1\n"                      //
                                      // insert new items before me
                                      " (%1)"},                          //
                                  GeneralDataType::Type::MULTI_LINE_STR, //
                                  GeneralBoolChecker};
constexpr KV NUMERIAZER_NO_FORMAT_DEFAULT_INDEX{"RenamerKey/NUMERIAZER_NO_FORMAT_DEFAULT_INDEX", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntChecker};
} // namespace RenamerKey

namespace JsonOpMemoryKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV STUDIO_OPERATION{"STUDIO_OPERATION", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>}; // [0,100]
constexpr KV CAST_OPERATION{"CAST_OPERATION", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};     // [0,100]
constexpr KV TAGS_OPERATION{"TAGS_OPERATION", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};     // [0,100]
} // namespace JsonOpMemoryKey

#endif // MEMORYKEY_H
