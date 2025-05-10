#include "Notificator.h"

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QStyle>
#include <QtGui>

namespace NOTIFICATOR_SETTING {
constexpr int DEFAULT_MESSAGE_SHOW_TIME = 3000;
constexpr float WINDOW_TRANSPARENT_OPACITY = 0.7;
constexpr float WINDOW_NONTRANSPARENT_OPACITY = 1.0;
constexpr int NOTIFICATION_MARGIN = 10;
constexpr int DISPLAY_NOTIFICATION_DIRECTION_TOP_TO_BOTTOM = false;
}  // namespace NOTIFICATOR_SETTING

using namespace NOTIFICATOR_SETTING;

Notificator::Notificator(bool autohide) : QFrame(0), d(new NotificatorPrivate(autohide)) {
  hide();
  initializeLayout();
  initializeUI();
}

Notificator::~Notificator() {
  delete d;
}

void Notificator::goodNews(const QString& title, const QString& message) {
  showMessage(QIcon(":img/SAVED"), title, message);
}

void Notificator::badNews(const QString& title, const QString& message) {
  showMessage(QIcon(":img/NOT_SAVED"), title, message);
}

void Notificator::critical(const QString& title, const QString& message) {
  qCritical("%s:%s", qPrintable(title), qPrintable(message));
  static const QIcon icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxCritical);
  showMessage(icon, title, message);
}

void Notificator::warning(const QString& title, const QString& message) {
  qWarning("%s:%s", qPrintable(title), qPrintable(message));
  static const QIcon icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning);
  showMessage(icon, title, message);
}

void Notificator::information(const QString& title, const QString& message) {
  static const QIcon icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation);
  showMessage(icon, title, message);
}

void Notificator::question(const QString& title, const QString& message) {
  qDebug("%s:%s", qPrintable(title), qPrintable(message));
  static const QIcon icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxQuestion);
  showMessage(icon, title, message);
}

void Notificator::showMessage(const QIcon& icon, const QString& title, const QString& message) {
  Notificator* instance = new Notificator;
  configureInstance(instance);

  instance->notify(icon, title, message);
  QTimer::singleShot(DEFAULT_MESSAGE_SHOW_TIME, instance, &Notificator::close);
}

Notificator* Notificator::showMessage(const QIcon& icon, const QString& title, const QString& message, const QObject* sender, const char* finishedSignal) {
  if (sender != 0) {
    Notificator* instance = new Notificator(false);
    configureInstance(instance);

    // Запуск уведомления
    instance->notify(icon, title, message);
    connect(sender, finishedSignal, instance, SLOT(close()));
    return instance;
  }
  return nullptr;
}

void Notificator::setMessage(const QString& _message) {
  d->message()->setText(_message);
}

void Notificator::setProgressValue(int _value) {
  d->progress()->show();
  d->progress()->setValue(_value);
}

bool Notificator::event(QEvent* event) {
  if (event->type() == QEvent::HoverEnter) {
    setWindowOpacity(WINDOW_NONTRANSPARENT_OPACITY);
  } else if (event->type() == QEvent::HoverLeave) {
    setWindowOpacity(WINDOW_TRANSPARENT_OPACITY);
  } else if (event->type() == QEvent::MouseButtonPress && d->autoHide()) {
    // Если сообщение не отображает информации о выполняющемся действии можно его закрыть.
    // Т.к. кликом может быть активирована ссылка, то необходимо выждать 100 мсек
    // перед скрытием уведомления, для открытия её в браузере
    QTimer::singleShot(200, this, SLOT(hide()));
  }

  return QFrame::event(event);
}

void Notificator::notify(const QIcon& icon, const QString& title, const QString& message) {
  hide();
  d->initialize(icon, title, message);
  correctPosition();
  show();
}

void Notificator::initializeLayout() {
  QGridLayout* layout = new QGridLayout(this);
  layout->setHorizontalSpacing(12);
  layout->addWidget(d->icon(), 0, 0, 2, 1, Qt::AlignTop);
  layout->addWidget(d->title(), 0, 1);
  layout->addWidget(d->preloader(), 0, 2, 1, 1, Qt::AlignRight);
  layout->addWidget(d->message(), 1, 1, 1, 2);
  layout->addWidget(d->progress(), 2, 1, 1, 2);
}

