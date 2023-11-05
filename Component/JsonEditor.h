#ifndef JSONEDITOR_H
#define JSONEDITOR_H

#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QRegExp>
#include <QTextEdit>
#include <QWidget>
#include <QMenu>

const QRegExp sepComp(" and | & | , |,\r\n|, | ,|& | &|; | ;|\r\n|,\n|\n|,|;|&", Qt::CaseInsensitive);

#include <QDir>
#include <QFile>

#include "PublicVariable.h"

#include <QVariantMap>

class JsonEditor : public QMainWindow {
 public:
  explicit JsonEditor(QWidget* parent = nullptr);

  auto getListPanelRightClickMenu() -> QMenu*;

  auto hasLast() const -> bool;
  auto last() -> void;
  auto hasNext() const -> bool;
  auto next() -> void;
  auto onAutoSkip() -> void;
  void autoNext();
  auto load(const QString& path) -> bool;
  auto refreshEditPanel() -> void;

  auto currentJsonString() const -> QString;

  auto subscribe() -> void;

  auto onStageChanges() -> bool;
  auto onResetChanges() -> bool;
  auto onSubmitAllChanges() -> bool;

  auto onCapitalizeEachWord()->void;
  auto onLearnPerfomersFromJsonFile() -> bool;
  auto onPerformersHint() -> QStringList;

  auto sizeHint() const -> QSize override { return QSize(1024, 768); }
 signals:

 private:
  static inline auto getBackupJsonFile(const QString& origin) -> QString { return origin + ".bkp"; }

  auto formatter() -> bool;

  QListWidget* jsonListPanel;
  QFormLayout* editorPanel;

  QMenu* jsonListPanelMenu;

  static const QMap<QString, QString> key2ValueType;
  static const QString TITLE_TEMPLATE;

  static const QColor MEET_CONDITION_COLOR;
  static const QColor NOT_MEET_CONDITION_COLOR;
};

#endif  // JSONEDITOR_H
