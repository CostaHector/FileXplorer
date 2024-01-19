#ifndef MP4DURATIONGETTER_H
#define MP4DURATIONGETTER_H
#include <QStringList>
#include <QList>

namespace MP4DurationGetter {

char* videoinfo_mem(char* haystack, unsigned int sizehaystack, char* needle, unsigned int sizeneedle);

void* videoinfo_find(char* filename, void* find, int size, int resultSize);

unsigned long videoinfo_flip(unsigned long val);

unsigned long videoinfo_duration(char* filename);

QList<unsigned long> getBatchVideosDuration(const QStringList& filepaths);

QString DisplayVideosDuration(const QList<unsigned long>& durationLst, const QStringList& fileNames, const QStringList& fileDirs);

QString DisplayVideosDuration(const QList<unsigned long>& durationLst, const QStringList& fileAbsPaths);

QString DisplayVideosDuration(const QStringList& fileAbsPaths);
}  // namespace MP4DurationGetter

#endif  // MP4DURATIONGETTER_H
