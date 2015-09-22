from PySide2.QtCore import QStateMachine, QState

mach = QStateMachine()
state = QState(mach)
print(state.machine())
