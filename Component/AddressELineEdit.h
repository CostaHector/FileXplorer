#ifndef ADDRESSELINEEDIT_H
#define ADDRESSELINEEDIT_H

#include "Tools/PathTool.h"
#include <QComboBox>
#include <QLabel>
#include <QToolBar>
#include <QWidget>

#include <QAction>
#include <QLineEdit>

#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMimeData>

#include <QStackedWidget>

class FocusEventWatch : public QObject {
  Q_OBJECT
 public:
  explicit FocusEventWatch(QObject* parent = nullptr);
  bool eventFilter(QObject* watched, QEvent* event) override;

 signals:
  void focusChanged(bool hasFocus);

 private:
  bool mouseButtonPressedBefore = false;
};

class AddressELineEdit : public QStackedWidget {
  Q_OBJECT
 public:
  explicit AddressELineEdit(QWidget* parent = nullptr);

  inline auto pathFromLineEdit() const -> QString {
    return PATHTOOL::StripTrailingSlash(PATHTOOL::normPath(pathLineEdit->text()));
  }
  inline auto pathFromFullActions() const -> QString {
    return pathFromCursorAction(nullptr);
  }
  inline auto pathFromCursorAction(const QAction* cursorAtAction) const -> QString {
    // if cursor at NOTHING, should return the full path
    QString path;
    for (QAction* action : m_pathActionsTB->actions()) {
      path += action->text() + PATHTOOL::PATH_SEP_CHAR;
      if (action == cursorAtAction) {
        break;
      }
    }
    return PATHTOOL::StripTrailingSlash(path);
  }

  inline auto dirname() const -> QString {
    return PATHTOOL::absolutePath(pathFromLineEdit());
  }

  auto onPathActionTriggered(const QAction* cursorAt) -> void;
  auto onReturnPressed() -> void;

  auto ChangePath(const QString& path) -> bool;

  auto onFocusChange(bool hasFocus) -> void;

  auto clickMode() -> void;
  auto inputMode() -> void;
  auto subscribe() -> void;

  void mousePressEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* e) override;

  void dragEnterEvent(QDragEnterEvent* event) override;

  void dropEvent(QDropEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;

  void dragLeaveEvent(QDragLeaveEvent* event) override;

 signals:
  void pathActionsTriggeredOrLineEditReturnPressed(const QString&);

 public slots:
  void updateAddressToolBarPathActions(const QString& newPath);

 private:
  QToolBar* m_pathActionsTB;
  QLineEdit* pathLineEdit;
  QComboBox* pathComboBox;
  static const QString DRAG_HINT_MSG;
  static const QString RELEASE_HINT_MSG;
  QLabel* m_dropPanel;
  FocusEventWatch* pathComboBoxFocusWatcher;
};
#endif  // ADDRESSELINEEDIT_H
