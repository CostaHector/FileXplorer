#ifndef ADDRESSELINEEDIT_H
#define ADDRESSELINEEDIT_H

#include <QStackedWidget>
#include <QToolBar>
#include <QAction>
#include <QLineEdit>
#include "PathTool.h"
#include "PathComboBox.h"


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

  inline QString parentPath(bool* isSame=nullptr) const {  //
    const QString& curPath = pathFromLineEdit();
    const QString& parPath = PathTool::absolutePath(curPath);
    if (isSame != nullptr) {
      *isSame = (parPath == curPath);
    }
    return parPath;
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
