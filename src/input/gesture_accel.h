#ifndef LEME_INPUT_GESTURE_ACCEL_H
#define LEME_INPUT_GESTURE_ACCEL_H

#include <stdbool.h>
#include <libinput.h>

struct leme_gesture_accel_ops;

struct leme_gesture_accel {
  struct libinput_device *device; /* owned ref while non-NULL */
  enum libinput_config_accel_profile saved_profile;
  double saved_speed;
  const struct leme_gesture_accel_ops *ops; /* borrowed static table */
};

bool leme_gesture_accel_begin(struct leme_gesture_accel *state,
                              struct libinput_device *device);
bool leme_gesture_accel_restore(struct leme_gesture_accel *state);

#endif
