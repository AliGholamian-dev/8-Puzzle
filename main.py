import sys
import os
import ctypes
import numpy as np
from PyQt5 import uic, QtCore
from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox

lib_path = os.path.join(os.getcwd(), "obj/libpuzzle.dll")
lib = ctypes.cdll.LoadLibrary(lib_path)
solve_from_cpp = lib.solve_for_Python
solve_from_cpp.argtypes = [
    np.ctypeslib.ndpointer(dtype=np.int, ndim=1, flags="C"),
    np.ctypeslib.ndpointer(dtype=np.int, ndim=1, flags="C"),
    np.ctypeslib.ndpointer(dtype=np.int, ndim=1, flags="C"),
    ctypes.POINTER(ctypes.c_int),
]
solve_from_cpp.restype = np.ctypeslib.ndpointer(dtype=np.int, ndim=1, flags="C")
Form = uic.loadUiType(os.path.join(os.getcwd(), "Resources/eight_Puzzle.ui"))[0]
Form_setStates = uic.loadUiType(os.path.join(os.getcwd(), "Resources/setStates.ui"))[0]


class SetStatesWindow(QMainWindow, Form_setStates):
    update_trigger = QtCore.pyqtSignal(np.ndarray, np.ndarray, np.ndarray)

    def __init__(self):
        super(SetStatesWindow, self).__init__()
        self.setupUi(self)
        self.returned_settings = np.array([0, 100])
        self.error_dialog = QMessageBox()
        self.error_dialog.setWindowTitle("Hey User")
        self.init_tiles = [
            self.init_lineEdit_1,
            self.init_lineEdit_2,
            self.init_lineEdit_3,
            self.init_lineEdit_4,
            self.init_lineEdit_5,
            self.init_lineEdit_6,
            self.init_lineEdit_7,
            self.init_lineEdit_8,
            self.init_lineEdit_0,
        ]
        self.goal_tiles = [
            self.goal_lineEdit_1,
            self.goal_lineEdit_2,
            self.goal_lineEdit_3,
            self.goal_lineEdit_4,
            self.goal_lineEdit_5,
            self.goal_lineEdit_6,
            self.goal_lineEdit_7,
            self.goal_lineEdit_8,
            self.goal_lineEdit_0,
        ]
        self.RandInit.clicked.connect(self.generate_rand_puzzle_init)
        self.RandGoal.clicked.connect(self.generate_rand_puzzle_goal)
        self.updateButton.clicked.connect(self.checkSates)
        self.AlgoSelector.currentIndexChanged.connect(self.AlgoUpdate)

    def __solvable(self):
        inv_count = 0
        inv_count_goal = 0
        for i in range(9):
            for j in range(i + 1, 9):
                if (
                    self.init_tiles[i].text() != "0"
                    and self.init_tiles[j].text() != "0"
                    and self.init_tiles[i].text() > self.init_tiles[j].text()
                ):
                    inv_count = inv_count + 1
        for i in range(9):
            for j in range(i + 1, 9):
                if (
                    self.goal_tiles[i].text() != "0"
                    and self.goal_tiles[j].text() != "0"
                    and self.goal_tiles[i].text() > self.goal_tiles[j].text()
                ):
                    inv_count_goal = inv_count_goal + 1
        return inv_count % 2 == inv_count_goal % 2

    def generate_rand_puzzle_init(self):
        while True:
            a = np.random.choice(9, 9, replace=False)
            for i in range(9):
                self.init_tiles[i].setText(f"{a[i]}")
            if self.__solvable():
                break

    def generate_rand_puzzle_goal(self):
        while True:
            a = np.random.choice(9, 9, replace=False)
            for i in range(9):
                self.goal_tiles[i].setText(f"{a[i]}")
            if self.__solvable():
                break

    def AlgoUpdate(self):
        self.returned_settings[0] = self.AlgoSelector.currentIndex()
        if self.returned_settings[0] == 2:
            self.DLSDepthSelector.setEnabled(True)
        else:
            self.DLSDepthSelector.setDisabled(True)

    def checkSates(self):
        for i in range(9):
            if not (
                self.init_tiles[i].text().isdigit()
                and self.goal_tiles[i].text().isdigit()
            ):
                self.error_dialog.setText("Are you out of your mind?")
                self.error_dialog.show()
                break
        else:
            returned_init = np.array([int(i.text()) for i in self.init_tiles])
            returned_goal = np.array([int(i.text()) for i in self.goal_tiles])
            self.returned_settings[1] = self.DLSDepthSelector.value()
            if len(returned_init) == len(set(returned_init)) and len(
                returned_goal
            ) == len(set(returned_goal)):
                if not self.__solvable():
                    self.error_dialog.setText("Sorry but this puzzle is Not Solvable")
                    self.error_dialog.show()
                else:
                    self.update_trigger.emit(
                        returned_init, returned_goal, self.returned_settings
                    )
            else:
                self.error_dialog.setText("You entered Repetetive Number")
                self.error_dialog.show()


