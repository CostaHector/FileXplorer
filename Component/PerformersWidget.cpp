#include "PerformersWidget.h"
#include <QFile>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>
#include "Component/PerformersManager.h"
#include "Tools/JsonFileHelper.h"
PerformerLineEditor::PerformerLineEditor(QWidget* parent) : QLineEdit(parent) {
  setCompleter(&PerformersManager::getIns().perfsCompleter);
}

PerformersWidget::PerformersWidget(QWidget* parent, Qt::WindowFlags f)
    : QDialog{parent, f},
      m_onePerf(new PerformerLineEditor),
      m_perfsList(new QLineEdit),
      buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)),
      p_dict(nullptr) {
  m_onePerf->addAction(QIcon(":/themes/RENAME_PERFORMERS"), QLineEdit::LeadingPosition);
  m_onePerf->setClearButtonEnabled(true);
  m_perfsList->setClearButtonEnabled(true);

  auto* lo = new QFormLayout;
  lo->addRow("performer", m_onePerf);
  lo->addRow("list", m_perfsList);
  lo->addWidget(buttonBox);
  setLayout(lo);

  subscribe();
  setWindowIcon(QIcon(":/themes/RENAME_PERFORMERS"));
  setWindowTitle("mod performers");
}

bool PerformersWidget::appendAPerformer() {
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
  QStringList perfsL = (perfs.isEmpty() ? QStringList() : perfs.split(SEPERATOR_COMP));

  if (perfsL.contains(stdPerf)) {
    return false;
  }
  perfsL.append(stdPerf);
  m_perfsList->setText(perfsL.join(", "));
  return true;
}

void PerformersWidget::uniquePerformers() {
  const QString& perfs = text();
  if (perfs.isEmpty()) {
    return;
  }
  QStringList perfL = perfs.split(SEPERATOR_COMP);
  perfL.removeDuplicates();
  m_perfsList->setText(perfL.join(", "));
}

bool PerformersWidget::submitPerformersListToJsonFile() {
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
  dict[JSONKey::Performers] = JsonFileHelper::PerformersString2StringList(perfs);
  return JsonFileHelper::MovieJsonDumper(dict, jsonFilePath);
}

bool PerformersWidget::reloadPerformersFromJsonFile(const QString& jsonFilePath, QVariantHash& dict) {
  p_dict = &dict;
  if (not QFile::exists(jsonFilePath)) {
    setWindowFilePath("");
    qDebug("error path[%s] not exist", jsonFilePath.toStdString().c_str());
    return false;
  }
  setWindowFilePath(jsonFilePath);
  if (not dict.contains(JSONKey::Performers)) {
    return false;
  }
  QStringList perfL = dict[JSONKey::Performers].toStringList();
  if (perfL.isEmpty()) {
    if (dict.contains(JSONKey::Name)) {
      static PerformersManager& pm = PerformersManager::getIns();
      const QString& name = dict[JSONKey::Name].toString();
      perfL = pm(name);
    }
  }
  perfL.removeDuplicates();
  m_perfsList->setText(perfL.join(", "));
}

void PerformersWidget::subscribe() {
  connect(m_onePerf, &QLineEdit::editingFinished, this, &PerformersWidget::appendAPerformer);
  connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [this]() {
    bool ret = PerformersWidget::submitPerformersListToJsonFile();
    qDebug("Save mod performers result: %d", ret);
    hide();
  });
  connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &PerformersWidget::hide);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  PerformersWidget pw;
  pw.show();
  a.exec();
  return 0;
}
#endif
