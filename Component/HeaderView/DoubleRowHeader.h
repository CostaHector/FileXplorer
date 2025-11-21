#ifndef DOUBLEROWHEADER_H
#define DOUBLEROWHEADER_H

#include "HorMenuInHeader.h"
#include <QMetaObject>
#include <QLineEdit>

class DoubleRowHeader : public HorMenuInHeader {
  Q_OBJECT
public:
  explicit DoubleRowHeader(const QString& proName, QWidget* parent = nullptr);
  ~DoubleRowHeader();

  static QString GetColumn2SearchTemplate(const QString& columnName);

  void UpdateSearchStatement();

  void updateFilterEditorsGeometry();

  void InitFilterEditors();

signals:
  void searchStatementChanged(const QString& s);
  void reqParentTableUpdateGeometries();

public slots:
  void onToggleEnableOrDisableFilter(bool bEnableFilter);

protected:
  void resizeEvent(QResizeEvent* event) override;

  QSize sizeHint() const override;

private:
  void hideAllLineEdit();
  bool isFilterEnabled() const { return _ENABLE_FILTERS->isChecked(); }
  QAction* _ENABLE_FILTERS{nullptr};
  const QString m_enableFilterKey;

  QVector<QLineEdit*> m_filterEditors;
  QMetaObject::Connection m_connSectionResized;
};

#endif // DOUBLEROWHEADER_H
