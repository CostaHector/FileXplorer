#ifndef IMGREORDERDATATYPE_H
#define IMGREORDERDATATYPE_H

#include <QString>

struct ImgReorderDataType {
  QString fullPath;
  int number;  // index
  ImgReorderDataType& operator+=(int step) {
    number += step;
    return *this;
  }
  bool operator==(const ImgReorderDataType& rhs) const { return number == rhs.number; }
  bool operator!=(const ImgReorderDataType& rhs) const { return !(*this == rhs); }
  bool operator<(const ImgReorderDataType& rhs) const { return number < rhs.number; }
};

using ImgReorderDataLst = QList<ImgReorderDataType>;
#endif  // IMGREORDERDATATYPE_H
