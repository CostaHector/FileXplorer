#ifndef PUBLICTOOL_H
#define PUBLICTOOL_H

#include <QActionGroup>
#include <QLayout>
#include <QString>
#include <QTranslator>
#include <QFile>

QString MoveToNewPathAutoUpdateActionText(const QString& first_path, QActionGroup* oldAG);
QString TextReader(const QString& textPath);
bool TextWriter(const QString& fileName, const QString& content = "", const QIODevice::OpenMode openMode = QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
bool ByteArrayWriter(const QString& fileName, const QByteArray& ba);
void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align);
QString ChooseCopyDestination(QString defaultPath, QWidget* parent = nullptr);

inline void ImgsSortFileSizeFirst(QStringList& imgs) {
  static const auto FILE_SIZE_SORTER = [](const QString& lhs, const QString& rhs) -> bool {  //
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
  };
  std::sort(imgs.begin(), imgs.end(), FILE_SIZE_SORTER);
}

inline void ImgsSortNameLengthFirst(QStringList& imgs) {
  static const auto NAME_LENGTH_SORTER = [](const QString& lhs, const QString& rhs) -> bool {  //
    const auto left{lhs.size()};
    const auto right{rhs.size()};
    return left < right || (left == right && lhs < rhs);  //
  };
  std::sort(imgs.begin(), imgs.end(), NAME_LENGTH_SORTER);
}

void LoadCNLanguagePack(QTranslator& translator);
void LoadSysLanaguagePack(QTranslator& translator);

#endif  // PUBLICTOOL_H
