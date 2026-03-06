#ifndef PLAYMODECONTROL_H
#define PLAYMODECONTROL_H

namespace VideoPlayTool {
enum class PlaybackTriggerMode { MANUAL = 0, AUTO = 1, DISABLED = 2 };
constexpr PlaybackTriggerMode DEFAULT_PLAYBACK_TRIGGER_MODE{VideoPlayTool::PlaybackTriggerMode::MANUAL};
} // namespace VideoPlayTool

#endif // PLAYMODECONTROL_H
