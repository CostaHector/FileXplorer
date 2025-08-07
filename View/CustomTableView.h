#ifndef CUSTOMTABLEVIEW_H
#define CUSTOMTABLEVIEW_H

#include <QContextMenuEvent>
#include <QMenu>
#include <QTableView>
class CustomTableView : public QTableView {
 public:
  explicit CustomTableView(const QString& name, QWidget* parent = nullptr);

  virtual void contextMenuEvent(QContextMenuEvent* event) override;

  void BindMenu(QMenu* menu);
  void AppendVerticalHeaderMenuAGS(QActionGroup* extraAgs);
  void AppendHorizontalHeaderMenuAGS(QActionGroup* extraAgs);
  inline int GetClickedHorIndex() const { return m_horizontalHeaderSectionClicked; }

  bool ShowOrHideColumnCore();
  bool onShowHideColumn();
  inline bool onShowAllColumn() {
    m_columnsShowSwitch.replace('0', '1');
    return ShowOrHideColumnCore();
  }
  inline bool onReverseVisibility() {
    m_columnsShowSwitch.replace('0', 'T');
    m_columnsShowSwitch.replace('1', 'F');
    m_columnsShowSwitch.replace('T', '1');
    m_columnsShowSwitch.replace('F', '0');
    return ShowOrHideColumnCore();
  }

  bool onHideThisColumn();

  void onStretchLastSection(const bool checked);

  void onResizeRowToContents(const bool checked);
  void onResizeColumnToContents(const bool checked);

  void onHorizontalHeaderMenuRequest(const QPoint& pnt);
  void onVerticalHeaderMenuRequest(const QPoint& pnt);

  void onSetRowMaxHeight();
  void onSetRowDefaultSectionSize();
  void onSetColumnDefaultSectionSize();

  void onShowHorizontalHeader(bool showChecked);
  void onShowVerticalHeader(bool showChecked);
  void onEnableColumnSort(const bool enableChecked);

  void onHorizontalHeaderChanged() const;
  void onVerticalHeaderChanged() const;

  void InitTableView(const bool bHideShowCol = true);
  void SubscribeHeaderActions();

 private:
  QAction* COLUMNS_VISIBILITY = new (std::nothrow) QAction("column title visibility", this);
  QAction* HIDE_THIS_COLUMN = new (std::nothrow) QAction("hide this column", this);
  QAction* SHOW_ALL_COLUMNS = new (std::nothrow) QAction("show all columns", this);
  QAction* STRETCH_DETAIL_SECTION = new (std::nothrow) QAction("stretch last column", this);
  QAction* ENABLE_COLUMN_SORT = new (std::nothrow) QAction("enable column sort", this);
  QAction* RESIZE_COLUMN_TO_CONTENTS = new (std::nothrow) QAction(QIcon(":img/RESIZE_COLUMN_TO_CONTENTS"), "resize cols to content", this);
  QAction* SET_COLS_DEFAULT_SECTION_SIZE = new (std::nothrow) QAction(QIcon(":img/DEFAULT_COLUMN_WIDTH"), "set default cols section size", this);

  QAction* SHOW_HORIZONTAL_HEADER = new (std::nothrow) QAction("show horizontal header", this);
  QAction* SHOW_VERTICAL_HEADER = new (std::nothrow) QAction("show vertical header", this);
  QAction* RESIZE_ROW_TO_CONTENTS = new (std::nothrow) QAction(QIcon(":img/RESIZE_ROW_TO_CONTENTS"), "resize rows to content", this);
  QAction* SET_ROWS_DEFAULT_SECTION_SIZE = new (std::nothrow) QAction(QIcon(":img/DEFAULT_ROW_HEIGHT"), "set default rows section size", this);
  QAction* SET_MAX_ROWS_SECTION_SIZE = new (std::nothrow) QAction("set max row section size", this);

  int m_horizontalHeaderSectionClicked = -1;
  QString m_name;
  QString m_columnVisibiltyKey;
  QString m_stretchLastSectionKey;
  const QString m_DEFAULT_SECTION_SIZE_KEY;
  const QString m_DEFAULT_COLUMN_SECTION_SIZE_KEY;
  QString m_horizontalHeaderStateKey;
  QString m_showHorizontalHeaderKey;
  QString m_showVerticalHeaderKey;
  QString m_sortByColumnSwitchKey;
  QString m_rowResizeToContentKey;

  int m_defaultTableRowHeight;
  int m_defaultTableColumnWidth;

  QString m_horHeaderTitles;
  bool m_horHeaderTitlesInit = false;
  QString m_columnsShowSwitch;  // 111110000011111
  static constexpr int SWITCHS_BATCH_COUNT = 5;
  QMenu* m_menu = nullptr;
  QMenu* m_verMenu = new QMenu{tr("vertical header menu"), this};
  QMenu* m_horMenu = new QMenu{tr("horizontal header menu"), this};

  bool m_enableDefaultSectionSize {true};

  inline bool isNameExists(const QString& name) const { return TABLES_SET.contains(name); }
  static QSet<QString> TABLES_SET;
};

#endif  // CUSTOMTABLEVIEW_H
