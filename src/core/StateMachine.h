#pragma once

#include "core/State.h"

class StateMachine
{
public:
	State currentState() const;
	void setState(State state);

private:
	State m_currentState = State::IDLE;
};

