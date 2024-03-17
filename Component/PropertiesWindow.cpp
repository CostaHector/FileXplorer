#include "PropertiesWindow.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPushButton>
#include <QVBoxLayout>

#include "Tools/FileSystemItemFilter.h"
#include "Tools/MD5Calculator.h"
#include "Tools/MP4DurationGetter.h"

PropertiesWindow::PropertiesWindow(const QStringList& items, QWidget* parent)
    : QDialog(parent),
      m_items(items),
      m_propertiesInfoTextEdit(new QPlainTextEdit(this)),
      m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Orientation::Horizontal, this)),
      m_showMore(new QAction("More", this)),
      m_showMD5(new QAction(QIcon(":/themes/MD5_FILE_IDENTIFIER_PATH"), tr("MD5"), this)),
      m_extraToolbar(new QToolBar("Extra Info", this)) {
  m_showMore->setCheckable(true);
  m_showMore->setToolTip("Display videos duration of each *mp4 file");
  m_showMD5->setCheckable(true);
  m_showMD5->setToolTip("Display MD5 of each file");
  m_propertiesInfoTextEdit->setFont(QFont("Consolas"));
  m_extraToolbar->addActions({m_showMore, m_showMD5});
  m_extraToolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  m_extraToolbar->setOrientation(Qt::Orientation::Horizontal);
  auto* lo = new QVBoxLayout(this);
  lo->addWidget(m_extraToolbar);
  lo->addWidget(m_propertiesInfoTextEdit);
  lo->addWidget(m_buttonBox);
  setLayout(lo);
  UpdateMessage();

  subscribe();
  setWindowIcon(QIcon(":/themes/PROPERTIES"));
}

bool PropertiesWindow::UpdateMessage() {
  return (*this)(m_items);
}

bool PropertiesWindow::operator()(const QStringList& items) {
  if (items.isEmpty()) {
    setWindowTitle("empty property");
    return true;
  }
  if (items.size() == 1) {
    setWindowTitle(QString("%1 property").arg(items.first()));
  } else {
    setWindowTitle(QString("%1 and other %2 items(s) properties").arg(items.first()).arg(items.size() - 1));
  }

  QFileInfo fi(items.first());
  QString propertiesMsg;
  if (items.size() == 1) {
    propertiesMsg += QString("General: Name:[%1]\n  Location:[%2]\n").arg(fi.fileName()).arg(fi.absolutePath());
  } else {
    propertiesMsg += QString("General: %1 items\n").arg(items.size());
  }
  if (m_showMore->isChecked()) {
    // total size, files count, folders count
    propertiesMsg += QString(40, '-');
    const auto& itemStatic = FileSystemItemFilter::ItemCounter(items);

    const qint64 total = itemStatic.fileSize;
    const qint64 xGiB = total / (1 << 30);
    const qint64 xMiB = total % (1 << 30) / (1 << 20);
    const qint64 xkiB = total % (1 << 30) % (1 << 20) / (1 << 10);
    const qint64 xB = total % (1 << 30) % (1 << 20) % (1 << 10);
    const QString sizeMsg = QString("%1GiB+%2MiB+%3KiB+%4Byte = %5B").arg(xGiB).arg(xMiB).arg(xkiB).arg(xB).arg(total);

    propertiesMsg += QString("Contents:\n%1 file(s), %2 folder(s), totalling:\n%3\n").arg(itemStatic.fileCnt).arg(itemStatic.folderCnt).arg(sizeMsg);
    propertiesMsg += QString(40, '-');
    // lag here
    // const QStringList& mp4Files = FileSystemItemFilter::MP4Out(items);
    // propertiesMsg += MP4DurationGetter::DisplayVideosDuration(mp4Files);
  }
  if (m_showMD5->isChecked()) {
    propertiesMsg += QString(40, '-');
    const QStringList& files = FileSystemItemFilter::FilesOut(items);
    propertiesMsg += MD5Calculator::DisplayFilesMD5(files);
  }
  m_propertiesInfoTextEdit->setPlainText(propertiesMsg);
  return true;
}

void PropertiesWindow::subscribe() {
  connect(m_showMore, &QAction::triggered, this, &PropertiesWindow::UpdateMessage);
  connect(m_showMD5, &QAction::triggered, this, &PropertiesWindow::UpdateMessage);
  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &QDialog::accept);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  // PropertiesWindow propertiesWindow({__FILE__});
  PropertiesWindow propertiesWindow({QFileInfo(__FILE__).absolutePath()});
  propertiesWindow.show();
  a.exec();
  return 0;
}
#endif
