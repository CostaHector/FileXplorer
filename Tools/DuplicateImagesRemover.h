#ifndef DUPLICATEIMAGESREMOVER_H
#define DUPLICATEIMAGESREMOVER_H

#include <QStringList>

class DuplicateImagesRemover
{
 public:
  DuplicateImagesRemover()=default;
  QStringList GetDuplicateImagesFiles(const QStringList& imgs);
  int operator()(const QString& imgPath);
};

#endif // DUPLICATEIMAGESREMOVER_H
