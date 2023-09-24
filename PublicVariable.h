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


class GVar{
public:
    explicit GVar(QVariant v_): v(v_){
    };
    virtual bool checker() = 0;
    QVariant v;
};

class GVarBool:public GVar{
public:
    explicit GVarBool(bool v_):
        GVar(v_){
    }

    bool checker() override{
        return true;
    }
};

class GVarInt:public GVar{
public:
    explicit GVarInt(int v_, int minV_=INT32_MIN, int maxV_=INT32_MAX):
        GVar(v_), minV(minV_), maxV(maxV_){
    }

    bool checker() override{
        return minV <= v.toInt() and v.toInt() < maxV;
    }
    int minV;
    int maxV;
};

class GVarStr:public GVar{
public:
    explicit GVarStr(QString v_, const QStringList& candidatePool_={}):
        GVar(v_), candidatePool(candidatePool_){
    }

    bool checker() override{
        return candidatePool.isEmpty() or candidatePool.contains(v.toString());
    }
    QStringList candidatePool;
};

class GVarStrFile:public GVar{
public:
    explicit GVarStrFile(QString v_, const QStringList& candidateSuffixPool_={"exe"}):
        GVar(v_), candidateSuffixPool(candidateSuffixPool_){
    }

    bool checker() override{
        QFileInfo fi(v.toString());
        return fi.isFile() and (candidateSuffixPool.isEmpty() or candidateSuffixPool.contains(fi.suffix()));
    }
    QStringList candidateSuffixPool;
};

class GVarStrFolder:public GVar{
public:
    explicit GVarStrFolder(QString v_):
        GVar(v_){
    }

    bool checker() override{
        return QFileInfo(v.toString()).isDir();
    }
};

class GVarListStr:public GVar{
public:
    explicit GVarListStr(QStringList v_):
        GVar(v_){
    }

    bool checker() override{
        return true;
    }
};


namespace MemoryKey {
extern GVarStrFile BACKGROUND_IMAGE;
extern GVarBool SHOW_BACKGOUND_IMAGE;
extern GVarStrFolder PATH_LAST_TIME_COPY_TO;
extern GVarBool SHOW_FOLDER_PREVIEW_HTML;
extern GVarBool SHOW_FOLDER_PREVIEW_WIDGET;
extern GVarBool SHOW_FOLDER_PREVIEW_IMAGE;
extern GVarBool SHOW_FOLDER_PREVIEW_JSON_EDITOR;
}
#endif // PUBLICVARIABLE_H