class EightPuzzleSolver(QMainWindow, Form):
    def __init__(self):
        super(EightPuzzleSolver, self).__init__()
        self.setupUi(self)
        self.error_dialog = QMessageBox()
        self.error_dialog.setWindowTitle("Attention Please")
        self.tiles = np.array(
            [
                self.puzzleTile_1,
                self.puzzleTile_2,
                self.puzzleTile_3,
                self.puzzleTile_4,
                self.puzzleTile_5,
                self.puzzleTile_6,
                self.puzzleTile_7,
                self.puzzleTile_8,
                self.puzzleTile_0,
            ]
        )
        self.initial_state = np.array([1, 2, 3, 4, 5, 6, 7, 8, 0])
        self.goal_state = np.array([1, 2, 3, 4, 5, 6, 7, 8, 0])
        self.cppSettings = np.array([0, 100])
        self.step = 0
        self.moves = None
        self.backup_pos = np.zeros((9, 2))
        self.firstTime = True
        self.actionSet_States.triggered.connect(self.showSetting)
        self.startButton.clicked.connect(self.start_solve)
        self.stopButton.clicked.connect(self.stop_animate)
        self.rightButton.clicked.connect(self.stepRight)
        self.leftButton.clicked.connect(self.stepLeft)
        self.stopButton.setEnabled(False)
        self.rightButton.setEnabled(False)
        self.leftButton.setEnabled(False)
        self.thread = None
        self.animation = None

    def update_states(self, init_puzzle, goal_puzzle, returned_settings):
        if not self.startButton.isEnabled():
            return
        if self.firstTime:
            for i in range(9):
                self.backup_pos[i][0] = self.tiles[i].x()
                self.backup_pos[i][1] = self.tiles[i].y()
        self.firstTime = False
        for i in range(9):
            self.tiles[init_puzzle[i] - 1 if init_puzzle[i] != 0 else 8].move(
                int(self.backup_pos[i][0]), int(self.backup_pos[i][1])
            )
        self.initial_state = init_puzzle
        self.goal_state = goal_puzzle
        self.cppSettings = returned_settings

    def showSetting(self):
        self.w_states = SetStatesWindow()
        self.w_states.update_trigger.connect(self.update_states)
        self.w_states.show()

    def warn_user(self, mode):
        self.thread = None
        self.startButton.setEnabled(True)
        if mode == -1:
            self.error_dialog.setText("Max Depth reached but no answers found")
            self.error_dialog.show()
        else:
            self.error_dialog.setText("I did my best but Sorry No Answers!!!!")
            self.error_dialog.show()

    def consecutive_animation(self):
        if self.step == len(self.moves):
            self.rightButton.setEnabled(True)
            self.leftButton.setEnabled(True)
            self.startButton.setEnabled(True)
            return
        index = self.moves[self.step] - 1 if self.moves[self.step] != 0 else 8
        self.animation = QtCore.QPropertyAnimation(self.tiles[index], b"pos")
        self.animation.setDuration(self.spinBox.value())
        self.animation.setStartValue(QtCore.QPoint(self.tiles[index].pos()))
        self.animation.setEndValue(QtCore.QPoint(self.puzzleTile_0.pos()))
        self.puzzleTile_0.move(self.tiles[index].x(), self.tiles[index].y())
        self.step = self.step + 1
        self.animation.finished.connect(self.consecutive_animation)
        self.animation.start()

    def start_animate(self, moves_arr):
        self.thread = None
        if len(moves_arr) == 0:
            self.startButton.setEnabled(True)
            return
        self.moves = moves_arr
        self.stopButton.setEnabled(True)
        self.rightButton.setEnabled(False)
        self.leftButton.setEnabled(False)
        self.consecutive_animation()

    def start_solve(self):
        if self.thread:
            return
        self.step = 0
        self.stopButton.setEnabled(False)
        self.rightButton.setEnabled(False)
        self.leftButton.setEnabled(False)
        self.update_states(self.initial_state, self.goal_state, self.cppSettings)
        self.startButton.setEnabled(False)
        self.thread = dataFetchThread(
            self,
            cpp_init=self.initial_state,
            cpp_goal=self.goal_state,
            cpp_algo=self.cppSettings,
        )
        self.thread.fetched_trigger.connect(self.start_animate)
        self.thread.error_trigger.connect(self.warn_user)
        self.thread.start()

    def stop_animate(self):
        self.stopButton.setEnabled(False)
        self.rightButton.setEnabled(True)
        self.leftButton.setEnabled(True)
        self.startButton.setEnabled(True)
        self.animation.setDuration(0)
        self.animation.stop()

    def stepRight(self):
        if self.step >= len(self.moves):
            return
        index = self.moves[self.step] - 1 if self.moves[self.step] != 0 else 8
        bck_x = self.puzzleTile_0.x()
        bck_y = self.puzzleTile_0.y()
        self.puzzleTile_0.move(self.tiles[index].x(), self.tiles[index].y())
        self.tiles[index].move(bck_x, bck_y)
        self.step = self.step + 1

    def stepLeft(self):
        self.step = self.step - 1
        if self.step <= -1:
            self.step = 0
            return
        index = self.moves[self.step] - 1 if self.moves[self.step] != 0 else 8
        bck_x = self.puzzleTile_0.x()
        bck_y = self.puzzleTile_0.y()
        self.puzzleTile_0.move(self.tiles[index].x(), self.tiles[index].y())
        self.tiles[index].move(bck_x, bck_y)


