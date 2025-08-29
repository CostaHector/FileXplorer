#ifndef STRINGTOOL_H
#define STRINGTOOL_H

#include <QStringList>

namespace StringTool {
extern const QRegularExpression IMG_VID_SEP_COMP;
constexpr char PERFS_VIDS_IMGS_SPLIT_CHAR {'\n'};

bool FileNameLengthSorter(const QString& lhs, const QString& rhs);
bool FileSizeSorter(const QString& lhs, const QString& rhs);

QStringList GetImgsVidsListFromField(const QString& imgVidsField);

inline void ImgsSortFileSizeFirst(QStringList& imgs) {
  std::sort(imgs.begin(), imgs.end(), FileSizeSorter);
}
inline void ImgsSortNameLengthFirst(QStringList& imgs) {
  std::sort(imgs.begin(), imgs.end(), FileNameLengthSorter);
}
inline void ImgsSortNameFirst(QStringList& imgs) {
  std::sort(imgs.begin(), imgs.end());
}

int RemoveDuplicateKeepSequence(QStringList& hists);
int TrimEachElementAndRemoveEmpty(QStringList& keywords);
void SearchHistoryListProc(QStringList& hists);
}

#endif // STRINGTOOL_H
