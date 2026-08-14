#pragma once

#include "InputEnum.h"
#include "VulkanWindow.h"

#ifndef __ANDROID__
class GameController
{
public:
    static GameController& Get();

private:
	GameController() = default;
    ~GameController() = default;
	GameController(const GameController&) = delete;
	GameController& operator=(const GameController&) = delete;
	GameController(GameController&&) = delete;
	GameController& operator=(GameController&&) = delete;

	static constexpr float Sensitivity = 0.1f;
	GLFWgamepadstate GamePadState[4] = { };

public:

	bool ButtonPressed(int controllerId, int button);
	vec2 LeftJoyStickMoved(int controllerId);
	vec2 RightJoyStickMoved(int controllerId);
	vec2 R2L2Pressed(int controllerId);
	GLFWgamepadstate GetGamePadState() { return GamePadState[0]; }
};
extern GameController& gameController;
inline GameController& GameController::Get()
{
    static GameController instance;
    return instance;
}
#endif