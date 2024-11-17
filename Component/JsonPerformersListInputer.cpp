#include "JsonPerformersListInputer.h"
#include <QFile>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>
#include "Tools/JsonFileHelper.h"
#include "Tools/NameTool.h"
#include "Tools/PerformersManager.h"

JsonPerformersListInputer::JsonPerformersListInputer(QWidget* parent, Qt::WindowFlags f)
    : QDialog{parent, f},
      m_onePerf(new QLineEdit),
      m_perfsList(new QLineEdit),
      buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)),
      p_dict(nullptr) {
  m_onePerf->setCompleter(&PerformersManager::getIns().perfsCompleter);
  m_onePerf->addAction(QIcon(":img/RENAME_PERFORMERS"), QLineEdit::LeadingPosition);
  m_onePerf->setClearButtonEnabled(true);
  m_perfsList->setClearButtonEnabled(true);

  auto* lo = new QFormLayout;
  lo->addRow("performer", m_onePerf);
  lo->addRow("list", m_perfsList);
  lo->addWidget(buttonBox);
  setLayout(lo);

  subscribe();
  setWindowIcon(QIcon(":img/RENAME_PERFORMERS"));
  setWindowTitle("mod performers");
}

bool JsonPerformersListInputer::appendAPerformer() {
  const QString& perf = m_onePerf->text().trimmed();
  if (perf.isEmpty()) {
    return true;
  }
  QStringList stdPerfL = perf.split(' ');
  for (auto& word : stdPerfL) {
    if (word.isEmpty()) {
      continue;
    }
    word[0] = word[0].toUpper();
  }
  const QString& stdPerf = stdPerfL.join(' ');

  const QString& perfs = text();
  QStringList perfsL = (perfs.isEmpty() ? QStringList() : NameTool()(perfs));

  if (perfsL.contains(stdPerf)) {
    return false;
  }
  perfsL.append(stdPerf);
  m_perfsList->setText(perfsL.join(", "));
  return true;
}

void JsonPerformersListInputer::uniquePerformers() {
  const QString& perfs = text();
  if (perfs.isEmpty()) {
    return;
  }
  const QStringList& perfL = NameTool()(perfs);
  m_perfsList->setText(perfL.join(", "));
}

bool JsonPerformersListInputer::submitPerformersListToJsonFile() {
  const QString& jsonFilePath = windowFilePath();
  if (not p_dict) {
    qDebug("Cannot submit. dict is nullptr");
    return false;
  }
  QVariantHash& dict = *p_dict;
  if (not dict.contains(JSONKey::Performers)) {
    return false;
  }
  const QString& perfs = text();
  dict[JSONKey::Performers] = NameTool()(perfs);
  return JsonFileHelper::MovieJsonDumper(dict, jsonFilePath);
}

bool JsonPerformersListInputer::reloadPerformersFromJsonFile(const QString& jsonFilePath, QVariantHash& dict) {
  p_dict = &dict;
  if (not QFile::exists(jsonFilePath)) {
    setWindowFilePath("");
    qDebug("error path[%s] not exist", qPrintable(jsonFilePath));
    return false;
  }
  setWindowFilePath(jsonFilePath);
  if (not dict.contains(JSONKey::Performers)) {
    return false;
  }
  static PerformersManager& pm = PerformersManager::getIns();
  QStringList perfL = dict[JSONKey::Performers].toStringList();
  if (perfL.isEmpty()) {
    if (dict.contains(JSONKey::Name)) {
      const QString& name = dict[JSONKey::Name].toString();
      perfL = pm(name);
    }
  }
  perfL.removeDuplicates();
  m_perfsList->setText(perfL.join(", "));
  return true;
}

void JsonPerformersListInputer::subscribe() {
  connect(m_onePerf, &QLineEdit::editingFinished, this, &JsonPerformersListInputer::appendAPerformer);
  connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [this]() {
    bool ret = JsonPerformersListInputer::submitPerformersListToJsonFile();
    qDebug("Save mod performers result: %d", ret);
    hide();
  });
  connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &JsonPerformersListInputer::hide);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  PerformersListInputer pw;
  pw.show();
  a.exec();
  return 0;
}
#endif
