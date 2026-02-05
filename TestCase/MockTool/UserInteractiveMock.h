#ifndef USERINTERACTIVEMOCK_H
#define USERINTERACTIVEMOCK_H

#include <QInputDialog>
#include <QMessageBox>
#include <QSet>
namespace UserInteractiveMock {
using QUESTION_TYPE = QMessageBox::StandardButton (*)(QWidget *, const QString &, const QString&, QMessageBox::StandardButtons, QMessageBox::StandardButton);
using FILE_EXIST_TYPE = bool (*)(const QString&);
using FILE_COPY_TYPE = bool (*)(const QString &fileName, const QString &newName);

namespace InputDialog {

inline std::pair<bool, QString>& getItem_set() {
  static std::pair<bool, QString> ok2returnValue;
  return ok2returnValue;
}

QString invoke_getItem(QWidget *parent,
                       const QString &title,
                       const QString &label,
                       const QStringList &items,
                       int current = 0,
                       bool editable = true,
                       bool *ok = nullptr,
                       Qt::WindowFlags flags = Qt::WindowFlags(),
                       Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
}

inline QSet<QString>& existsSet() {
  static QSet<QString> existsFiles;
  return existsFiles;
}
bool invoke_exists(const QString& filePath);

} // namespace UserInteractiveMock

#endif // USERINTERACTIVEMOCK_H
