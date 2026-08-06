#pragma once

#include <memory>

#include <events.hpp>

class IWindowManager {
public:
    virtual bool createWindow()      = 0;
    virtual void update()            = 0;
    virtual void renderToWindow()    = 0;
    virtual bool shouldWindowClose() = 0;
    virtual ~IWindowManager()        = default;

    virtual void newImGuiFrame() = 0;
    virtual void toggleMouse()   = 0;
    virtual float getTime()      = 0;

private:
};

std::unique_ptr<IWindowManager> makeGLFWWindowManager(int window_width, int window_height,
                                                      std::shared_ptr<EventManager> event_manager);