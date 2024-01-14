#ifndef NAMESTANDARDIZER_H
#define NAMESTANDARDIZER_H

#include <QString>
#include <QMap>


class NameStandardizer
{
 public:
  auto operator()(QString aFileName)->QString;
};

#endif // NAMESTANDARDIZER_H
