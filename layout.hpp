#pragma once

#include "includes.hpp"
#include "surface.hpp"
#include "output.hpp"
#include "titlebar.hpp"
#include <memory>

namespace Layout {

    void addSurface(Surface::Toplevel* surface);
    void removeSurface(Surface::Toplevel* surface);
    void handleCursorMovement(const double x, const double y);
    void setDesktop(uint desktop);

    class Base;

    //logical representation of one or multiple mirrored monitors
    class Display {
        Extends extends;
        
        public:
        Display(Extends ext);
        ~Display();
        void updateExtends(Extends ext);
        bool contains(const double x, const double y);

        std::unique_ptr<Base> node;
        Titlebar titlebar;
    };
}