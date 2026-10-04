#include "StagingStatisticsDialog.h"
#include "Configuration.h"
#include "SizeTool.h"
#include "StyleSheet.h"

#include <QtCharts>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

#include <QTextBrowser>
#include <QVBoxLayout>

StagingStatisticsDialog::StagingStatisticsDialog(QMap<QString, QStringList>&& title2Items,
                                                 QWidget* parent)
  : QDialog{parent}, m_title2Items{std::move(title2Items)} {
  // ---- 饼状图 ----
  QtCharts::QChart* chart = new QtCharts::QChart();
  chart->setTitle("Staging File Statistics");
  chart->legend()->setVisible(true);
  chart->legend()->setAlignment(Qt::AlignRight);
  chart->legend()->setLabelColor(Qt::black);
  chart->legend()->setMarkerShape(QtCharts::QLegend::MarkerShapeRectangle);
  chart->setTheme(QChart::ChartThemeLight);

  auto* series = new QtCharts::QPieSeries();
  for (auto it = m_title2Items.constBegin(); it != m_title2Items.constEnd(); ++it) {
    const QString& title = it.key();
    const QStringList& items = it.value();
    const QString label{QString{"%1 (%2)"}.arg(title).arg(items.size())};
    QtCharts::QPieSlice* slice = series->append(label, items.size());
    slice->setLabelVisible(!items.isEmpty());
  }
  chart->addSeries(series);

  auto* chartView = new QtCharts::QChartView(chart, this);
  chartView->setRenderHint(QPainter::Antialiasing);

  // ---- 文件列表 ----
  mTextBrowser = new QTextBrowser(this);
  mTextBrowser->setPlaceholderText("Click a pie slice to list the files in that category.");

  connect(series, &QtCharts::QPieSeries::clicked, this, &StagingStatisticsDialog::onSliceClicked);
  connect(series, &QtCharts::QPieSeries::hovered, this, &StagingStatisticsDialog::onSliceHoverd);

  const QList<QLegendMarker*> markers = chart->legend()->markers();
  for (QtCharts::QLegendMarker* marker : markers) {
    auto* pieMarker = qobject_cast<QtCharts::QPieLegendMarker*>(marker);
    if (pieMarker != nullptr) {
      connect(marker, &QtCharts::QLegendMarker::clicked, this, [this, pieMarker]() { onSliceClicked(pieMarker->slice()); });
      mSlice2Marker[pieMarker->slice()] = marker;
    }
  }

  auto* layout = new QVBoxLayout(this);
  layout->addWidget(chartView);
  layout->addWidget(mTextBrowser);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  setWindowIcon(QIcon{":img/SHADOW_STAGING_STATISTICS"});
  setWindowTitle("Staging Statistics");

  ReadSettings();
  setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
}

void StagingStatisticsDialog::onSliceHoverd(QtCharts::QPieSlice* slice, bool hovered) {
  if (slice == nullptr) {
    return;
  }
  slice->setExploded(hovered);
}

void StagingStatisticsDialog::onSliceClicked(QtCharts::QPieSlice* slice) {
  if (slice == nullptr) {
    return;
  }
  // ---- 旧图例取消加粗 ----
  if (mLastSelectedMarker != nullptr) {
    QFont cancelBoldFont = mLastSelectedMarker->font();
    cancelBoldFont.setBold(false);
    mLastSelectedMarker->setFont(cancelBoldFont);
  }
  // ---- 根据 slice 找到对应的 legend marker ----
  QtCharts::QLegendMarker* targetMarker = mSlice2Marker.value(slice, nullptr);
  // ---- 新图例加粗 ----
  if (targetMarker != nullptr) {
    QFont boldFont = targetMarker->font();
    boldFont.setBold(true);
    targetMarker->setFont(boldFont);
  }
  mLastSelectedMarker = targetMarker;

  // ---- 找到 slice 在 series 中的索引 ----
  auto* series = qobject_cast<QtCharts::QPieSeries*>(slice->series());
  if (series == nullptr) {
    return;
  }
  const int idx = series->slices().indexOf(slice);
  if (idx < 0 || idx >= m_title2Items.size()) {
    return;
  }

  auto it = m_title2Items.constBegin();
  std::advance(it, idx);
  const QStringList& items = it.value();
  if (items.isEmpty()) {
    mTextBrowser->setPlainText("Empty here");
    return;
  }
  mTextBrowser->setPlainText(items.join('\n'));
}

void StagingStatisticsDialog::ReadSettings() {
  if (Configuration().contains("Geometry/STAGING_STATISTICS_DIALOG")) {
    restoreGeometry(Configuration().value("Geometry/STAGING_STATISTICS_DIALOG").toByteArray());
  } else {
    setGeometry(SizeTool::DEFAULT_GEOMETRY);
  }
}

void StagingStatisticsDialog::showEvent(QShowEvent* event) {
  StyleSheet::UpdateTitleBar(this);
  QDialog::showEvent(event);
}

void StagingStatisticsDialog::closeEvent(QCloseEvent* event) {
  Configuration().setValue("Geometry/ADVANCE_RENAMER", saveGeometry());
  QDialog::closeEvent(event);
}