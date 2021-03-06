#ifndef IJENGINE_JOYSTICK_EVENT_H
#define IJENGINE_JOYSTICK_EVENT_H

#include "event.h"

#define JOYSTICK_EVENT_ID 0x04

#include <sstream>

using std::ostringstream;

namespace ijengine {

    class JoystickEvent : public Event {
    public:
        typedef enum {BUTTON_PRESSED, BUTTON_RELEASED, AXIS_MOTION} State;

        typedef
        enum {
	    BUTTON_INVALID, A, B, X, Y, BACK, GUIDE, START, 
	    LEFTSTICK, RIGHTSTICK, LEFTSHOULDER, RIGHTSHOULDER,
	    DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT, BUTTON_MAX
             } Button;

	typedef
	enum {
	    AXIS_INVALID, LEFTX, LEFTY, RIGHTX, RIGHTY,
	    TRIGGERLEFT, TRIGGERRIGHT, AXIS_MAX
	     } Axis;

        JoystickEvent(unsigned t, State s, Button b, Axis a) :
            Event(t), m_state(s), m_button(b), m_axis(a) {}

        State state() const { return m_state; }
        Button button() const { return m_button; }
	Axis axis() const { return m_axis; }

    private:
        State m_state;
        Button m_button;
	Axis m_axis;
    };
}

#endif
