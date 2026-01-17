#include "ResourceMonitorPanel.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "PublicVariable.h"
#include "ResourceMonitor.h"
#include "FileLeafAction.h"
#include "StyleSheet.h"

QByteArray UsageReport::toByteArray() const {
  QByteArray content;
  QTextStream stream(&content);
  stream.setCodec("UTF-8");
  stream << timePoint << ',' << memoryUsed << ',' << cpuUsageRate << '\n';
  return content;
}

ResourceMonitorPanel::ResourceMonitorPanel(QWidget* parent) : QWidget{parent} {
  {
    mInterval = new (std::nothrow) QSlider{Qt::Orientation::Horizontal, this};
    mInterval->setMinimum(0);
    mInterval->setMaximum(20 * 1000);
    mInterval->setValue(1000);
    mInterval->setSingleStep(1000);
    mInterval->setPageStep(1000);
    mInterval->setTickInterval(1000);
    mInterval->setTickPosition(QSlider::TicksBelow);
    mInterval->setToolTip("Sample period(unit: millisecond)");
    mIntervalLabel = new (std::nothrow) QLabel{GetSampleIntervalString(mInterval->value()), this};

    mExportBtn = new (std::nothrow) QPushButton{QIcon(":/edit/EXPORT"), "Export", this};

    mCpuSwitch = new (std::nothrow) QCheckBox{"cpu", this};
    mCpuSwitch->setChecked(true);
    mMemorySwitch = new (std::nothrow) QCheckBox{"memory", this};
    mMemorySwitch->setChecked(true);

    mControlsLayout = new (std::nothrow) QHBoxLayout{nullptr};
    mControlsLayout->addWidget(mIntervalLabel);
    mControlsLayout->addWidget(mInterval, 3);
    mControlsLayout->addWidget(mCpuSwitch);
    mControlsLayout->addWidget(mMemorySwitch);
    mControlsLayout->addWidget(mExportBtn);
  }

  {
    cpuSeries = new QLineSeries();
    cpuSeries->setName("cpu");
    cpuSeries->setPen(QPen(Qt::GlobalColor::blue, 2));
    cpuSeries->setPointsVisible(true);
    cpuAxisX = new QValueAxis();
    cpuAxisX->setTitleText("Time (second)");
    cpuAxisX->setRange(-WINDOW_SIZE, 0);
    cpuAxisY = new QValueAxis();
    cpuAxisY->setTitleText("Cpu(%)");
    cpuAxisY->setRange(cpuYmin, cpuYmax);
    mCpuChart = new (std::nothrow) QChart(nullptr);
    mCpuChart->setTitle("CPU");
    mCpuChart->addAxis(cpuAxisX, Qt::AlignBottom);
    mCpuChart->addAxis(cpuAxisY, Qt::AlignRight);
    mCpuChart->addSeries(cpuSeries);
    cpuSeries->attachAxis(cpuAxisX);
    cpuSeries->attachAxis(cpuAxisY);
    mCpuChartView = new QChartView(mCpuChart);
    mCpuChartView->setRenderHint(QPainter::Antialiasing);

    memorySeries = new QLineSeries();
    memorySeries->setName("memory");
    memorySeries->setPen(QPen(Qt::GlobalColor::red, 2));
    memorySeries->setPointsVisible(true);
    memoryAxisX = new QValueAxis();
    memoryAxisX->setTitleText("Time (second)");
    memoryAxisX->setRange(-WINDOW_SIZE, 0);
    memoryAxisY = new QValueAxis();
    memoryAxisY->setTitleText("Memory(kB)");
    memoryAxisY->setRange(memoryYmin, memoryYmax);
    mMemoryChart = new (std::nothrow) QChart(nullptr);
    mMemoryChart->setTitle("Memory");
    mMemoryChart->addAxis(memoryAxisX, Qt::AlignBottom);
    mMemoryChart->addAxis(memoryAxisY, Qt::AlignRight);
    mMemoryChart->addSeries(memorySeries);
    memorySeries->attachAxis(memoryAxisX);
    memorySeries->attachAxis(memoryAxisY);
    mMemoryChartView = new QChartView(mMemoryChart);
    mMemoryChartView->setRenderHint(QPainter::Antialiasing);
  }

  mChartsLayout = new (std::nothrow) QHBoxLayout;
  mChartsLayout->addWidget(mCpuChartView);
  mChartsLayout->addWidget(mMemoryChartView);

  mLayout = new (std::nothrow) QVBoxLayout{this};
  mLayout->addLayout(mControlsLayout, 1);
  mLayout->addLayout(mChartsLayout, 5);

  setWindowIcon(QIcon(":img/USAGE_MONITOR"));
  setWindowTitle("Resource monitor");

  if (!IsPaused()) {
    mTimer.setInterval(GetInterval());
    mTimer.start();
  }
  InitReportFileName();
  Subscribe();
}

void ResourceMonitorPanel::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void ResourceMonitorPanel::hideEvent(QHideEvent* event) {
  g_fileLeafActions()._CPU_MEMORY_USAGE_MONITOR->setChecked(false);
  QWidget::hideEvent(event);
}

