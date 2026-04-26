#ifndef VIDEOPLAYERKEY_H
#define VIDEOPLAYERKEY_H

#include "KV.h"

namespace VideoPlayerKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV VOLUME{"VideoPlayerKey/VOLUME", Var{100}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};
constexpr KV MUTE{"VideoPlayerKey/MUTE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV PLAYBACK_MODE{"VideoPlayerKey/PLAYBACK_MODE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};
constexpr KV PLAYBACK_TRIGGER_MODE{"VideoPlayerKey/PLAYBACK_TRIGGER_MODE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};
constexpr KV AUTO_HIDE_TOOLBAR{"VideoPlayerKey/AUTO_HIDE_TOOLBAR", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV DUPLICATE_FINDER_DEVIATION_DURATION{"VideoPlayerKey/DUPLICATE_FINDER_DEVIATION_DURATION", Var{2 * 1000}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, INT_MAX>};   // 2s ~ 20s
constexpr KV DUPLICATE_FINDER_DEVIATION_FILESIZE{"VideoPlayerKey/DUPLICATE_FINDER_DEVIATION_FILESIZE", Var{2 * 1024}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, INT_MAX>}; // 2kB ~ 30MB

constexpr KV RATE_MOVIE_DEFAULT_VALUE{"VideoPlayerKey/RATE_MOVIE_DEFAULT_VALUE", Var{5}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 10>};
} // namespace VideoPlayerKey

#endif // VIDEOPLAYERKEY_H
