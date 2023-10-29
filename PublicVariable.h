#ifndef PUBLICVARIABLE_H
#define PUBLICVARIABLE_H

#include <QFileInfo>
#include <QDir>
#include <QRect>
#include <QSettings>
#include <QSize>
#include <QTextStream>
#include <qDebug>

const QRect DEFAULT_GEOMETRY(0, 0, 1024, 768);
const QSize DOCKER_DEFAULT_SIZE(DEFAULT_GEOMETRY.width() / 2, DEFAULT_GEOMETRY.height());

static inline QSettings& PreferenceSettings() {
  static QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Costa", "FileExplorerReadOnly");
  return settings;
}

auto TextReader(const QString& textPath) -> QString;

typedef std::function<bool(QString, bool, bool)> T_IntoNewPath;
typedef std::function<bool(QString)> T_on_searchTextChanged;
typedef std::function<bool(QString)> T_on_searchEnterKey;
typedef std::function<void()> T_HotUpdate;


namespace MainKey {
constexpr int Name = 0;
constexpr int Size = 1;
constexpr int Type = 2;
constexpr int DateModified = 3;
const QStringList EXPLORER_COLUMNS_TITLE{"Name", "Size", "Type", "DateModified"};

const int NAME_COLUMN = EXPLORER_COLUMNS_TITLE.indexOf("Name");
const int TYPE_COLUMN = EXPLORER_COLUMNS_TITLE.indexOf("Type");
const int EXPLORER_COLUMNS_COUNT = EXPLORER_COLUMNS_TITLE.size();
}  // namespace MainKey

namespace DB_HEADER_KEY{
const QString Name = "Name";
const QString Size = "Size";
const QString Type = "Type";
const QString DateModified = "DateModified";

const QString Performers = "Performers";
const QString Tags = "Tags";
const QString Rate = "Rate";
const QString Driver = "Driver";
const QString Prepath = "Prepath";
const QString Extra = "Extra";

const QStringList DB_HEADER{Name, Size, Type, DateModified, Performers, Tags, Rate, Driver, Prepath, Extra};
const int DB_DRIVER_INDEX = DB_HEADER.indexOf(Driver);
const int DB_PREPATH_INDEX = DB_HEADER.indexOf(Prepath);
const int DB_NAME_INDEX = DB_HEADER.indexOf(Name);
const int DB_SIZE_COLUMN = DB_HEADER.indexOf(Size);
const int DB_TYPE_INDEX = DB_HEADER.indexOf(Type);
}

namespace HEADERVIEW_SORT_INDICATOR_ORDER {
class OrderClass {
 public:
  QString name;
  Qt::SortOrder value;
};
const OrderClass AscendingOrder{"AscendingOrder", Qt::SortOrder::AscendingOrder};
const OrderClass DescendingOrder{"DescendingOrder", Qt::SortOrder::DescendingOrder};
const QMap<QString, Qt::SortOrder> string2SortOrderEnumListTable = {{AscendingOrder.name, AscendingOrder.value},
                                                                    {DescendingOrder.name, DescendingOrder.value}};
const QStringList HEADERVIEW_SORT_INDICATOR_ORDER_STR = string2SortOrderEnumListTable.keys();
QString SortOrderEnum2String(const Qt::SortOrder orderEnum);
}  // namespace HEADERVIEW_SORT_INDICATOR_ORDER

class GVar {
 public:
  explicit GVar(const QString& name_, const QVariant& v_) : name(name_), v(v_){};
  virtual bool checker() = 0;
  QString name;
  QVariant v;
};

class GVarBool : public GVar {
 public:
  explicit GVarBool(const QString& name_, bool v_) : GVar(name_, v_) {}

  bool checker() override { return true; }
};

class GVarInt : public GVar {
 public:
  explicit GVarInt(const QString& name_, int v_, int minV_ = INT32_MIN, int maxV_ = INT32_MAX) : GVar(name_, v_), minV(minV_), maxV(maxV_) {}

  bool checker() override { return minV <= v.toInt() and v.toInt() < maxV; }
  int minV;
  int maxV;
};

class GVarStr : public GVar {
 public:
  explicit GVarStr(const QString& name_, QString v_, const QStringList& candidatePool_ = {}) : GVar(name_, v_), candidatePool(candidatePool_) {}

  bool checker() override { return candidatePool.isEmpty() or candidatePool.contains(v.toString()); }
  QStringList candidatePool;
};

class GVarStrFile : public GVar {
 public:
  explicit GVarStrFile(const QString& name_, QString v_, const QStringList& candidateSuffixPool_ = {"exe"})
      : GVar(name_, v_), candidateSuffixPool(candidateSuffixPool_) {}

  bool checker() override {
    QFileInfo fi(v.toString());
    return fi.isFile() and (candidateSuffixPool.isEmpty() or candidateSuffixPool.contains(fi.suffix()));
  }
  QStringList candidateSuffixPool;
};

class GVarStrFolder : public GVar {
 public:
  explicit GVarStrFolder(const QString& name_, QString v_) : GVar(name_, v_) {}

  bool checker() override { return QFileInfo(v.toString()).isDir(); }
};

class GVarListStr : public GVar {
 public:
  explicit GVarListStr(const QString& name_, QStringList v_) : GVar(name_, v_) {}

  bool checker() override { return true; }
};

constexpr char MOVE_COPT_TO_PATH_STR_SEPERATOR = '\n';

