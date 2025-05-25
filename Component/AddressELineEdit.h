#ifndef ADDRESSELINEEDIT_H
#define ADDRESSELINEEDIT_H

#include "public/PathTool.h"
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
  bool mouseButtonPressedBefore{false};
};

class AddressELineEdit : public QStackedWidget {
  Q_OBJECT
 public:
  explicit AddressELineEdit(QWidget* parent = nullptr);

  inline QString pathFromLineEdit() const {  //
    return PathTool::StripTrailingSlash(PathTool::normPath(pathLineEdit->text()));
  }

  inline QString pathFromFullActions() const {  //
    return pathFromCursorAction(nullptr);
  }

  inline QString pathFromCursorAction(const QAction* cursorAtAction) const {  //
    // if cursor at NOTHING, should return the full path
    QString path;
    for (const QAction* pAct : m_pathActionsTB->actions()) {
      if (pAct == nullptr) {
        continue;
      }
      path += pAct->text();
      if (pAct == cursorAtAction) {
        break;
      }
      path += PathTool::PATH_SEP_CHAR;
    }
    return PathTool::StripTrailingSlash(path);
  }

  inline QString dirname() const {  //
    return PathTool::absolutePath(pathFromLineEdit());
  }

  void onPathActionTriggered(const QAction* cursorAt);
  void onReturnPressed();
  bool ChangePath(const QString& path);
  void onFocusChange(bool hasFocus);
  void clickMode();
  void inputMode();
  void subscribe();

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
  static constexpr int MAX_PATH_SECTIONS_CNT = 260;
  QAction mPathSections[MAX_PATH_SECTIONS_CNT];

  QToolBar* m_pathActionsTB{nullptr};
  QLineEdit* pathLineEdit{nullptr};
  QComboBox* pathComboBox{nullptr};
  QLabel* m_dropPanel{nullptr};
  FocusEventWatch* pathComboBoxFocusWatcher{nullptr};
  static const QString DRAG_HINT_MSG;
  static const QString RELEASE_HINT_MSG;
};
#endif  // ADDRESSELINEEDIT_H
