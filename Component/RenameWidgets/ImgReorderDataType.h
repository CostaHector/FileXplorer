#ifndef IMGREORDERDATATYPE_H
#define IMGREORDERDATATYPE_H

#include <QString>

struct ImgReorderDataType {
  QString fullPath;
  int number;  // index
};

using ImgReorderDataLst = QList<ImgReorderDataType>;
#endif  // IMGREORDERDATATYPE_H
