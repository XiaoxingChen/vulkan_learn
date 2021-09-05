#if !defined(_EVENT_MANAGER_H_)
#define _EVENT_MANAGER_H_

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <list>

struct GLFWwindow;
namespace vk
{
namespace su
{
enum class KeyCode
{
	Unknown,
	Space,
	Apostrophe, /* ' */
	Comma,      /* , */
	Minus,      /* - */
	Period,     /* . */
	Slash,      /* / */
	_0,
	_1,
	_2,
	_3,
	_4,
	_5,
	_6,
	_7,
	_8,
	_9,
	Semicolon, /* ; */
	Equal,     /* = */
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	LeftBracket,  /* [ */
	Backslash,    /* \ */
	RightBracket, /* ] */
	GraveAccent,  /* ` */
	Escape,
	Enter,
	Tab,
	Backspace,
	Insert,
	DelKey,
	Right,
	Left,
	Down,
	Up,
	PageUp,
	PageDown,
	Home,
	End,
	Back,
	CapsLock,
	ScrollLock,
	NumLock,
	PrintScreen,
	Pause,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	KP_0,
	KP_1,
	KP_2,
	KP_3,
	KP_4,
	KP_5,
	KP_6,
	KP_7,
	KP_8,
	KP_9,
	KP_Decimal,
	KP_Divide,
	KP_Multiply,
	KP_Subtract,
	KP_Add,
	KP_Enter,
	KP_Equal,
	LeftShift,
	LeftControl,
	LeftAlt,
	RightShift,
	RightControl,
	RightAlt
};

enum class KeyAction
{
	Down,
	Up,
	Repeat,
	Unknown
};
struct KeyBoardEvent
{
    KeyBoardEvent(KeyCode key_, KeyAction action_)
    :key(key_), action(action_){}
    KeyCode key = KeyCode::Unknown;
    KeyAction action = KeyAction::Unknown;
};
void keyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/);

glm::mat4x4 handleMotion(std::list<KeyBoardEvent>& eventList, const glm::mat4x4& prevPose);
bool handleExit(const std::list<KeyBoardEvent>& eventList);
std::list<KeyBoardEvent>& eventList();


// void
} // namespace su

} // namespace vk


#endif // _EVENT_MANAGER_H_
