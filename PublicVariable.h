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



#endif // PUBLICVARIABLE_H
