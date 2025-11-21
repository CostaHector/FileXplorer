#ifndef HORMENUINHEADER_H
#define HORMENUINHEADER_H

#include "MenuInHeader.h"

class HorMenuInHeader : public MenuInHeader {
  Q_OBJECT
public:
  explicit HorMenuInHeader(const QString& proName, QWidget* parent = nullptr);
  ~HorMenuInHeader();

  bool isSortingEnabled() const { return _ENABLE_COLUMN_SORT->isChecked(); }
  const QString& GetColumnsShowSwitch() const { return m_columnsShowSwitch; }
  const QStringList& getTitles() const;

signals:
  void reqHideAColumn(int colIndex, bool bHide);
  void reqSortEnabled(bool bEnableSort);
  void reqUpdateColumnVisibilty();

public slots:
  bool onHideThisColumnTriggered();
  bool onColumnVisibilityAdjust();

protected:
  void InitHorHeaderTitles(QStringList& newTitles) const;
  bool isColumnHidden(int col) const { return m_columnsShowSwitch[col] == SW_OFF; }

private:
  const QString m_sortByColumnSwitchKey;
  const QString m_columnVisibiltyKey;

  QString m_columnsShowSwitch; // 111110000011111
  QAction* _COLUMNS_VISIBILITY{nullptr};
  QAction* _HIDE_THIS_COLUMN{nullptr};
  QAction* _ENABLE_COLUMN_SORT{nullptr};

  mutable QStringList m_horHeaderTitles;

  static const QString& DEFAULT_SWITCHES();
  static constexpr QChar SW_OFF = '0';
  static constexpr QChar SW_ON = '1';
};

#endif // HORMENUINHEADER_H
