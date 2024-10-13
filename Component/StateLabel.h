#ifndef STATELABEL_H
#define STATELABEL_H

#include <QLabel>
#include <QMap>
#include <QPixmap>

class StateLabel : public QLabel {
  static const QString SAVED_STR;
  static const QString NOT_SAVED_STR;
  const QMap<QString, QPixmap> pixmapMap;
  QString currentState;

 public:
  explicit StateLabel(const QString& text, QWidget* parent = nullptr)
      : QLabel(text, parent),
        pixmapMap{{SAVED_STR, QPixmap(":img/SAVED").scaled(24, 24, Qt::KeepAspectRatio)},
                  {NOT_SAVED_STR, QPixmap(":img/NOT_SAVED").scaled(24, 24, Qt::KeepAspectRatio)}},
        currentState(SAVED_STR) {
    setPixmap(pixmapMap[currentState]);
    setAlignment(Qt::AlignRight);
  }
 public slots:
  void ToSaved() {
    currentState = SAVED_STR;
    setPixmap(pixmapMap[currentState]);
  }
  void ToNotSaved() {
    currentState = NOT_SAVED_STR;
    setPixmap(pixmapMap[currentState]);
  }
};

#endif  // STATELABEL_H
