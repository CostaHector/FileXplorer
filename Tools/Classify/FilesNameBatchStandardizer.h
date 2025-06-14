#ifndef FILESNAMEBATCHSTANDARDIZER_H
#define FILESNAMEBATCHSTANDARDIZER_H

#include "Tools/NameStandardizer.h"
#include <QString>

class FilesNameBatchStandardizer {
 public:
  FilesNameBatchStandardizer() = default;
  bool operator()(const QString& rootPath);

  NameStandardizer ns;
};

#endif  // FILESNAMEBATCHSTANDARDIZER_H
