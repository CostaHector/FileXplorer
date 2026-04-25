#ifndef CUSTOMTABLEVIEW_H
#define CUSTOMTABLEVIEW_H

#include <QTableView>
#include "AddableMenu.h"

class VerMenuInHeader;
class DoubleRowHeader;
class ScrollBarPolicyMenu;
class TextElideModeMenu;

class CustomTableView : public QTableView {
  Q_OBJECT
 public:
  explicit CustomTableView(const QString& instName, QWidget* parent = nullptr);
  ~CustomTableView();

  const QString& GetName() const { return m_name; }

  void PushFrontExclusiveActions(const QList<QAction*>& acts);

  void InitTableView();

  int rowHeight() const;
  bool setRowHeight(int newRowHeight);

 signals:
  void searchSqlStatementChanged(const QString& sqlStatement);

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void scrollContentsBy(int dx, int dy) override;
  void contextMenuEvent(QContextMenuEvent* event) override;
  virtual void initExclusivePreferenceSetting() {}
  bool m_defaultShowHorizontalHeader{true}, m_defaultShowVerticalHeader{true}, m_defaultShowBackgroundImage{false};
  void paintEvent(QPaintEvent *event) override;
  void changeEvent(QEvent* event) override;

 private:
  void AddItselfAction2Menu();
  void SubscribeHeaderActions();
  bool ShowOrHideColumnCore();
  void UpdateCachedColor();
  void onStyleChanged();

  const QString m_name;

  const QString m_showHorizontalHeaderKey;
  const QString m_showVerticalHeaderKey;
  const QString m_autoScrollKey;
  const QString m_alternatingRowColorsKey;
  const QString m_showGridKey;

  QAction* _SHOW_ALL_HORIZONTAL_COLUMNS{nullptr};
  QAction *_SHOW_HORIZONTAL_HEADER{nullptr}, *_SHOW_VERTICAL_HEADER{nullptr};
  QAction *_RESIZE_ROW_TO_CONTENTS{nullptr}, *_RESIZE_COLUMN_TO_CONTENTS{nullptr};
  QAction* _AUTO_SCROLL{nullptr};
  QAction* _ALTERNATING_ROW_COLORS{nullptr};
  QAction* _SHOW_GRID{nullptr};
  ScrollBarPolicyMenu *m_horScrollBarPolicyMenu{nullptr}, *m_verScrollBarPolicyMenu{nullptr};

  DoubleRowHeader* m_horHeader{nullptr};
  VerMenuInHeader* m_verHeader{nullptr};
  TextElideModeMenu* _TEXT_ELIDE_MODE_MENU{nullptr};
  AddableMenu* m_menu{nullptr};

  QColor m_cachedColor;

  inline bool isNameExists(const QString& name) const { return mTableInstSet.contains(name); }
  static QSet<QString> mTableInstSet;
};

#endif  // CUSTOMTABLEVIEW_H
