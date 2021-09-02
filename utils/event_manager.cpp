#include "event_manager.h"
#include <list>
#include <unordered_map>
#include <iostream>
#include <GLFW/glfw3.h>
#include <set>

namespace vk
{
namespace su
{

KeyCode translate_key_code(int key)
{
	static const std::unordered_map<int, KeyCode> key_lookup =
	    {
	        {GLFW_KEY_SPACE, KeyCode::Space},
	        {GLFW_KEY_APOSTROPHE, KeyCode::Apostrophe},
	        {GLFW_KEY_COMMA, KeyCode::Comma},
	        {GLFW_KEY_MINUS, KeyCode::Minus},
	        {GLFW_KEY_PERIOD, KeyCode::Period},
	        {GLFW_KEY_SLASH, KeyCode::Slash},
	        {GLFW_KEY_0, KeyCode::_0},
	        {GLFW_KEY_1, KeyCode::_1},
	        {GLFW_KEY_2, KeyCode::_2},
	        {GLFW_KEY_3, KeyCode::_3},
	        {GLFW_KEY_4, KeyCode::_4},
	        {GLFW_KEY_5, KeyCode::_5},
	        {GLFW_KEY_6, KeyCode::_6},
	        {GLFW_KEY_7, KeyCode::_7},
	        {GLFW_KEY_8, KeyCode::_8},
	        {GLFW_KEY_9, KeyCode::_9},
	        {GLFW_KEY_SEMICOLON, KeyCode::Semicolon},
	        {GLFW_KEY_EQUAL, KeyCode::Equal},
	        {GLFW_KEY_A, KeyCode::A},
	        {GLFW_KEY_B, KeyCode::B},
	        {GLFW_KEY_C, KeyCode::C},
	        {GLFW_KEY_D, KeyCode::D},
	        {GLFW_KEY_E, KeyCode::E},
	        {GLFW_KEY_F, KeyCode::F},
	        {GLFW_KEY_G, KeyCode::G},
	        {GLFW_KEY_H, KeyCode::H},
	        {GLFW_KEY_I, KeyCode::I},
	        {GLFW_KEY_J, KeyCode::J},
	        {GLFW_KEY_K, KeyCode::K},
	        {GLFW_KEY_L, KeyCode::L},
	        {GLFW_KEY_M, KeyCode::M},
	        {GLFW_KEY_N, KeyCode::N},
	        {GLFW_KEY_O, KeyCode::O},
	        {GLFW_KEY_P, KeyCode::P},
	        {GLFW_KEY_Q, KeyCode::Q},
	        {GLFW_KEY_R, KeyCode::R},
	        {GLFW_KEY_S, KeyCode::S},
	        {GLFW_KEY_T, KeyCode::T},
	        {GLFW_KEY_U, KeyCode::U},
	        {GLFW_KEY_V, KeyCode::V},
	        {GLFW_KEY_W, KeyCode::W},
	        {GLFW_KEY_X, KeyCode::X},
	        {GLFW_KEY_Y, KeyCode::Y},
	        {GLFW_KEY_Z, KeyCode::Z},
	        {GLFW_KEY_LEFT_BRACKET, KeyCode::LeftBracket},
	        {GLFW_KEY_BACKSLASH, KeyCode::Backslash},
	        {GLFW_KEY_RIGHT_BRACKET, KeyCode::RightBracket},
	        {GLFW_KEY_GRAVE_ACCENT, KeyCode::GraveAccent},
	        {GLFW_KEY_ESCAPE, KeyCode::Escape},
	        {GLFW_KEY_ENTER, KeyCode::Enter},
	        {GLFW_KEY_TAB, KeyCode::Tab},
	        {GLFW_KEY_BACKSPACE, KeyCode::Backspace},
	        {GLFW_KEY_INSERT, KeyCode::Insert},
	        {GLFW_KEY_DELETE, KeyCode::DelKey},
	        {GLFW_KEY_RIGHT, KeyCode::Right},
	        {GLFW_KEY_LEFT, KeyCode::Left},
	        {GLFW_KEY_DOWN, KeyCode::Down},
	        {GLFW_KEY_UP, KeyCode::Up},
	        {GLFW_KEY_PAGE_UP, KeyCode::PageUp},
	        {GLFW_KEY_PAGE_DOWN, KeyCode::PageDown},
	        {GLFW_KEY_HOME, KeyCode::Home},
	        {GLFW_KEY_END, KeyCode::End},
	        {GLFW_KEY_CAPS_LOCK, KeyCode::CapsLock},
	        {GLFW_KEY_SCROLL_LOCK, KeyCode::ScrollLock},
	        {GLFW_KEY_NUM_LOCK, KeyCode::NumLock},
	        {GLFW_KEY_PRINT_SCREEN, KeyCode::PrintScreen},
	        {GLFW_KEY_PAUSE, KeyCode::Pause},
	        {GLFW_KEY_F1, KeyCode::F1},
	        {GLFW_KEY_F2, KeyCode::F2},
	        {GLFW_KEY_F3, KeyCode::F3},
	        {GLFW_KEY_F4, KeyCode::F4},
	        {GLFW_KEY_F5, KeyCode::F5},
	        {GLFW_KEY_F6, KeyCode::F6},
	        {GLFW_KEY_F7, KeyCode::F7},
	        {GLFW_KEY_F8, KeyCode::F8},
	        {GLFW_KEY_F9, KeyCode::F9},
	        {GLFW_KEY_F10, KeyCode::F10},
	        {GLFW_KEY_F11, KeyCode::F11},
	        {GLFW_KEY_F12, KeyCode::F12},
	        {GLFW_KEY_KP_0, KeyCode::KP_0},
	        {GLFW_KEY_KP_1, KeyCode::KP_1},
	        {GLFW_KEY_KP_2, KeyCode::KP_2},
	        {GLFW_KEY_KP_3, KeyCode::KP_3},
	        {GLFW_KEY_KP_4, KeyCode::KP_4},
	        {GLFW_KEY_KP_5, KeyCode::KP_5},
	        {GLFW_KEY_KP_6, KeyCode::KP_6},
	        {GLFW_KEY_KP_7, KeyCode::KP_7},
	        {GLFW_KEY_KP_8, KeyCode::KP_8},
	        {GLFW_KEY_KP_9, KeyCode::KP_9},
	        {GLFW_KEY_KP_DECIMAL, KeyCode::KP_Decimal},
	        {GLFW_KEY_KP_DIVIDE, KeyCode::KP_Divide},
	        {GLFW_KEY_KP_MULTIPLY, KeyCode::KP_Multiply},
	        {GLFW_KEY_KP_SUBTRACT, KeyCode::KP_Subtract},
	        {GLFW_KEY_KP_ADD, KeyCode::KP_Add},
	        {GLFW_KEY_KP_ENTER, KeyCode::KP_Enter},
	        {GLFW_KEY_KP_EQUAL, KeyCode::KP_Equal},
	        {GLFW_KEY_LEFT_SHIFT, KeyCode::LeftShift},
	        {GLFW_KEY_LEFT_CONTROL, KeyCode::LeftControl},
	        {GLFW_KEY_LEFT_ALT, KeyCode::LeftAlt},
	        {GLFW_KEY_RIGHT_SHIFT, KeyCode::RightShift},
	        {GLFW_KEY_RIGHT_CONTROL, KeyCode::RightControl},
	        {GLFW_KEY_RIGHT_ALT, KeyCode::RightAlt},
	    };

	auto key_it = key_lookup.find(key);

	if (key_it == key_lookup.end())
	{
		return KeyCode::Unknown;
	}

	return key_it->second;
}

KeyAction translate_key_action(int action)
{
	if (action == GLFW_PRESS)
	{
		return KeyAction::Down;
	}
	else if (action == GLFW_RELEASE)
	{
		return KeyAction::Up;
	}
	else if (action == GLFW_REPEAT)
	{
		return KeyAction::Repeat;
	}

	return KeyAction::Unknown;
}

std::list<KeyBoardEvent>& eventList()
{
    static std::list<KeyBoardEvent> eventList;
    return eventList;
}

void keyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    std::cout << "key["<< key << "] action: " << action << std::endl;
    eventList().push_back(KeyBoardEvent{
        translate_key_code(key),
        translate_key_action(action)});
}

bool handleExit(const std::list<KeyBoardEvent>& eventList)
{
    for(const auto& e: eventList)
    {
        if(e.key == KeyCode::Escape) return true;
    }
    return false;
}
glm::mat4x4 handleMotion(std::list<KeyBoardEvent>& eventList)
{
    std::set<KeyCode> relatedKeys{KeyCode::A, KeyCode::S, KeyCode::W, KeyCode::D, KeyCode::Up, KeyCode::Down, KeyCode::Left, KeyCode::Right};
    glm::mat4x4 tf(1.f);
    const float ROT_STEP = 0.1;
    const float TRA_STEP = 0.5;
    for(auto it = eventList.begin(); it != eventList.end(); )
    {
        if(0 == relatedKeys.count(it->key) ||
            (it->action != KeyAction::Down && it->action != KeyAction::Repeat))
        {
            it++;
            continue;
        }

        glm::mat4x4 localTf(1.f);
        if(it->key == KeyCode::Up)
        {
            localTf = glm::mat4_cast(glm::angleAxis(ROT_STEP, glm::vec3(1,0,0)));
        }else if(it->key == KeyCode::Down)
        {
            localTf = glm::mat4_cast(glm::angleAxis(ROT_STEP, glm::vec3(-1,0,0)));
        }else if(it->key == KeyCode::Left)
        {
            localTf = glm::mat4_cast(glm::angleAxis(ROT_STEP, glm::vec3(0,1,0)));
        }else if(it->key == KeyCode::Right)
        {
            localTf = glm::mat4_cast(glm::angleAxis(ROT_STEP, glm::vec3(0,-1,0)));
        }else if(it->key == KeyCode::W)
        {
            localTf[3][2] = TRA_STEP;
        }else if(it->key == KeyCode::S)
        {
            localTf[3][2] = -TRA_STEP;
        }else if(it->key == KeyCode::A)
        {
            localTf[3][0] = TRA_STEP;
        }else if(it->key == KeyCode::D)
        {
            localTf[3][0] = -TRA_STEP;
        }
        tf = tf * localTf;
        it = eventList.erase(it);
    }
    return tf;
}

} // namespace su

} // namespace vk

