#ifndef PUBLICVARIABLE_H
#define PUBLICVARIABLE_H

#include <QSettings>
#include <QFileInfo>
#include <QTextStream>
#include <QSize>
#include <QRect>

const QRect DEFAULT_GEOMETRY(0, 0, 1024, 768);
const QSize DOCKER_DEFAULT_SIZE(DEFAULT_GEOMETRY.width()/2, DEFAULT_GEOMETRY.height());

static inline QSettings& PreferenceSettings(){
    static QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Costa", "FileExplorerReadOnly");
    return settings;
}

auto TextReader(const QString& textPath) -> QString;

typedef std::function<bool(QString, bool, bool)> T_IntoNewPath;
typedef std::function<bool(QString)> T_on_searchTextChanged;
typedef std::function<bool(QString)> T_on_searchEnterKey;



namespace MainKey{
constexpr int Name = 0;
constexpr int Size = 1;
constexpr int Type = 2;
constexpr int DateModified = 3;
const QStringList EXPLORER_COLUMNS_TITLE{"Name", "Size", "Type", "DateModified"};

const int NAME_COLUMN = EXPLORER_COLUMNS_TITLE.indexOf("Name");
const int TYPE_COLUMN = EXPLORER_COLUMNS_TITLE.indexOf("Type");
const int EXPLORER_COLUMNS_COUNT = EXPLORER_COLUMNS_TITLE.size();
}


namespace HEADERVIEW_SORT_INDICATOR_ORDER{
class OrderClass{
public:
    QString name;
    Qt::SortOrder value;
};
const OrderClass AscendingOrder{"AscendingOrder", Qt::SortOrder::AscendingOrder};
const OrderClass DescendingOrder{"DescendingOrder", Qt::SortOrder::DescendingOrder};
const QMap<QString, Qt::SortOrder> string2SortOrderEnumListTable={{AscendingOrder.name,AscendingOrder.value},
                                                                    {DescendingOrder.name,DescendingOrder.value}};
const QStringList HEADERVIEW_SORT_INDICATOR_ORDER_STR = string2SortOrderEnumListTable.keys();
QString SortOrderEnum2String(const Qt::SortOrder orderEnum);
}


class GVar{
public:
    explicit GVar(const QString& name_, const QVariant& v_): name(name_), v(v_){};
    virtual bool checker() = 0;
    QVariant v;
    QString name;
};

class GVarBool:public GVar{
public:
    explicit GVarBool(const QString& name_, bool v_):
        GVar(name_, v_){
    }

    bool checker() override{
        return true;
    }
};

class GVarInt:public GVar{
public:
    explicit GVarInt(const QString& name_, int v_, int minV_=INT32_MIN, int maxV_=INT32_MAX):
        GVar(name_, v_), minV(minV_), maxV(maxV_){
    }

    bool checker() override{
        return minV <= v.toInt() and v.toInt() < maxV;
    }
    int minV;
    int maxV;
};

class GVarStr:public GVar{
public:
    explicit GVarStr(const QString& name_, QString v_, const QStringList& candidatePool_={}):
        GVar(name_, v_), candidatePool(candidatePool_){
    }

    bool checker() override{
        return candidatePool.isEmpty() or candidatePool.contains(v.toString());
    }
    QStringList candidatePool;
};

class GVarStrFile:public GVar{
public:
    explicit GVarStrFile(const QString& name_, QString v_, const QStringList& candidateSuffixPool_={"exe"}):
        GVar(name_, v_), candidateSuffixPool(candidateSuffixPool_){
    }

    bool checker() override{
        QFileInfo fi(v.toString());
        return fi.isFile() and (candidateSuffixPool.isEmpty() or candidateSuffixPool.contains(fi.suffix()));
    }
    QStringList candidateSuffixPool;
};

class GVarStrFolder:public GVar{
public:
    explicit GVarStrFolder(const QString& name_, QString v_):
        GVar(name_, v_){
    }

    bool checker() override{
        return QFileInfo(v.toString()).isDir();
    }
};

class GVarListStr:public GVar{
public:
    explicit GVarListStr(const QString& name_, QStringList v_):
        GVar(name_, v_){
    }

    bool checker() override{
        return true;
    }
};

namespace MemoryKey {
const GVarStrFile BACKGROUND_IMAGE("BACKGROUND_IMAGE", "");
const GVarBool SHOW_BACKGOUND_IMAGE("SHOW_BACKGOUND_IMAGE", false);
const GVarStrFolder PATH_LAST_TIME_COPY_TO("PATH_LAST_TIME_COPY_TO", "");
const GVarBool SHOW_FOLDER_PREVIEW_HTML("SHOW_FOLDER_PREVIEW_HTML", true);
const GVarBool SHOW_FOLDER_PREVIEW_WIDGET("SHOW_FOLDER_PREVIEW_WIDGET", true);
const GVarBool SHOW_FOLDER_PREVIEW_IMAGE("SHOW_FOLDER_PREVIEW_IMAGE", false);
const GVarBool SHOW_FOLDER_PREVIEW_JSON_EDITOR("SHOW_FOLDER_PREVIEW_JSON_EDITOR", false);
const GVarInt NAME_COLUMN_WIDTH("NAME_COLUMN_WIDTH", 400, 0, 2048);
const GVarInt HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX("HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX", MainKey::Name, 0);
const GVarStr HEARVIEW_SORT_INDICATOR_ORDER("HEARVIEW_SORT_INDICATOR_ORDER",
                                            HEADERVIEW_SORT_INDICATOR_ORDER::AscendingOrder.name,
                                            HEADERVIEW_SORT_INDICATOR_ORDER::HEADERVIEW_SORT_INDICATOR_ORDER_STR);
const GVarInt ITEM_VIEW_FONT_SIZE("ITEM_VIEW_FONT_SIZE", 12, 8, 25);
}


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

}

extern const char* SUBMIT_BTN_STYLE;


#endif // PUBLICVARIABLE_H
