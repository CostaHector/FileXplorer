#ifndef STAGINGSTATISTICSDIALOG_H
#define STAGINGSTATISTICSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QStringList>

class QTextBrowser;
namespace QtCharts{
class QChart;
class QPieSlice;
class QLegendMarker;
}

class StagingStatisticsDialog : public QDialog {
  Q_OBJECT
public:
  explicit StagingStatisticsDialog(QMap<QString, QStringList>&& title2Items, QWidget* parent);

private slots:
  void onSliceClicked(QtCharts::QPieSlice* slice);
  void onSliceHoverd(QtCharts::QPieSlice* slice, bool hovered);

private:
  void ReadSettings();
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

  QMap<QString, QStringList> m_title2Items;
  QTextBrowser* mTextBrowser{nullptr};

  QtCharts::QLegendMarker* mLastSelectedMarker{nullptr};
  QMap<QtCharts::QPieSlice*, QtCharts::QLegendMarker*> mSlice2Marker;
};

#endif // STAGINGSTATISTICSDIALOG_H
