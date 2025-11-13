import sys

import os

from PySide2 import QtUiTools, QtGui

from PySide2.QtWidgets import QApplication, QMainWindow


class MainView(QMainWindow):

    def __init__(self):
        super().__init__()

        self.setupUI()

    def setupUI(self):
        global UI_set

        UI_set = QtUiTools.QUiLoader().load(resource_path("Matrix Calculator.ui"))

        UI_set.BTN_add.clicked.connect(self.add)




        self.setCentralWidget(UI_set)

        self.setWindowTitle("Matrix Calculator")

        self.setWindowIcon(QtGui.QPixmap(resource_path("./images/jbmpa.png")))

        self.resize(390, 600)

        self.show()

    def add(self):
        x = []
        x.append(int(UI_set.LE_item1.text()))
        x.append(int(UI_set.LE_item2.text()))
        x.append(int(UI_set.LE_item3.text()))
        x.append(int(UI_set.LE_item4.text()))
        x.append(int(UI_set.LE_item5.text()))
        x.append(int(UI_set.LE_item6.text()))
        x.append(int(UI_set.LE_item7.text()))
        x.append(int(UI_set.LE_item8.text()))
        x.append(int(UI_set.LE_item9.text()))
        y = []
        y.append(int(UI_set.LE2_item1.text()))
        y.append(int(UI_set.LE2_item2.text()))
        y.append(int(UI_set.LE2_item3.text()))
        y.append(int(UI_set.LE2_item4.text()))
        y.append(int(UI_set.LE2_item5.text()))
        y.append(int(UI_set.LE2_item6.text()))
        y.append(int(UI_set.LE2_item7.text()))
        y.append(int(UI_set.LE2_item8.text()))
        y.append(int(UI_set.LE2_item9.text()))



# 파일 경로

# pyinstaller로 원파일로 압축할때 경로 필요함

def resource_path(relative_path):
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)

    return os.path.join(os.path.abspath("."), relative_path)


if __name__ == '__main__':
    app = QApplication(sys.argv)

    main = MainView()

    # main.show()

    sys.exit(app.exec_())