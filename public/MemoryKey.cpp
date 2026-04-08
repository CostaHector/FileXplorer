#include "MemoryKey.h"
#include "SizeTool.h"
#include <QDir>

QList<const KV*> KV::mEditableKVs;

KV::KV(const QString& name_, const QVariant& v_, const ValueChecker& checker_, bool bUserEditable)  //
    : name{name_}, v{v_}, checker{checker_} {
  if (bUserEditable) {
    mEditableKVs.push_back(this);
  }
}

QString KV::InitialValueToString() const {
  return checker.valueToString(v);
}

QString KV::valueToString(const QVariant& v_) const {
  return checker.valueToString(v_);
}

using namespace VALUE_CHECKER_TYPE;

const KV MemoryKey::LANGUAGE_ZH_CN("LANGUAGE_ZH_CN", false, ValueChecker{PLAIN_BOOL});
const KV MemoryKey::ROW_HEIGHT{"ROW_HEIGHT", SizeTool::TABLE_DEFAULT_ROW_SECTION_SIZE, ValueChecker{0, 9999}};

const KV FontKey::FAMILY("FontKey/FAMILY",
#ifdef _WIN32
                         "Microsoft YaHei",
#else
                         "Noto Sans",
#endif
                         ValueChecker{PLAIN_STR});
const KV FontKey::POINT("FontKey/POINT", 12, ValueChecker{8, 25 + 1});

const KV PathKey::STARTUP_PATH{"PathKey/STARTUP_PATH", "./", ValueChecker{FOLDER_PATH}};
const KV PathKey::LAST_TIME_COPY_TO("PathKey/LAST_TIME_COPY_TO", "", ValueChecker{FOLDER_PATH});
const KV PathKey::JSON_EDITOR_LOAD_FROM("PathKey/JSON_EDITOR_LOAD_FROM", "", ValueChecker{FOLDER_PATH});
const KV PathKey::VIDEO_PLAYER_OPEN_PATH("PathKey/VIDEO_PLAYER_OPEN_PATH", "./", ValueChecker{FOLDER_PATH});
const KV PathKey::PERFORMER_IMAGEHOST_LOCATE("PathKey/PERFORMER_IMAGEHOST_LOCATE", "./", ValueChecker{FOLDER_PATH}, true);
const KV PathKey::DB_INSERT_VIDS_FROM("PathKey/DB_INSERT_VIDS_FROM", "./", ValueChecker{FOLDER_PATH});
const KV PathKey::DB_INSERT_TORRENTS_FROM("PathKey/DB_INSERT_VIDS_FROM", "./", ValueChecker{FOLDER_PATH});

const KV BehaviorKey::NEW_CHOICE("BehaviorKey/NEW_CHOICE", 0, ValueChecker{0, 100 + 1});
const KV BehaviorKey::COPY_CHOICE("BehaviorKey/COPY_CHOICE", 0, ValueChecker{0, 100 + 1});
const KV BehaviorKey::RENAME_CHOICE("BehaviorKey/RENAME_CHOICE", 0, ValueChecker{0, 100 + 1});
const KV BehaviorKey::THUMBNAILS_DIMENSION("BehaviorKey/THUMBNAILS_DIMENSION", "Create 2x2 thumbnail", ValueChecker{PLAIN_STR});
const KV BehaviorKey::EXTRACT_CHOICE("BehaviorKey/EXTRACT_CHOICE", 0, ValueChecker{0, 100 + 1});

const KV BehaviorKey::MOVE_TO_HISTORY("BehaviorKey/MOVE_TO_HISTORY", ".\n..\n\\", ValueChecker{PLAIN_STR});
const KV BehaviorKey::COPY_TO_HISTORY("BehaviorKey/COPY_TO_HISTORY", ".\n..\n\\", ValueChecker{PLAIN_STR});
const KV BehaviorKey::FILESYSTEM_STRUCTURE{"BehaviorKey/FILESYSTEM_STRUCTURE", 0, ValueChecker{PLAIN_INT}};

