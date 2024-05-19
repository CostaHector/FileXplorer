#ifndef DISPLAYENHANCEMENT_H
#define DISPLAYENHANCEMENT_H
#include <QString>
#include <QTime>

namespace FILE_PROPERTY_DSP {
const QString FILE_SIZE_GKMB_TEMPLATE{"%1'%2'%3'%4"};
const QString FILE_SIZE_DETAIL_TEMPLATE{"%1 (%2 Bytes)"};
inline QString sizeToHumanReadFriendly(qint64 total) {
  const qint64 xGiB = total / (1 << 30);
  const qint64 xMiB = total % (1 << 30) / (1 << 20);
  const qint64 xkiB = total % (1 << 30) % (1 << 20) / (1 << 10);
  const qint64 xB = total % (1 << 30) % (1 << 20) % (1 << 10);
  return FILE_SIZE_GKMB_TEMPLATE.arg(xGiB).arg(xMiB).arg(xkiB).arg(xB);
}
inline QString sizeToFileSizeDetail(qint64 total) {
  return FILE_SIZE_DETAIL_TEMPLATE.arg(sizeToHumanReadFriendly(total)).arg(total);
}
inline QString durationToHumanReadFriendly(qint64 ms) {
  return QTime::fromMSecsSinceStartOfDay(ms).toString(Qt::ISODateWithMs);
}

}  // namespace FILE_PROPERTY_DSP
#endif
