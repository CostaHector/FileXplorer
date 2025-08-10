#include "PropertiesWindow.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPushButton>
#include <QVBoxLayout>

#include "PropertiesWindowActions.h"
#include "TableFields.h"
#include "FileSystemItemFilter.h"
#include "MD5Calculator.h"
#include "VidsDurationDisplayString.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "DisplayEnhancement.h"

const QString PropertiesWindow::STRING_SPLITTER{60, '-'};

PropertiesWindow::PropertiesWindow(QWidget* parent)  //
    : QDialog{parent}                                //
{
  m_propertyTB = g_propertiesWindowAct().getPropertiesToolBar(this);

  m_mainLo->addWidget(m_propertyTB);
  m_mainLo->addWidget(m_propertiesInfoTextEdit);
  setLayout(m_mainLo);

  subscribe();
  setWindowFlags(Qt::Window);  // show maximize, minimize button at title bar

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
  ReadSetting();
  setWindowIcon(QIcon(":img/PROPERTIES"));
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
  const auto& itemStatic = FileSystemItemFilter::ItemCounter(m_items);
  const QString sizeMsg = FILE_PROPERTY_DSP::sizeToFileSizeDetail(itemStatic.fileSize);
  m_commonInfomation = QString("Contents: %1 file(s), %2 folder(s).<br/>\n").arg(itemStatic.fileCnt).arg(itemStatic.folderCnt);
  m_commonInfomation += QString("Size: %3").arg(sizeMsg);
}

void PropertiesWindow::InitDurationInfo() {
  const QStringList& mp4Files = FileSystemItemFilter::MP4Out(m_items);
  m_durations = VidsDurationDisplayString::DisplayVideosDuration(mp4Files);
}

void PropertiesWindow::InitFileIndentifierInfo() {
  const QStringList& files = FileSystemItemFilter::FilesOut(m_items);
  m_fileIdentifier = MD5Calculator::DisplayFilesMD5(files);
}

bool PropertiesWindow::operator()(const QStringList& items) {
  m_items = items;
  setWindowTitle(QString("Property | [%1] item(s)").arg(m_items.size()));
  if (m_items.isEmpty()) {
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

bool PropertiesWindow::operator()(const QSqlTableModel* model, const QTableView* tv) {
  if (model == nullptr || tv == nullptr) {
    qCritical("model is nullptr");
    return false;
  }

  m_commonInfomation = "not available";
  m_durations = "not available";
  m_fileIdentifier = "not available";

  if (g_propertiesWindowAct().SHOW_FILES_SIZE->isChecked()) {
    qint64 totalSz = 0;
    const QModelIndexList& selIdxs = tv->selectionModel()->selectedRows();
    const QModelIndex rootIndex = tv->rootIndex();
    for (const QModelIndex& idx : selIdxs) {
      const QModelIndex szInd = model->index(idx.row(), MOVIE_TABLE::Size, rootIndex);
      totalSz += model->QSqlTableModel::data(szInd, Qt::ItemDataRole::DisplayRole).toLongLong();
    }
    m_commonInfomation = QString("Contents: %1 file(s), %2 folder(s).<br/>\n").arg(selIdxs.size()).arg(0);
    const QString sizeMsg = FILE_PROPERTY_DSP::sizeToFileSizeDetail(totalSz);
    m_commonInfomation += QString("Size: %3").arg(sizeMsg);
  }

  UpdateMessage();
  return true;
}

void PropertiesWindow::ReadSetting() {
  if (PreferenceSettings().contains("PropertiesWindowGeometry")) {
    restoreGeometry(PreferenceSettings().value("PropertiesWindowGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  m_propertiesInfoTextEdit->setFont(StyleSheet::TEXT_EDIT_FONT);
}

void PropertiesWindow::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void PropertiesWindow::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("PropertiesWindowGeometry", saveGeometry());
  QDialog::closeEvent(event);
}

void PropertiesWindow::subscribe() {
  connect(g_propertiesWindowAct().SHOW_FILES_SIZE, &QAction::triggered, this, &PropertiesWindow::UpdateMessage);
  connect(g_propertiesWindowAct().SHOW_VIDS_DURATION, &QAction::triggered, this, &PropertiesWindow::UpdateMessage);
  connect(g_propertiesWindowAct().SHOW_FILES_MD5, &QAction::triggered, this, &PropertiesWindow::UpdateMessage);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  // PropertiesWindow propertiesWindow({__FILE__});
  PropertiesWindow propertiesWindow;
  propertiesWindow.show();
  //  propertiesWindow({QFileInfo(__FILE__).absolutePath()});
  propertiesWindow({"E:/P/Leaked And Loaded/After taking 12 loads, Gage thanks his with a hot suck - XVIDEOS.COM.ts",
                    "E:/P/Leaked And Loaded/Billy Santoro adds load number 6 to Gage's welcome to DC - XVIDEOS.COM.ts",
                    "E:/P/Leaked And Loaded/Gage Billy Santoro is pretty sensitive after his foreskin removal surgery - XVIDEOS.COM.ts"});
  a.exec();
  return 0;
}
#endif
