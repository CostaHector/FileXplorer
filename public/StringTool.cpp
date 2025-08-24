#include "StringTool.h"
#include <QRegularExpression>
#include <QFile>

namespace StringTool{
const QRegularExpression IMG_VID_SEP_COMP{"\\||\r\n|\n"};
bool ImgHumanSorter(const QString& lhs, const QString& rhs) {
  if (lhs.size() != rhs.size()) {
    return lhs.size() < rhs.size();
  }
  return lhs < rhs;
}

bool ImgLengthSorter(const QString& lhs, const QString& rhs) {  //
  const auto left{QFile{lhs}.size()};
  const auto right{QFile{rhs}.size()};
  if (left < right) {  // size first
    return true;
  } else if (left == right) {
    if (lhs.size() < rhs.size()) {  // length second
      return true;
    } else if (lhs.size() == rhs.size()) {
      return lhs < rhs;  // name ascii third
    }
  }
  return false;
}

QStringList InitImgsList(const QString& imgs) {
  // images human sort 0 < 1 < ... < 9 < 10. not in alphabeit
  QStringList imgsLst = imgs.split(IMG_VID_SEP_COMP);
  std::sort(imgsLst.begin(), imgsLst.end(), ImgHumanSorter);
  return imgsLst;
}

}
