#pragma once

#include "includes.hpp"
#include "surface.hpp"
#include <functional>

typedef std::function<void(xkb_keysym_t sym, uint32_t modmask)> ShortcutCallback;

namespace Input {
    extern wlr_seat* seat;

    void setup();
    void cleanup();

    void registerKeyCallback(xkb_keysym_t sym, uint32_t modmask, ShortcutCallback callback);
    void setCursorFocus(Surface::Base* surface);
    void setKeyboardFocus(Surface::Base* surface);
}