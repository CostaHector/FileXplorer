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

namespace PathActionHelper {
bool Path2Actions(QToolBar& outTb, const QString& fullpath);
QString PathFromActions(const QList<QAction*>& actions, const QAction* cursorAtAction, bool bStandalize = true);
}  // namespace PathActionHelper

class AddressELineEdit : public QStackedWidget {
  Q_OBJECT
 public:
  explicit AddressELineEdit(QWidget* parent = nullptr);
  ~AddressELineEdit();

  static QString NormToolBarActionPath(QString actionPath);

  inline QString pathFromLineEdit() const {  //
    return PathTool::normPath(m_pathComboBox->currentText());
  }

  inline QString pathFromFullActions() const {  //
    return PathActionHelper::PathFromActions(m_pathActionsTB->actions(), nullptr, true);
  }

  inline QString pathFromCursorAction(const QAction* cursorAtAction) const {  //
    return PathActionHelper::PathFromActions(m_pathActionsTB->actions(), cursorAtAction, true);
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