void Notificator::initializeUI() {
  QPalette palette = this->palette();
  palette.setColor(QPalette::Base, Qt::red);
  palette.setColor(QPalette::AlternateBase, Qt::green);
  setPalette(palette);

  setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_Hover, true);
  setStyleSheet(
      "Notificator { background-color: black; border: none; }"
      "QLabel { color: white; }"
      "QLabel#title { font-weight: bold; }"
      "QProgressBar { border: 1px solid black; text-align: top; background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fff, stop: 0.4999 "
      "#eee, stop: 0.5 #ddd, stop: 1 #eee ); }"
      "QProgressBar::chunk { background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #b5e2f9, stop: 0.4999 #68c2f3, stop: 0.5 #67bff0, "
      "stop: 1 #1e9bda );  }"
      //				"QProgressBar::chunk { background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #9bd66d, stop: 0.4999
      // #81d142, stop: 0.5 #81d143, stop: 1 #58bf08 );  }"
  );
  setAutoFillBackground(true);
  setWindowOpacity(WINDOW_TRANSPARENT_OPACITY);
}

void Notificator::correctPosition() {
  // Вычисляем позицию для отображения уведомления
  // ... сперва сформируем позицию для самой верхней точки
  QRect notificationGeometry = QGuiApplication::screens()[0]->geometry();
  const QSize notificationSize = sizeHint();

  if (DISPLAY_NOTIFICATION_DIRECTION_TOP_TO_BOTTOM) {  // Notifications Display From Top to Bottom
    notificationGeometry.setTop(notificationGeometry.top());
    notificationGeometry.setLeft(notificationGeometry.right() - notificationSize.width());
    // ... определяем доступную верхнюю координату
    foreach (Notificator* instance, instances) {
      if (instance != this) {
        if (instance->geometry().bottom() > notificationGeometry.top()) {
          notificationGeometry.setTop(instance->geometry().bottom() + NOTIFICATION_MARGIN);
        }
      }
    }
  } else {  // Notifications Display From Bottom to Top
    notificationGeometry.setTop(notificationGeometry.bottom() - notificationSize.height());
    notificationGeometry.setLeft(notificationGeometry.right() - notificationSize.width());
    // ... определяем доступную верхнюю координату
    foreach (Notificator* instance, instances) {
      if (instance != this) {
        if (instance->geometry().top() < notificationGeometry.bottom()) {
          notificationGeometry.setTop(instance->geometry().top() - notificationSize.height() - NOTIFICATION_MARGIN);
        }
      }
    }
  }

  // Устанавливаем размер
  notificationGeometry.setSize(notificationSize);
  // Отображаем
  setGeometry(notificationGeometry);
}

QList<Notificator*> Notificator::instances;
void Notificator::configureInstance(Notificator* notificator) {
  // Удаляем уже отработавшие уведомления
  QMutableListIterator<Notificator*> iter(instances);
  while (iter.hasNext()) {
    Notificator* instance = iter.next();
    if (instance->isHidden()) {
      iter.remove();
      instance = 0;
    }
  }

  // Добавляем новое уведомление к списку всех уведомлений
  if (notificator != 0) {
    instances.append(notificator);
  }
}

// #define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE
#include <QToolBar>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  QToolBar w;
  static const QIcon icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation);
  static constexpr int BATCH_SIZE = 10;
  for (int i = QStyle::StandardPixmap::SP_TitleBarMenuButton;  //
       i <= QStyle::StandardPixmap::SP_RestoreDefaultsButton;  //
       ++i) {
    w.addAction(qApp->style()->standardIcon((QStyle::StandardPixmap)i), QString::number(i));
    if (i % BATCH_SIZE == 0) {
      w.addSeparator();
    }
  }
  w.setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  w.show();
  a.exec();
  return 0;
}

#endif