namespace MemoryKey {
const GVarStrFile BACKGROUND_IMAGE("BACKGROUND_IMAGE", "");
const GVarBool SHOW_BACKGOUND_IMAGE("SHOW_BACKGOUND_IMAGE", false);
const GVarStrFolder PATH_LAST_TIME_COPY_TO("PATH_LAST_TIME_COPY_TO", "");
const GVarBool SHOW_FOLDER_PREVIEW_HTML("SHOW_FOLDER_PREVIEW_HTML", true);
const GVarBool SHOW_FOLDER_PREVIEW_WIDGET("SHOW_FOLDER_PREVIEW_WIDGET", true);
const GVarBool SHOW_FOLDER_PREVIEW_IMAGE("SHOW_FOLDER_PREVIEW_IMAGE", false);
const GVarBool SHOW_FOLDER_PREVIEW_JSON_EDITOR("SHOW_FOLDER_PREVIEW_JSON_EDITOR", false);

const GVarBool SHOW_QUICK_NAVIGATION_TOOL_BAR("SHOW_QUICK_NAVIGATION_TOOL_BAR", true);
const GVarBool SHOW_FRAMELESS_WINDOW("SHOW_FRAMELESS_WINDOW", true);
const GVarBool EXPAND_OFFICE_STYLE_MENUBAR("EXPAND_OFFICE_STYLE_MENUBAR", true);
const GVarBool SHOW_DATABASE("SHOW_DATABASE", false);

const GVarInt NAME_COLUMN_WIDTH("NAME_COLUMN_WIDTH", 400, 0, 2048);
const GVarInt HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX("HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX", MainKey::Name, 0);
const GVarStr HEARVIEW_SORT_INDICATOR_ORDER("HEARVIEW_SORT_INDICATOR_ORDER",
                                            HEADERVIEW_SORT_INDICATOR_ORDER::AscendingOrder.name,
                                            HEADERVIEW_SORT_INDICATOR_ORDER::HEADERVIEW_SORT_INDICATOR_ORDER_STR);
const GVarInt ITEM_VIEW_FONT_SIZE("ITEM_VIEW_FONT_SIZE", 12, 8, 25);
const GVarStr DEFAULT_NEW_CHOICE("DEFAULT_NEW_CHOICE", "New folder", {"New folder", "New text", "New json", "New folder html"});
const GVarStr DEFAULT_COPY_CHOICE("DEFAULT_COPY_CHOICE", "Copy fullpath", {"Copy fullpath", "Copy path", "Copy name", "Copy the path"});
const GVarStr DEFAULT_RENAME_CHOICE("DEFAULT_RENAME_CHOICE",
                                    "Rename (ith)",
                                    {"Rename (ith)", "swap 1-2-3 to 1-3-2", "Case", "Str Inserter", "Str Deleter", "Str Replacer"});
const GVarStr MOVE_TO_PATH_HISTORY("MOVE_TO_PATH_HISTORY", ".\n..\n\\", {});
const GVarStr COPY_TO_PATH_HISTORY("COPY_TO_PATH_HISTORY", ".\n..\n\\", {});

const GVarInt MENU_RIBBON_CURRENT_TAB_INDEX("MENU_RIBBON_CURRENT_TAB_INDEX", 0, 0);
}  // namespace MemoryKey

#include <QDir>
namespace SystemPath {
const QString drivePath = "";
const QString desktopPath = QDir(QDir::homePath()).absoluteFilePath("Desktop");
const QString documentPath = QDir(QDir::homePath()).absoluteFilePath("Documents");
const QString downloadPath = QDir(QDir::homePath()).absoluteFilePath("Downloads");
const QString musicPath = QDir(QDir::homePath()).absoluteFilePath("Music");
const QString picturesPath = QDir(QDir::homePath()).absoluteFilePath("Pictures");
const QString videosPath = QDir(QDir::homePath()).absoluteFilePath("Videos");
const QString starredPath = QDir(QDir::homePath()).absoluteFilePath("Documents");
const QString FILE_INFO_DATABASE = QDir(QDir::homePath()).absoluteFilePath("FILE_INFO_DATABASE");
}  // namespace SystemPath
const QString TABLE_NAME = "movies_info";

extern const char* SUBMIT_BTN_STYLE;

constexpr int TABS_ICON_IN_MENU_3x1 = 16;
constexpr int TABS_ICON_IN_MENU_2x1 = 24;
constexpr int TABS_ICON_IN_MENU_1x1 = 48;

namespace TYPE_FILTER {
const QStringList VIDEO_TYPE_SET = {"*.mp4", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.ts", "*.vob"};
const QStringList IMAGE_TYPE_SET = {"*.jpeg", "*.jpg", "*.png", "*.tiff", "*.jfif", "*.gif", "*.webp", "*.ico", "*.svg"};
const QStringList JSON_TYPE_SET = {"*.json"};
const QStringList TEXT_TYPE_SET = {"*.json", "*.txt", "*.html", "*.md", "*.dat"};
}  // namespace TYPE_FILTER

enum class CCMMode {
  ERROR = -1,
  MERGE = 0,
  COPY = 1,
  CUT = 2,
  LINK = 3,
};

static const QMap<CCMMode, QString> CCMMode2QString = {{CCMMode::MERGE, "MERGE"},
                                                       {CCMMode::COPY, "COPY"},
                                                       {CCMMode::CUT, "CUT"},
                                                       {CCMMode::LINK, "LINK"}};
#endif  // PUBLICVARIABLE_H
