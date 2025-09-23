#include "PublicVariable.h"
#include <QDir>

namespace SystemPath{
const QString& HOME_PATH() {
    static const QString path = QDir::homePath();
    return path;
}
}