void ResourceMonitorPanel::Subscribe() {
  connect(&mTimer, &QTimer::timeout, this, &ResourceMonitorPanel::onTimeout);
  connect(mInterval, &QSlider::valueChanged, this, &ResourceMonitorPanel::onSamplePeriodChanged);
  connect(mCpuSwitch, &QCheckBox::toggled, mCpuChartView, &QChartView::setVisible);
  connect(mMemorySwitch, &QCheckBox::toggled, mMemoryChartView, &QChartView::setVisible);
  connect(mExportBtn, &QPushButton::clicked, this, &ResourceMonitorPanel::onExportUsageToLocalFile);
}

void ResourceMonitorPanel::InitReportFileName() {
  char reportStartTime[LOG_TIME_PATTERN_LEN]{0};
  get_timestamp(reportStartTime, LOG_TIME_PATTERN_LEN);
  mExportCSVName = "usage_monitor_";
  mExportCSVName += reportStartTime;
  mUsageReports = "Time,Memory(kB),CPU(%)\n";
}
void ResourceMonitorPanel::FillReportFileEndTime() {
  char reportEndTime[LOG_TIME_PATTERN_LEN]{0};
  get_timestamp(reportEndTime, LOG_TIME_PATTERN_LEN);
  mExportCSVName += "_to_";
  mExportCSVName += reportEndTime;
  mExportCSVName += ".csv";
  mExportCSVName.replace('/', '_').replace(':', '_');
}

void ResourceMonitorPanel::onSamplePeriodChanged(int newPeriodInMilliSecond) {
  mIntervalLabel->setText(GetSampleIntervalString(newPeriodInMilliSecond));
  if (newPeriodInMilliSecond == 0) {
    mTimer.stop();
    mTimer.setInterval(newPeriodInMilliSecond);
    return;
  }
  if (mTimer.interval() == newPeriodInMilliSecond) {
    LOG_D("sample period remains[%d] millisecond", newPeriodInMilliSecond);
    return;
  }
  mTimer.stop();
  mTimer.setInterval(newPeriodInMilliSecond);
  mTimer.start();
  LOG_D("sample period changed to[%d]ms", newPeriodInMilliSecond);
}

void ResourceMonitorPanel::onTimeout() {
  UsageReport usageRep;
  char timestamp[LOG_TIME_PATTERN_LEN]{0};
  get_timestamp(timestamp, LOG_TIME_PATTERN_LEN);
  usageRep.timePoint = timestamp;

  // memory usage(kB)
  if (mMemorySwitch->isChecked()) {
    double memoryUsed = 0;
    memoryUsed = ResourceMonitor::getMemoryUsage();
    if (memorySeries->count() >= WINDOW_SIZE) {
      memorySeries->remove(0);
    }
    memorySeries->append(currentTime, memoryUsed);
    memoryYmin = std::min(memoryYmin, memoryUsed);
    memoryYmax = std::max(memoryYmax, memoryUsed);
    memoryAxisY->setRange(memoryYmin, memoryYmax);
    usageRep.memoryUsed = memoryUsed;
  }
  const int minX = currentTime - WINDOW_SIZE;
  const int maxX = currentTime;
  memoryAxisX->setRange(minX, maxX);

  // cpu usage rate(%)
  if (mCpuSwitch->isChecked()) {
    float cpuUsageRate = 0.0;
    quint64 currentCpuTime = ResourceMonitor::getProcessCpuTime();
    if (m_lastCpuTime != 0) { // m_lastCpuTime should not be empty
      quint64 cpuDelta = currentCpuTime - m_lastCpuTime;
      static const int numCores = ResourceMonitor::GetNumbersCore();
      static const quint64 maxCpuTime = GetInterval() * numCores;  // max cpu available time in ms
      if (maxCpuTime > 0) {
        cpuUsageRate = (static_cast<float>(cpuDelta) / maxCpuTime) * 100;
      }
      cpuYmin = std::min(cpuYmin, cpuUsageRate);
      cpuYmax = std::max(cpuYmax, cpuUsageRate);
      cpuAxisY->setRange(cpuYmin, cpuYmax);
      usageRep.cpuUsageRate = cpuUsageRate;
    }
    m_lastCpuTime = currentCpuTime;
    if (cpuSeries->count() >= WINDOW_SIZE) {
      cpuSeries->remove(0);
    }
    cpuSeries->append(currentTime, cpuUsageRate);
  }
  cpuAxisX->setRange(minX, maxX);

  mUsageReports += usageRep.toByteArray();
  NextSamplePoint();
}

bool ResourceMonitorPanel::onExportUsageToLocalFile() {
  FillReportFileEndTime();
  QString cpuMemoryCsvFile = SystemPath::HOME_PATH();
  cpuMemoryCsvFile += "/Downloads/";
  cpuMemoryCsvFile += mExportCSVName;
  QFileDialog::saveFileContent(mUsageReports, cpuMemoryCsvFile);
  LOG_INFO_P("Export cpu/memory reports to", "size:%d, path:%s", mUsageReports.size(), qPrintable(cpuMemoryCsvFile));
  InitReportFileName();
  return true;
}
