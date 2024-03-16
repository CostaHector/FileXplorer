#ifndef CUSTOMTABLEVIEW_H
#define CUSTOMTABLEVIEW_H

#include <QContextMenuEvent>
#include <QMenu>
#include <QTableView>
class CustomTableView : public QTableView {
  Q_OBJECT
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

  void onResizeRowDefaultSectionSize();

  void onShowVerticalHeader(bool showChecked);
  void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
  void onEnableColumnSort(const bool enableChecked);

  void InitTableView();

 private:
  QAction* COLUMNS_VISIBILITY = new QAction(tr("Column title visibility"), this);
  QAction* HIDE_THIS_COLUMN = new QAction(tr("hide this column"), this);
  QAction* SHOW_ALL_COLUMNS = new QAction(tr("show all columns"), this);
  QAction* STRETCH_DETAIL_SECTION = new QAction(tr("stretch last column"), this);
  QAction* ENABLE_COLUMN_SORT = new QAction(tr("enable column sort"), this);

  QAction* SHOW_VERTICAL_HEADER = new QAction(tr("show vertical header"), this);
  QAction* RESIZE_ROWS_TO_CONTENT = new QAction(tr("resize rows to content"), this);
  QAction* RESIZE_ROWS_DEFAULT_SECTION_SIZE = new QAction(tr("adjust default rows section size"), this);

  int m_horizontalHeaderSectionClicked = -1;
  QString m_name;
  QString m_columnVisibiltyKey;
  QString m_stretchLastSectionKey;
  QString m_defaultSectionSizeKey;
  QString m_horizontalHeaderStateKey;
  QString m_showVerticalHeaderKey;
  QString m_sortByColumnSwitchKey;

  int m_defaultTableRowHeight;

  QString m_horHeaderTitles;
  bool m_horHeaderTitlesInit = false;
  QString m_columnsShowSwitch;  // 111110000011111
  static constexpr int SWITCHS_BATCH_COUNT = 5;
  QMenu* m_menu = nullptr;
  QMenu* m_verMenu = new QMenu{tr("vertical header menu"), this};
  QMenu* m_horMenu = new QMenu{tr("horizontal header menu"), this};

  inline bool isNameExists(const QString& name) const { return TABLES_SET.contains(name); }
  static QSet<QString> TABLES_SET;

  bool isIndicatorHoldByRestoredStateTrustable = false;
};

#endif  // CUSTOMTABLEVIEW_H
