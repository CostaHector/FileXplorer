#ifndef LOWRESIMGSREMOVER_H
#define LOWRESIMGSREMOVER_H

#include <QStringList>

class LowResImgsRemover {
 public:
  LowResImgsRemover() = default;
  QStringList GetLowResImgsToDel(const QStringList& imgs) const;
  int operator()(const QString& imgPath);
};

#endif  // LOWRESIMGSREMOVER_H
