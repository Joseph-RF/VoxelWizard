#pragma once

#include <queue>
#include <unordered_map>

enum class Action {
    SCREEN_RESIZE,
    MOVE_FORWARD,
    MOVE_LEFT,
    MOVE_BACKWARD,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    TURN,
    MOVE_CURSOR,
    ZOOM,
    L_CLICK,
    L_BUTTON_PRESSED,
    R_CLICK,
    R_BUTTON_PRESSED,
    TOGGLE_MOUSE,
    QUIT
};

// Consider moving this somewhere else
enum class Key { W, A, S, D, L_CTRL, SPACE, M, ESC, OTHER };

enum class MouseButton { LEFT, RIGHT, OTHER };

struct Event {
    Event(Action action, float mouse_xpos, float mouse_ypos, float scroll_x, float scroll_y);

    enum Action action;
    float mouse_xpos;
    float mouse_ypos;
    float scroll_x;
    float scroll_y;
};

class EventManager {
public:
    EventManager();

    void update();

    void addScreenResizeEvent(int width, int height);
    void keyEvent(Key key, bool pressed);
    void addMouseMovedEvent(float mouse_xpos, float mouse_ypos, bool turning);
    void addScrollEvent(float scroll_x, float scroll_y);
    void addMouseClickEvent(MouseButton button, bool pressed);

    std::unordered_map<Key, bool> pressed_keys;
    std::unordered_map<MouseButton, bool> pressed_buttons;
    std::queue<Event> events;
};
