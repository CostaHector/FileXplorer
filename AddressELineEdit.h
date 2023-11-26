#ifndef ADDRESSELINEEDIT_H
#define ADDRESSELINEEDIT_H

#include <QComboBox>
#include <QToolBar>
#include <QWidget>

#include <QAction>
#include <QLineEdit>

#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMimeData>

#include <QStackedWidget>

constexpr int CONTROL_HEIGHT = 28;

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

  inline auto text() const -> QString { return pathLineEdit->text(); }
  inline auto textFromActions() const -> QString {
    QString path;
    for (QAction* action : m_pathActionsTB->actions()) {
      path += action->text() + PATH_SEP_CHAR;
    }
    return PathProcess(path);
  }
  inline auto textFromCurrentCursor(const QAction* cursorAtAction) const -> QString {
    QString path;
    for (QAction* action : m_pathActionsTB->actions()) {
      path += action->text() + PATH_SEP_CHAR;
      if (action == cursorAtAction) {
        break;
      }
    }
    return PathProcess(path);
  }
  auto onPathActionTriggered(const QAction* clkAct) -> void {
    const QString& rawPath = textFromCurrentCursor(clkAct);
    const QString& fullPth = AddressELineEdit::PathProcess(rawPath);
    qDebug("path clicked [%s]", fullPth.toStdString().c_str());
    onReturnPressed(fullPth);
  }
  auto onReturnPressed(const QString& path) -> bool;

  inline auto dirname() const -> QString {
    const QFileInfo fi(textFromActions());
    return fi.isRoot() ? "" : fi.absolutePath();
  }

  auto onFocusChange(bool hasFocus) -> void;

  auto clickMode() -> void;
  auto inputMode() -> void;
  auto subscribe() -> void;

  void mousePressEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* e) override;

  void dragEnterEvent(QDragEnterEvent* event) override;

  void dropEvent(QDropEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;

 signals:
  void intoAPath_active(const QString&);

 public slots:
  void pathChangeTo(const QString& newPath);

 private:
  inline auto PathProcess(const QString& path) const -> QString {
    // drive letter will be kept while trailing path seperator will be trunc
    // i.e., "XX:/" -> "XX:/" and "/home/user/" ->"/home/user"
    return (path.size() > 2 and path[path.size() - 2] != ':' and path.back() == PATH_SEP_CHAR) ? path.chopped(1) : path;
  }
  static constexpr char PATH_SEP_CHAR = '/';
  QToolBar* m_pathActionsTB;

  QLineEdit* pathLineEdit;
  QComboBox* pathComboBox;
  FocusEventWatch* pathComboBoxFocusWatcher;
};

class TestAddressELineEdit : public QWidget {
  Q_OBJECT
 public:
  explicit TestAddressELineEdit(QWidget* parent = nullptr) : QWidget(parent) {
    AddressELineEdit* add = new AddressELineEdit;

    QLineEdit* searchLe = new QLineEdit("Search here");
    searchLe->setClearButtonEnabled(true);

    searchLe->addAction(QIcon(":/themes/SEARCH"), QLineEdit::LeadingPosition);
    searchLe->setPlaceholderText("Search...");

    QHBoxLayout* lo = new QHBoxLayout;
    lo->addWidget(add);
    lo->addWidget(searchLe);
    setLayout(lo);

    add->setMinimumWidth(400);
    searchLe->setMinimumWidth(100);

    add->pathChangeTo(QFileInfo(__FILE__).absolutePath());
  }
 signals:
};

#endif  // ADDRESSELINEEDIT_H