const KV BehaviorKey::VIDS_LAST_TABLE_NAME("BehaviorKey/VIDS_LAST_TABLE_NAME", "", ValueChecker{PLAIN_STR}, true);
const KV BehaviorKey::WHERE_CLAUSE_HISTORY("BehaviorKey/WHERE_CLAUSE_HISTORY", "A\nA&B\nA|B", ValueChecker{PLAIN_STR}, true);

const KV BehaviorKey::BATCH_FILES_NAME_PATTERN("BehaviorKey/BATCH_FILES_NAME_PATTERN", "Page %03d%1$1$11.html", ValueChecker{PLAIN_STR});
const KV BehaviorKey::BATCH_FOLDERS_NAME_PATTERN("BehaviorKey/BATCH_FOLDERS_NAME_PATTERN", "Page %03d%1$1$11", ValueChecker{PLAIN_STR});

const KV BehaviorKey::DIR_FILTER_ON_SWITCH_ENABLE("BehaviorKey/DIR_FILTER_ON_SWITCH_ENABLE",
                                                  int(QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Drives | QDir::Filter::Hidden | QDir::Filter::NoDotAndDotDot),
                                                  ValueChecker{0});

// only work after call SetAutoFlushAllLevel manually
const KV BehaviorKey::ALL_LOG_LEVEL_AUTO_FFLUSH{"BehaviorKey/ALL_LOG_LEVEL_AUTO_FFLUSH", QtMsgType::QtWarningMsg, ValueChecker{PLAIN_INT}};
const KV BehaviorKey::RETURN_ERRORCODE_UPON_ANY_FAILURE{"BehaviorKey/RETURN_ERRORCODE_UPON_ANY_FAILURE", true, ValueChecker{PLAIN_BOOL}};

const KV SearchKey::INCLUDING_SUBDIRECTORIES("SearchKey/INCLUDING_SUBDIRECTORIES", false, ValueChecker{PLAIN_BOOL});
const KV SearchKey::NAME_CASE_SENSITIVE{"SearchKey/NAME_CASE_SENSITIVE", false, ValueChecker{PLAIN_BOOL}};
const KV SearchKey::CONTENTS_CASE_SENSITIVE{"SearchKey/CONTENTS_CASE_SENSITIVE", false, ValueChecker{PLAIN_BOOL}};
const KV SearchKey::GRAY_ENTRIES_DONT_PASS_FILTER{"SearchKey/GRAY_ENTRIES_DONT_PASS_FILTER", false, ValueChecker{PLAIN_BOOL}};
const KV SearchKey::ADVANCE_LINEEDIT_VALUE("SearchKey/ADVANCE_LINEEDIT_VALUE", "", ValueChecker{PLAIN_STR});
const KV SearchKey::ADVANCE_CONTENTS_LINEEDIT_VALUE("SearchKey/ADVANCE_CONTENTS_LINEEDIT_VALUE", "", ValueChecker{PLAIN_STR});
const KV SearchKey::ADVANCE_MODE{"SearchKey/ADVANCE_MODE", 0, ValueChecker{PLAIN_INT}};

const KV CompoVisKey::MENU_RIBBON_CURRENT_TAB_INDEX("CompoVisKey/MENU_RIBBON_CURRENT_TAB_INDEX", 0, ValueChecker{0});
const KV CompoVisKey::FOLDER_PREVIEW_TYPE{"CompoVisKey/FOLDER_PREVIEW_TYPE", 0, ValueChecker{PLAIN_INT}};
const KV CompoVisKey::SHOW_PREVIEW_DOCKER{"CompoVisKey/SHOW_PREVIEW_DOCKER", true, ValueChecker{PLAIN_BOOL}};
const KV CompoVisKey::SHOW_NAVIGATION_SIDEBAR("CompoVisKey/SHOW_NAVIGATION_SIDEBAR", true, ValueChecker{PLAIN_BOOL});
const KV CompoVisKey::SHOW_HAR_IMAGE_PREVIEW{"CompoVisKey/SHOW_HAR_IMAGE_PREVIEW", false, ValueChecker{PLAIN_BOOL}};
const KV CompoVisKey::EXPAND_NAVIGATION_SIDEBAR("CompoVisKey/EXPAND_NAVIGATION_SIDEBAR", false, ValueChecker{PLAIN_BOOL});
const KV CompoVisKey::EXPAND_OFFICE_STYLE_MENUBAR("CompoVisKey/EXPAND_OFFICE_STYLE_MENUBAR", true, ValueChecker{PLAIN_BOOL});

