#ifndef STRINGEDITHELPER_H
#define STRINGEDITHELPER_H

#include <QLineEdit>
#include <QString>
#include <QTextEdit>

#include <functional>

class StringEditHelper {
 public:
  static QString lowercaseSentense(const QString& sentence) { return sentence.toLower(); };

  static QString capitalizeEachWord(QString sentence) {
    if (not sentence.isEmpty()) {
      sentence[0] = sentence[0].toTitleCase();
    }
    for (int i = 1; i < sentence.size(); ++i) {
      if (sentence[i - 1] == '.' or sentence[i - 1] == ' ') {
        sentence[i] = sentence[i].toTitleCase();
      }
    }
    return sentence;
  };

  static bool ReplaceAndUpdateSelection(QTextEdit* detailEditWidget, std::function<QString(QString)> fTrans);

  static bool ReplaceAndUpdateSelection(QLineEdit* lineEditWidget, std::function<QString(QString)> fTrans);
};

#endif  // STRINGEDITHELPER_H
