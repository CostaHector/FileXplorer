#ifndef STRINGTOOL_H
#define STRINGTOOL_H

#include <QStringList>

namespace StringTool {
extern const QRegularExpression IMG_VID_SEP_COMP;
constexpr char PERFS_VIDS_IMGS_SPLIT_CHAR {'\n'};

bool ImgHumanSorter(const QString& lhs, const QString& rhs);
bool ImgLengthSorter(const QString& lhs, const QString& rhs);

QStringList InitImgsList(const QString& imgs);

inline void ImgsSortFileSizeFirst(QStringList& imgs) {
  std::sort(imgs.begin(), imgs.end(), ImgLengthSorter);
}
inline void ImgsSortNameLengthFirst(QStringList& imgs) {
  std::sort(imgs.begin(), imgs.end(), ImgHumanSorter);
}
}

#endif // STRINGTOOL_H
