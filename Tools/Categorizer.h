#ifndef CATEGORIZER_H
#define CATEGORIZER_H

#include <QRegularExpression>

// support index format scene 1, sc. 1, sc.1, sc 1, sc1, and part 1, pt. 1, pt.1, pt 1, pt1, p1
const QRegularExpression VID_SCENE_NUM_PAT{"^(.*?) (- )?sc(ene |\\. |\\.| |)\\d{1,2}$", QRegularExpression::CaseInsensitiveOption};
const QRegularExpression VID_PART_NUM_PAT{"^(.*?) (- )?p(art |t\\. |t\\.|t |t|)\\d{1,2}$", QRegularExpression::CaseInsensitiveOption};
const QRegularExpression VID_NUM_PAT{"^(.*?) (- )?\\d{1,2}$"};
// Given "Name - 1", (.*) - \d{1, 2} will catch [1] = "Name - ";
// Given "Name - abc - 1", (.*?) - \d{1, 2} will catch [1] = "Name - abc";

const QRegularExpression IMG_SCENE_NUM_PAT{"^(.*?) (- )?sc(ene |\\. |\\.| |)\\d{1,2}( - \\d{1,2})?$", QRegularExpression::CaseInsensitiveOption};
const QRegularExpression IMG_PART_NUM_PAT{"^(.*?) (- )?p(art |t\\. |t\\.|t |t|)\\d{1,2}( - \\d{1,2})?$", QRegularExpression::CaseInsensitiveOption};
const QRegularExpression IMG_NUM_PAT{"^(.*?) (- )?\\d{1,2}( - \\d{1,2})?$"};
// scene can be scene/sc/sc./part/pt/pt.
// for vid, txt
// Name n
// Name - n
// Name scene n
// Name - scene n
// Name
// ===> Name

// for only images
// Name n
// Name n - n
// Name - n
// Name - n - n
// Name scene n
// Name scene n - n
// Name - scene n
// Name - scene n - n
// Name
// ===> Name

class Categorizer {
 public:
  static auto ImgCoreName(const QString& nm) -> QString {
    auto extDotIndex = nm.lastIndexOf('.');
    if (extDotIndex == -1) {
      return nm;
    }
    const QString& p = nm.left(extDotIndex);
    QRegularExpressionMatch result;
    if ((result = IMG_SCENE_NUM_PAT.match(p)).hasMatch()) {
    } else if ((result = IMG_PART_NUM_PAT.match(p)).hasMatch()) {
    } else if ((result = IMG_NUM_PAT.match(p)).hasMatch()) {
    } else {
      return p;
    }
    return result.captured(1);
  }

  static auto VidCoreName(const QString& nm) -> QString {
    auto extDotIndex = nm.lastIndexOf('.');
    if (extDotIndex == -1) {
      return nm;
    }
    const QString& p = nm.left(extDotIndex);
    QRegularExpressionMatch result;
    if ((result = VID_SCENE_NUM_PAT.match(p)).hasMatch()) {
    } else if ((result = VID_PART_NUM_PAT.match(p)).hasMatch()) {
    } else if ((result = VID_NUM_PAT.match(p)).hasMatch()) {
    } else {
      return p;
    }
    return result.captured(1);
  }

  auto operator()(const QString& rootDir) const -> bool;
 private:
  static auto Classify(const QString& rootDir) -> QMap<QString, QStringList>;
};

#endif  // CATEGORIZER_H