const KV SceneKey::SORT_ORDER_REVERSE{"SceneKey/SORT_ORDER_REVERSE", false, ValueChecker{PLAIN_BOOL}};  // true: reverse, false: not reverse
const KV SceneKey::SORT_BY_ROLE{"SceneKey/SORT_BY_ROLE", 0, ValueChecker{0, 4}};
const KV SceneKey::DISABLE_IMAGE_DECORATION{"SceneKey/DISABLE_IMAGE_DECORATION", false, ValueChecker{PLAIN_BOOL}};
const KV SceneKey::CNT_EACH_PAGE{"SceneKey/CNT_EACH_PAGE", 40, ValueChecker{1, 9999}};

const KV BrowserKey::CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES{"BrowserKey/CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES", true, ValueChecker{PLAIN_BOOL}};
const KV BrowserKey::CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS{"BrowserKey/CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS", true, ValueChecker{PLAIN_BOOL}};
const KV BrowserKey::CLICKABLE_TEXT_BROWSER_FONT_POINT_SIZE{"BrowserKey/CLICKABLE_TEXT_BROWSER_FONT_POINT_SIZE", 12, ValueChecker{PLAIN_INT}};
const KV BrowserKey::FLOATING_IMAGE_VIEW_SHOW{"BrowserKey/FLOATING_IMAGE_VIEW_SHOW", true, ValueChecker{PLAIN_BOOL}};
const KV BrowserKey::FLOATING_VIDEO_VIEW_SHOW{"BrowserKey/FLOATING_VIDEO_VIEW_SHOW", true, ValueChecker{PLAIN_BOOL}};
const KV BrowserKey::FLOATING_OTHER_VIEW_SHOW{"BrowserKey/FLOATING_OTHER_VIEW_SHOW", true, ValueChecker{PLAIN_BOOL}};
const KV BrowserKey::FLOATING_MEDIA_TYPE_SEQ{"BrowserKey/FLOATING_MEDIA_TYPE_SEQ", "012", ValueChecker{PLAIN_STR}};

const KV RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE{"RedunImgFinderKey/ALSO_RECYCLE_EMPTY_IMAGE", true, ValueChecker{PLAIN_BOOL}};
const KV RedunImgFinderKey::RUND_IMG_PATH("RedunImgFinderKey/RUND_IMG_PATH", ".", ValueChecker{FOLDER_PATH}, true);

const KV FavoritesNavigationKey::SORT_BY_ROLE{"FavoritesNavigationKey/SORT_BY_ROLE", 0, ValueChecker{0, 1000}};
const KV FavoritesNavigationKey::SORT_ORDER_REVERSE{"FavoritesNavigationKey/SORT_ORDER_REVERSE", false, ValueChecker{PLAIN_BOOL}};

const KV VideoPlayerKey::VOLUME("VideoPlayer/VOLUME", 100, ValueChecker{0, 100 + 1});
const KV VideoPlayerKey::MUTE("VideoPlayer/MUTE", false, ValueChecker{PLAIN_BOOL});
const KV VideoPlayerKey::PLAYBACK_MODE{"VideoPlayer/PLAYBACK_MODE", 0, ValueChecker{0, 10 + 1}};
const KV VideoPlayerKey::PLAYBACK_TRIGGER_MODE{"VideoPlayer/PLAYBACK_TRIGGER_MODE", 0, ValueChecker{0, 10 + 1}};
const KV VideoPlayerKey::AUTO_HIDE_TOOLBAR{"VideoPlayer/AUTO_HIDE_TOOLBAR", false, ValueChecker{PLAIN_BOOL}};
const KV VideoPlayerKey::DUPLICATE_FINDER_DEVIATION_DURATION("VideoPlayer/DUPLICATE_FINDER_DEVIATION_DURATION",
                                                             2 * 1000,
                                                             ValueChecker{0, 20 * 1000});  // 2s ~ 20s
