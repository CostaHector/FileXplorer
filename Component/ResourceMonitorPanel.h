#ifndef RESOURCEMONITORPANEL_H
#define RESOURCEMONITORPANEL_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QtCharts>
#include <QChart>
#include <QValueAxis>
#include <QLineSeries>
#include <QTimer>

struct UsageReport {
  QString timePoint;
  float memoryUsed; // kB
  float cpuUsageRate; // %
  QByteArray toByteArray() const;
};

class ResourceMonitorPanel : public QWidget {
public:
  explicit ResourceMonitorPanel(QWidget* parent=nullptr);
  int GetInterval() const { // unit: ms
    return mInterval->value();
  }
  bool IsPaused() const {
    return GetInterval() <= 0;
  }
  void NextSamplePoint() {
    currentTime += mInterval->value() / 1000;
  }
  void onTimeout();
  void onSamplePeriodChanged(int newPeriodInMilliSecond);
  bool onExportUsageToLocalFile();
protected:
  QSize sizeHint() const {
    return {1024, 768};
  }
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;
private:
  void Subscribe();
  static QString GetSampleIntervalString(const int newPeriodInMilliSecond) {
    return QString::asprintf("Period: %d(ms)", newPeriodInMilliSecond);
  }
  void InitReportFileName();
  void FillReportFileEndTime();

  QSlider* mInterval{nullptr};
  QLabel* mIntervalLabel{nullptr};
  QPushButton* mExportBtn{nullptr};
  QCheckBox* mCpuSwitch{nullptr}, *mMemorySwitch{nullptr};
  QHBoxLayout* mControlsLayout{nullptr};

  QLineSeries *cpuSeries{nullptr}, *memorySeries{nullptr};
  QValueAxis *memoryAxisX{nullptr}, *memoryAxisY{nullptr};
  QValueAxis *cpuAxisX{nullptr}, *cpuAxisY{nullptr};
  double memoryYmin{0}, memoryYmax{200*1024}; // [0, 200MB]
  float cpuYmin{0}, cpuYmax{20}; // [0, 20%]
  QChart* mCpuChart{nullptr}, *mMemoryChart{nullptr};
  QChartView *mCpuChartView{nullptr}, *mMemoryChartView{nullptr};
  QHBoxLayout* mChartsLayout{nullptr};
  QVBoxLayout* mLayout{nullptr};
  int WINDOW_SIZE = 60; // unit: second
  int currentTime = 0; // unit: second
  QTimer mTimer;

  quint64 m_lastCpuTime = 0;
  QString mExportCSVName;
  QByteArray mUsageReports;
};

#endif // RESOURCEMONITORPANEL_H
