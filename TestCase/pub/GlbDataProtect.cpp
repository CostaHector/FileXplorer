#include "GlbDataProtect.h"

template class GlbDataProtect<unsigned int>;
template class GlbDataProtect<int>;
template class GlbDataProtect<long long>;
template class GlbDataProtect<long>;
template class GlbDataProtect<char>;
template class GlbDataProtect<short>;
template class GlbDataProtect<QString>;
extern template class GlbDataProtect<QVariant>;
template class GlbDataProtect<QStringList>;
template class GlbDataProtect<QList<int>>;
template class GlbDataProtect<QSet<QString>>;
template class GlbDataProtect<QMap<QString, QString>>;
