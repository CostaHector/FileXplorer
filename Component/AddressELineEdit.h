#ifndef ADDRESSELINEEDIT_H
#define ADDRESSELINEEDIT_H

#include "PathTool.h"
#include "PathComboBox.h"
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

class AddressELineEdit : public QStackedWidget {
  Q_OBJECT
 public:
  explicit AddressELineEdit(QWidget* parent = nullptr);
  ~AddressELineEdit();

  inline QString pathFromLineEdit() const {  //
    return PathTool::StripTrailingSlash(PathTool::normPath(m_pathComboBox->currentText()));
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
#ifdef _WIN32
    if (!path.isEmpty()) {
      path = path.mid(1);  // in windows 1st action is a placeholder with icon only
    }
#endif
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

 signals:
  void pathActionsTriggeredOrLineEditReturnPressed(const QString&);

 public slots:
  void updateAddressToolBarPathActions(const QString& newPath);

 private:
  QMetaObject::Connection mFocusChangedConn;
  QToolBar* m_pathActionsTB{nullptr};
  PathComboBox* m_pathComboBox{nullptr};
  static const QString RELEASE_HINT_MSG;
};

#endif  // ADDRESSELINEEDIT_H
