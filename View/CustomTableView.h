#ifndef CUSTOMTABLEVIEW_H
#define CUSTOMTABLEVIEW_H

#include <QTableView>
#include "VerMenuInHeader.h"
#include "DoubleRowHeader.h"
#include "ScrollBarPolicyMenu.h"

class CustomTableView : public QTableView {
  Q_OBJECT
public:
  explicit CustomTableView(const QString& name, QWidget* parent = nullptr);
  ~CustomTableView();

  virtual void contextMenuEvent(QContextMenuEvent* event) override;

  void BindMenu(QMenu* menu);

  bool ShowOrHideColumnCore();

  void InitTableView(const bool bHideShowCol = true);
  void SubscribeHeaderActions();

  void mousePressEvent(QMouseEvent* event) override;

signals:
  void searchSqlStatementChanged(const QString& sqlStatement);

protected:
  void scrollContentsBy(int dx, int dy) override;

private:
  QMenu* m_menu{nullptr};

  const QString m_name;
  const QString m_showHorizontalHeaderKey;
  const QString m_showVerticalHeaderKey;
  const QString m_autoScrollKey;
  const QString m_alternatingRowColorsKey;
  const QString m_showGridKey;

  QAction *_SHOW_HORIZONTAL_HEADER{nullptr}, *_SHOW_VERTICAL_HEADER{nullptr};
  QAction *_RESIZE_ROW_TO_CONTENTS{nullptr}, *_RESIZE_COLUMN_TO_CONTENTS{nullptr};
  QAction* _AUTO_SCROLL{nullptr};
  QAction* _ALTERNATING_ROW_COLORS{nullptr};
  QAction* _SHOW_GRID{nullptr};
  ScrollBarPolicyMenu *m_horScrollBarPolicyMenu{nullptr}, *m_verScrollBarPolicyMenu{nullptr};

  DoubleRowHeader* m_horHeader{nullptr};
  VerMenuInHeader* m_verHeader{nullptr};

  inline bool isNameExists(const QString& name) const { return TABLES_SET.contains(name); }
  static QSet<QString> TABLES_SET;
};

#endif // CUSTOMTABLEVIEW_H
