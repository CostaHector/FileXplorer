#ifndef IMGREORDERDATATYPE_H
#define IMGREORDERDATATYPE_H

#include <QString>

struct ImgReorderDataType {
  ImgReorderDataType(const QString& fullPath_, int oldNumber_): fullPath{fullPath_}, oldNumber{oldNumber_}, newNumber{oldNumber_}{}
  QString fullPath;
  int oldNumber; // don't modify me
  int newNumber;
  ImgReorderDataType& operator+=(int step) {
    newNumber += step;
    return *this;
  }
  QString displayString() const {
    if (newNumber == oldNumber) { return QString::number(newNumber); }
    return QString::asprintf("%d from %d", newNumber, oldNumber);
  }
  bool operator==(const ImgReorderDataType& rhs) const { return newNumber == rhs.newNumber; }
  bool operator!=(const ImgReorderDataType& rhs) const { return !(*this == rhs); }
  bool operator<(const ImgReorderDataType& rhs) const { return newNumber < rhs.newNumber; }
};

using ImgReorderDataLst = QList<ImgReorderDataType>;
#endif  // IMGREORDERDATATYPE_H
