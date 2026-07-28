#include "core/StateMachine.h"

//=============================================================================
// Public Methods
//=============================================================================

State StateMachine::currentState() const {
	return m_currentState;
}

void StateMachine::setState(State state) {
	m_currentState = state;
}