const KV VideoPlayerKey::DUPLICATE_FINDER_DEVIATION_FILESIZE("VideoPlayer/DUPLICATE_FINDER_DEVIATION_FILESIZE",
                                                             2 * 1024,
                                                             ValueChecker{0, 30 * 1024 * 1024});  // 2kB ~ 30MB
const KV VideoPlayerKey::RATE_MOVIE_DEFAULT_VALUE{"VideoPlayer/RATE_MOVIE_DEFAULT_VALUE", 10, ValueChecker{0, 11}};

const KV RenamerKey::NAME_EXT_INDEPENDENT{"RenamerKey/NAME_EXT_INDEPENDENT", false, ValueChecker{PLAIN_BOOL}};
const KV RenamerKey::INCLUDING_DIR{"RenamerKey/INCLUDING_DIR", false, ValueChecker{PLAIN_BOOL}};
const KV RenamerKey::OLD_STR_LIST{"RenamerKey/OLD_STR_LIST",                                                                        //
                                  QStringList{"", "^(.*?)$", " BB ", " BB",                                                         //
                                              " - 360p", " - 480p", " - 516p", " - 720p", " - 1080p", " - 4K", " - FHD", " - UHD",  //
                                              "([A-Z])", "( s\\d{1,2})"},                                                           //
                                  ValueChecker{QSTRING_LIST}};                                                                      //
const KV RenamerKey::NEW_STR_LIST{"RenamerKey/NEW_STR_LIST",                                                                        //
                                  QStringList{"", " ", " \\1", "\\1 -", " - 1080p"},                                                //
                                  ValueChecker{QSTRING_LIST}};
const KV RenamerKey::INSERT_INDEXES_LIST{"RenamerKey/INSERT_INDEXES_LIST", QStringList{"0", "50", "100", "128", "200"}, ValueChecker{QSTRING_LIST}};
const KV RenamerKey::ARRANGE_SECTION_INDEX{"RenamerKey/ARRANGE_SECTION_INDEX", "1,2", ValueChecker{PLAIN_STR}};
const KV RenamerKey::REGEX_ENABLED{"RenamerKey/REGEX_ENABLED", false, ValueChecker{PLAIN_BOOL}};

const KV RenamerKey::NUMERIAZER_UNIQUE_EXT_COUNTER{"RenamerKey/NUMERIAZER_UNIQUE_EXT_COUNTER", true, ValueChecker{PLAIN_BOOL}};
const KV RenamerKey::NUMERIAZER_NO_FORMAT{"RenamerKey/NUMERIAZER_NO_FORMAT", QStringList{" %1", " - %1", " (%1)"}, ValueChecker{QSTRING_LIST}};
const KV RenamerKey::NUMERIAZER_NO_FORMAT_DEFAULT_INDEX{"RenamerKey/NUMERIAZER_NO_FORMAT_DEFAULT_INDEX", 0, ValueChecker{0, 16}};

namespace JsonOpMemoryKey {
const KV& STUDIO_OPERATION() {
  static const KV studioOperation("STUDIO_OPERATION", 0, ValueChecker{0, 100 + 1});
  return studioOperation;
}
const KV& CAST_OPERATION() {
  static const KV castOperation("CAST_OPERATION", 0, ValueChecker{0, 100 + 1});
  return castOperation;
}
const KV& TAGS_OPERATION() {
  static const KV tagsOperation("TAGS_OPERATION", 0, ValueChecker{0, 100 + 1});
  return tagsOperation;
}
}  // namespace JsonOpMemoryKey
