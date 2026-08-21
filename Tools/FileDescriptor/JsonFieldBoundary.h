#ifndef JSONFIELDBOUNDARY_H
#define JSONFIELDBOUNDARY_H

namespace JsonFieldBoundary {
constexpr int DURATION_INITIAL_VALUE = 0;  // need initial in movie table
constexpr int DURATION_GET_FAILED_VALUE = -1;

constexpr int SIZE_INITIAL_VALUE = 0; // need initial in movie table
constexpr int SIZE_GET_FAILED_VALUE = -1;

// never change MovieRateE value
enum MovieRateE {
  RATE_MIN_UNINITIALIZED_V = 0, // uninitialized, need initial in movie table
  RATE_MAX_V = 10,
  RATE_BUTT_V,
};
static_assert(JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V == 0, "Minumum rate value should be 0");
static_assert(JsonFieldBoundary::RATE_MAX_V == 10, "Maximum rate value should be 10");

inline int clampRate(int rate) {
  return (rate < RATE_MIN_UNINITIALIZED_V) ? RATE_MIN_UNINITIALIZED_V : (rate > RATE_MAX_V ? RATE_MAX_V : rate);
}

static constexpr int LOG_PER_CALC_FILE_FD = 500;
}

#endif // JSONFIELDBOUNDARY_H