class dataFetchThread(QtCore.QThread):
    fetched_trigger = QtCore.pyqtSignal(np.ndarray)
    error_trigger = QtCore.pyqtSignal(int)

    def __init__(self, window, cpp_init, cpp_goal, cpp_algo):
        QtCore.QThread.__init__(self, parent=window)
        self.parent = window
        self.c_init = cpp_init
        self.c_goal = cpp_goal
        self.c_algo = cpp_algo

    def run(self):
        size = 0
        _size = ctypes.c_int(size)
        pnt = solve_from_cpp(self.c_init, self.c_goal, self.c_algo, _size)
        size = _size.value
        newpnt = ctypes.cast(pnt, ctypes.POINTER(ctypes.c_int))
        result_of_cpp = np.ctypeslib.as_array(newpnt, (size,))
        if result_of_cpp[0] < 0:
            self.error_trigger.emit(result_of_cpp[0])
        elif result_of_cpp[size - 1] == 0:
            result_of_cpp = result_of_cpp[1 : size - 1]
            self.fetched_trigger.emit(result_of_cpp)
        else:
            result_of_cpp = result_of_cpp[1:]
            self.fetched_trigger.emit(result_of_cpp)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    w = EightPuzzleSolver()
    w.show()
    sys.exit(app.exec_())
