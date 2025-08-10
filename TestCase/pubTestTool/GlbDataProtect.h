#ifndef GLBDATAPROTECT_H
#define GLBDATAPROTECT_H

#include <QStringList>
#include <QSet>
#include <QMap>
#include <QVariant>
template <typename T>
class GlbDataProtect {
 public:
  GlbDataProtect(T& srvValue) : m_srcValue{srvValue}, m_srcObj{srvValue} {}
  ~GlbDataProtect() { m_srcObj = m_srcValue; }

 private:
  T m_srcValue;
  T& m_srcObj;
};

extern template class GlbDataProtect<unsigned int>;
extern template class GlbDataProtect<int>;
extern template class GlbDataProtect<long long>;
extern template class GlbDataProtect<long>;
extern template class GlbDataProtect<char>;
extern template class GlbDataProtect<short>;
extern template class GlbDataProtect<QString>;
extern template class GlbDataProtect<QVariant>;
extern template class GlbDataProtect<QStringList>;
extern template class GlbDataProtect<QList<int>>;
extern template class GlbDataProtect<QSet<QString>>;
extern template class GlbDataProtect<QMap<QString, QString>>;
#endif  // GLBDATAPROTECT_H
