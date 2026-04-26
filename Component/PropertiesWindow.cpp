#include "PropertiesWindow.h"
#include "PropertiesWindowActions.h"
#include "TableFields.h"
#include "SizeTool.h"
#include "Configuration.h"
#include "FileSystemItemFilter.h"
#include "MD5Calculator.h"
#include "StyleSheet.h"
#include "DataFormatter.h"
#include "VideoDurationGetter.h"
#include "VidsDurationDisplayString.h"

const QString PropertiesWindow::STRING_SPLITTER{60, '-'};

PropertiesWindow::PropertiesWindow(QWidget* parent)  //
    : QDialog{parent}                                //
{
  m_propertyTB = g_propertiesWindowAct().getPropertiesToolBar(this);
  m_propertiesInfoTextEdit = new (std::nothrow) QTextEdit(this);
  m_propertiesInfoTextEdit->setProperty("UseCodeFontFamily", true);

  m_mainLo = new (std::nothrow) QVBoxLayout{this};
  m_mainLo->addWidget(m_propertyTB);
  m_mainLo->addWidget(m_propertiesInfoTextEdit);
  m_mainLo->setSpacing(0);
  m_mainLo->setContentsMargins(0, 0, 0, 0);
  setLayout(m_mainLo);

  setWindowFlags(Qt::Window);  // show maximize, minimize button at title bar
  setWindowIcon(QIcon(":img/PROPERTIES"));

  ReadSetting();

  subscribe();
}

bool PropertiesWindow::UpdateMessage() {
  QString propertiesMsg;
  if (g_propertiesWindowAct().SHOW_FILES_SIZE->isChecked()) {
    if (m_commonInfomation.isEmpty()) {
      InitCommonInfo();
    }
    propertiesMsg += STRING_SPLITTER;
    propertiesMsg += m_commonInfomation;
    propertiesMsg += "<br/>\n";
  }
  if (g_propertiesWindowAct().SHOW_VIDS_DURATION->isChecked()) {
    if (m_durations.isEmpty()) {
      InitDurationInfo();
    }
    propertiesMsg += STRING_SPLITTER;
    propertiesMsg += m_durations;
    propertiesMsg += "<br/>\n";
  }
  if (g_propertiesWindowAct().SHOW_FILES_MD5->isChecked()) {
    if (m_fileIdentifier.isEmpty()) {
      InitFileIndentifierInfo();
    }
    propertiesMsg += STRING_SPLITTER;
    propertiesMsg += m_fileIdentifier;
  }
  m_propertiesInfoTextEdit->setHtml(propertiesMsg);
  return true;
}

void PropertiesWindow::InitCommonInfo() {
  mAllItemStatics = FileSystemItemFilter::ItemCounter(mAllItems);
  const QString sizeMsg = DataFormatter::formatFileSizeWithBytes(mAllItemStatics.fileSize);
  m_commonInfomation = QString("Contents: %1 file(s), %2 folder(s).<br/>\n").arg(mAllItemStatics.fileCnt).arg(mAllItemStatics.folderCnt);
  m_commonInfomation += QString("Size: %3").arg(sizeMsg);
}

bool PropertiesWindow::InitDurationInfo() {
  const QStringList& fileAbsPaths{FileSystemItemFilter::MP4Out(mAllItems)};
  VideoDurationGetter mi;
  if (!mi.StartToGet()) {
    return false;
  }
  const QList<int> durationsList {mi.GetLengthsQuick(fileAbsPaths)};
  m_durations = VidsDurationDisplayString::DurationPrepathName2Table(durationsList, fileAbsPaths);
  return true;
}

void PropertiesWindow::InitFileIndentifierInfo() {
  const QStringList& files = FileSystemItemFilter::FilesOut(mAllItems);
  m_fileIdentifier = MD5Calculator::DisplayFilesMD5(files);
}

bool PropertiesWindow::operator()(const QStringList& items) {
  mAllItems = items;
  setWindowTitle(QString("Property | [%1] item(s)").arg(mAllItems.size()));
  if (mAllItems.isEmpty()) {
    m_propertiesInfoTextEdit->setPlainText("Nothing selected");
    return true;
  }
  m_commonInfomation.clear();
  m_durations.clear();
  m_fileIdentifier.clear();

  if (g_propertiesWindowAct().SHOW_FILES_SIZE->isChecked()) {
    InitCommonInfo();
  }

  if (g_propertiesWindowAct().SHOW_VIDS_DURATION->isChecked()) {
    InitDurationInfo();
  }

  if (g_propertiesWindowAct().SHOW_FILES_MD5->isChecked()) {
    InitFileIndentifierInfo();
  }

  UpdateMessage();
  return true;
}

bool PropertiesWindow::operator()(const QList<qint64>& fileSizes, const QList<int>& durations) {
  setWindowTitle(QString("Property | [%1] item(s)").arg(durations.size()));
  if (fileSizes.isEmpty() && durations.isEmpty()) {
    m_propertiesInfoTextEdit->setPlainText("Nothing selected");
    return true;
  }

  m_fileIdentifier = "not available";

  if (g_propertiesWindowAct().SHOW_FILES_SIZE->isChecked()) {
    const qint64 totalSz{std::accumulate(fileSizes.cbegin(), fileSizes.cend(), (qint64)0)};
    const QString sizeMsg{DataFormatter::formatFileSizeWithBytes(totalSz)};
    m_commonInfomation = QString::asprintf("%d file(s) sizes: %s", fileSizes.size(), qPrintable(sizeMsg));
  } else {
    m_commonInfomation = "not available";
  }

  if (g_propertiesWindowAct().SHOW_VIDS_DURATION->isChecked()) {
    const qint64 totalDuration{std::accumulate(durations.cbegin(), durations.cend(), (qint64)0)};
    const QString durationMsg{DataFormatter::formatDurationISO(totalDuration)};
    m_durations = QString::asprintf("%d file(s) durations: %s", durations.size(), qPrintable(durationMsg));
  } else {
    m_durations = "not available";
  }

  UpdateMessage();
  return true;
}

void PropertiesWindow::ReadSetting() {
  if (Configuration().contains("Geometry/PropertiesWindow")) {
    restoreGeometry(Configuration().value("Geometry/PropertiesWindow").toByteArray());
  } else {
    setGeometry(SizeTool::DEFAULT_GEOMETRY);
  }
}

void PropertiesWindow::showEvent(QShowEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void PropertiesWindow::closeEvent(QCloseEvent* event) {
  Configuration().setValue("Geometry/PropertiesWindow", saveGeometry());
  QDialog::closeEvent(event);
}

void PropertiesWindow::subscribe() {
  connect(g_propertiesWindowAct().SHOW_FILES_SIZE, &QAction::triggered, this, &PropertiesWindow::UpdateMessage);
  connect(g_propertiesWindowAct().SHOW_VIDS_DURATION, &QAction::triggered, this, &PropertiesWindow::UpdateMessage);
  connect(g_propertiesWindowAct().SHOW_FILES_MD5, &QAction::triggered, this, &PropertiesWindow::UpdateMessage);
}
