// SPDX-License-Identifier: MIT
#define DT_DRV_COMPAT zmk_behavior_alt_tab

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <zmk/keys.h>
#include <zmk/hid.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// Structure to hold the state for each instance of the behavior
struct behavior_alt_tab_data {
    bool alt_pressed;        // Tracks if we have programmatically pressed Alt
    struct k_timer release_timer; // Timer for delayed Alt release
    uint32_t position;       // Keep track of the key position
};

// Timer handler function - called when the release timer expires
static void alt_tab_release_timer_handler(struct k_timer *timer) {
    struct behavior_alt_tab_data *data = CONTAINER_OF(timer, struct behavior_alt_tab_data, release_timer);

    LOG_DBG("Timer expired for position %d", data->position);
    if (data->alt_pressed) {
        LOG_DBG("Releasing LALT for position %d", data->position);
        zmk_keymap_release(data->position, K_LALT); // Use ZMK's function to handle release
        data->alt_pressed = false;
    }
}

// Behavior: Key Press Handler
static int behavior_alt_tab_on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                                     const struct zmk_behavior_binding_event *event) {
    const struct device *dev = zmk_behavior_get_device(binding->behavior_dev);
    struct behavior_alt_tab_data *data = dev->data;
    int32_t release_delay_ms = DT_INST_PROP_OR(event->position, release_ms, 250); // Get delay from DT

    LOG_DBG("Pressed '%s' at position %d, alt_pressed: %d", binding->behavior_dev->name, event->position, data->alt_pressed);

    // Stop any pending release timer if the key is pressed again
    k_timer_stop(&data->release_timer);
    data->position = event->position; // Update position just in case

    if (data->alt_pressed) {
        // Alt is already held down (by us), just send Tab
        LOG_DBG("Sending TAB for position %d", event->position);
        zmk_keymap_press(event->position, K_TAB);
        zmk_keymap_release(event->position, K_TAB);
    } else {
        // First press: Send Alt + Tab
        LOG_DBG("Sending LALT + TAB for position %d", event->position);
        // Use zmk_keymap_press/release for proper modifier handling
        zmk_keymap_press(event->position, K_LALT);
        data->alt_pressed = true; // Mark Alt as pressed by this behavior
        zmk_keymap_press(event->position, K_TAB);
        zmk_keymap_release(event->position, K_TAB);
        // NOTE: We keep LALT pressed
    }

    return ZMK_BEHAVIOR_OPAQUE; // Prevent default handling
}
// Behavior: Key Release Handler
static int behavior_alt_tab_on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                                      const struct zmk_behavior_binding_event *event) {
    const struct device *dev = zmk_behavior_get_device(binding->behavior_dev);
    struct behavior_alt_tab_data *data = dev->data;
    int32_t release_delay_ms = DT_INST_PROP_OR(event->position, release_ms, 250); // Get delay from DT

    LOG_DBG("Released '%s' at position %d, alt_pressed: %d", binding->behavior_dev->name, event->position, data->alt_pressed);

    if (data->alt_pressed) {
        // If Alt is currently held by us, start the timer to release it
        LOG_DBG("Starting release timer (%dms) for position %d", release_delay_ms, event->position);
        k_timer_start(&data->release_timer, K_MSEC(release_delay_ms), K_NO_WAIT);
    }

    return ZMK_BEHAVIOR_OPAQUE; // Prevent default handling
}

// Behavior Operations Structure
static const struct behavior_driver_api behavior_alt_tab_driver_api = {
    .binding_pressed = behavior_alt_tab_on_keymap_binding_pressed,
    .binding_released = behavior_alt_tab_on_keymap_binding_released,
    // .locality = BEHAVIOR_LOCALITY_EVENT_SOURCE // Or GLOBAL depending on exact needs, EVENT_SOURCE is usually fine
};

// Behavior Initialization Function
static int behavior_alt_tab_init(const struct device *dev) {
    struct behavior_alt_tab_data *data = dev->data;
    data->alt_pressed = false;
    // Initialize the timer
    k_timer_init(&data->release_timer, alt_tab_release_timer_handler, NULL);
    LOG_DBG("Initialized alt_tab behavior %s", dev->name);
	return 0;
};

// Instantiate the behavior data
struct behavior_alt_tab_data behavior_alt_tab_data;

// Define the behavior device
BEHAVIOR_DT_INST_DEFINE(0,                    // Instance number (usually 0 for single instance)
                        behavior_alt_tab_init, // Init function
                        NULL,                 // PM function (optional)
                        &behavior_alt_tab_data, // Data structure instance
                        NULL,                 // Config structure (optional)
                        POST_KERNEL,          // Initialization priority
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, // Sub-priority
                        &behavior_alt_tab_driver_api); // API structure)
