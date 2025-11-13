import math
import sys

import os

import numpy as np
import PySide2
import numpy.linalg
from PySide2 import QtUiTools

from PySide2.QtWidgets import QApplication, QMainWindow, QGridLayout, QLineEdit

#환경 변수 설정 코드
dirname = os.path.dirname(PySide2.__file__)
plugin_path = os.path.join(dirname, 'plugins', 'platforms')
os.environ['QT_QPA_PLATFORM_PLUGIN_PATH'] = plugin_path


class MainView(QMainWindow):

    def __init__(self):
        super().__init__()

        self.mat1Grid = QGridLayout()
        self.mat2Grid = QGridLayout()
        self.resultGrid = QGridLayout()
        self.setupUI()
        self.mat1 = []
        self.mat2 = []
        self.resultMat = []

    def setupUI(self):
        global UI_set, UI_set2

        UI_set = QtUiTools.QUiLoader().load(resource_path("main.ui"))
        UI_set2 = QtUiTools.QUiLoader().load(resource_path("error.ui"))

        UI_set.mat1SetBtn.clicked.connect(self.setMat1)
        UI_set.mat1Frame.setLayout(self.mat1Grid)
        UI_set.mat2SetBtn.clicked.connect(self.setMat2)
        UI_set.mat2Frame.setLayout(self.mat2Grid)
        UI_set.resultFrame.setLayout(self.resultGrid)

        UI_set.computeBtn.clicked.connect(self.getResultMatrix)

        UI_set2.hideBtn.clicked.connect(self.ErrorWindowHide)

        self.setCentralWidget(UI_set)

        self.setWindowTitle("Matrix Calculator")

        self.resize(1200, 600)

        self.show()

    def setMatrix(self, matList, grid, col, row, readOnly=False): #행렬을 초기화하고 설정한 값으로 설정해주는 함수
        print("초기화 전", matList)
        # if len(matList) > 0:
        #     for i in range(len(matList) * len(matList[0])):
        #         print(i)
        #         print("grid 초기화", grid.itemAt(i).widget())
        #         grid.itemAt(i).widget().deleteLater()
        #     matList.clear()
        for iii in reversed(range(grid.count())):
            grid.itemAt(iii).widget().setParent(None)
        matList.clear()

        print("초기화 후", matList)

        for i in range(col):
            matRow = []
            for j in range(row):
                text = QLineEdit()
                text.setReadOnly(readOnly)
                text.setStyleSheet("font: 15px; font-weight: bold; font-family: Arial;")
                grid.addWidget(text, i, j)
                matRow.append(text)
            matList.append(matRow)

        print(matList)

    def setMat1(self):  # 행렬 1 설정 (좌측)
        col = int(UI_set.mat1col.currentText())
        row = int(UI_set.mat1row.currentText())
        print(col, row, self.mat1Grid)
        self.setMatrix(self.mat1, self.mat1Grid, col, row)

    def setMat2(self):  # 행렬 2 설정 (우측)
        col = int(UI_set.mat2col.currentText())
        row = int(UI_set.mat2row.currentText())
        print(col, row, self.mat2Grid)

        self.setMatrix(self.mat2, self.mat2Grid, col, row)

    def CheckMatrixOperator(self):  # 행렬 사이에 있는 라디오버튼을 읽어들여 연산자 검사
        if len(self.mat1) == 0 or len(self.mat2) == 0:
            return "err"

        mat1Col = len(self.mat1)
        mat1Row = len(self.mat1[0])
        mat2Col = len(self.mat2)
        mat2Row = len(self.mat2[0])

        if UI_set.sumBtn.isChecked():
            if not(mat1Col == mat2Col and mat1Row == mat2Row):
                return "err"
            else:
                return "sum"
        elif UI_set.multiBtn.isChecked():
            if not(mat1Row == mat2Col):
                return "err"
            else:
                return "multi"
        elif UI_set.boolBtn.isChecked():
            if not(mat1Row == mat2Col):
                return "err"
            else:
                return "boolmulti"
        elif UI_set.inverseBtn.isChecked():
            if not(mat1Row == mat1Col == mat2Row == mat2Col):
                return "err"
            else:
                return "inverse"

    def getResultMatrix(self):  # 결과 출력할 행렬 만드는 함수
        op = self.CheckMatrixOperator()
        if op == "err":
            self.ErrorWindowShow()
            return
        else:
            mat1Col = len(self.mat1)
            mat1Row = len(self.mat1[0])
            mat2Row = len(self.mat2[0])

            if op == "sum":
                self.setMatrix(self.resultMat, self.resultGrid, mat1Col, mat1Row, True)
                UI_set.sizeLbl.setText(str(mat1Col) + " X " + str(mat1Row))
                self.MatrixAdd()
            elif op == "multi":
                self.setMatrix(self.resultMat, self.resultGrid, mat1Col, mat2Row, True)
                UI_set.sizeLbl.setText(str(mat1Col) + " X " + str(mat2Row))
                self.MatrixMultiply()
            elif op == "boolmulti":
                self.setMatrix(self.resultMat, self.resultGrid, mat1Col, mat2Row, True)
                UI_set.sizeLbl.setText(str(mat1Col) + " X " + str(mat2Row))
                self.MatrixBoolMultiply()
            elif op == "inverse":
                self.setMatrix(self.resultMat, self.resultGrid, mat1Col, mat2Row, True)
                UI_set.sizeLbl.setText(str(mat1Col) + " X " + str(mat2Row))
                self.InverseMatrix()

    def ErrorWindowShow(self):
        UI_set2.show()

    def ErrorWindowHide(self):
        UI_set2.close()

    def MatrixAdd(self):  #행렬의 합을 계산하는 함수
        mat1Data = []   #첫 번째 행렬의 값들을 저장하는 리스트
        mat2Data = []   #두 번째 행렬의 값들을 저장하는 리스트
        AddResult = []  #첫 번재 행렬의 값들과 두 번재 행렬의 값들을 더한 결과를 저장하는 리스트
        count = 0     #AddResult의 요소에 접근하기 위한 인덱스로 사용될 변수
        err = 0       #에러가 났는지 확인하기 위한 변수
        for i in self.mat1:   #첫 번째 행렬의 QLineEdit에 적혀있는 값들을 받아와 mat1Data에 저장하기 위한 반복문
            for j in i:
                content = j.text()
                if content == '':  #사용자가 QLineEdit에 값을 적지 않고 빈칸으로 나뒀을 경우, 0을 대신 추가함.
                    mat1Data.append("0")
                else:
                    mat1Data.append(j.text())

        for i in self.mat2:   #두 번째 행렬의 QLineEdit에 적혀있는 값들을 받아와 mat2Data에 저장하기 위한 반복문
            for j in i:
                content = j.text()
                if content == '':   #사용자가 QLineEdit에 값을 적지 않고 빈칸으로 나뒀을 경우, 0을 대신 추가함.
                    mat2Data.append("0")
                else:
                    mat2Data.append(j.text())

        for i in range(len(mat1Data)):  #첫 번째 행렬과 두 번째 행렬의 값들을 계산하여 AddResult에 저장하기 위한 반복문
            try:  #사용자가 숫자가 아닌 문자를 입력했을 경우 등을 대비하는 try except 문
                if "." in mat1Data[i]:  #소수
                    x = float(mat1Data[i])
                elif "/" in mat1Data[i]:  #분수
                    fr = mat1Data[i].split('/')
                    x = int(fr[0]) / int(fr[1])
                else:  #정수
                    x = int(mat1Data[i])

                if "." in mat2Data[i]:  #소수
                    y = float(mat2Data[i])
                elif "/" in mat2Data[i]:  #분수
                    fr2 = mat2Data[i].split('/')
                    y = int(fr2[0]) / int(fr2[1])
                else:  #정수
                    y = int(mat2Data[i])

                result = x + y
                AddResult.append(round(result,3))
            except Exception:  #예외(사용자가 문자를 입력 등)가 발생하면 Error창을 띄움
                self.ErrorWindowShow()
                err = 1  #에러가 발생하지 않으면 err의 값은 0이고 에러가 발생하면 err의 값은 1이 됨


        if err != 1:  #에러가 발생하면 값을 계산한 결과를 보여주지 않고 애러창만 띄우기 위함. 에러가 발생하지 않으면 정상 작동됨
            for i in self.resultMat:
                for j in i:
                    j.setText(str(AddResult[count]))
                    count += 1

    def MatrixMultiply(self):
        mat1ColCount = len(self.mat1[0])
        mat1RowCount = len(self.mat1)
        mat2ColCount = len(self.mat2[0])
        mat2RowCount = len(self.mat2)
        # print(mat1RowCount)
        # print(mat1ColCount)
        # print(mat2RowCount)
        # print(mat2ColCount)

        for rowTEdit in self.mat1:  # QTextEdit 배열임(2중)
            for columnTEdit in rowTEdit:
                tempText = columnTEdit.text()
                # print("1",tempText)
                if not(self.isNumber(tempText)):  # 숫자가 아닌경우 걸러냄
                    columnTEdit.setText("숫자를 입력하십시오")
                    self.ErrorWindowShow()
                    return
        for rowTEdit in self.mat2:  # QTextEdit 배열임(2중)
            for columnTEdit in rowTEdit:
                tempText = columnTEdit.text()
                # print("2",tempText)
                if not (self.isNumber(tempText)):
                    columnTEdit.setText("숫자를 입력하십시오")
                    self.ErrorWindowShow()
                    return

        for row in range(mat1RowCount):
            for col in range(mat2ColCount):  # 행렬을 계산해야하는 총 횟수
                sum2 = 0
                for calCount in range(mat1ColCount):
                    a = float(self.mat1Grid.itemAtPosition(row, calCount).widget().text())
                    b = float(self.mat2Grid.itemAtPosition(calCount, col).widget().text())
                    sum2 += a*b
                print("sum", sum2)
                if int(sum2) == float(sum2):
                    strSum = str(int(sum2))            # 결과가 정수일 때는 소수점 아래를 생략
                else:
                    strSum = "{:.2f}".format(sum2)      # 결과가 실수일 때는 소수점 둘째짜리까지만
                self.resultGrid.itemAtPosition(row, col).widget().setText(strSum)

    def isNumber(self, n):  # 숫자 판별 메소드
        try:
            if math.isnan(float(n)):
                return False
        except ValueError:
            return False
        return True

    def MatrixBoolMultiply(self):
        isBinary = False # 0과 1로만 채워져 있는지 확인하기위한 변수
        matrix_1 = [] # 첫번째 행렬
        for i in self.mat1:
            malData = []
            for j in i:
                malData.append(j.text())
                if j.text() != '1' and j.text() != '0':
                    isBinary = True # 0과 1이 아닌수가 들어오면 True
            matrix_1.append(malData)
        matrix_2 = [] # 두번째 행렬
        for i in self.mat2:
            malData = []
            for j in i:
                malData.append(j.text())
                if j.text() != '1' and j.text() != '0':
                    isBinary = True # 0과 1이 아닌수가 들어오면 True
            matrix_2.append(malData)
        if isBinary:
            self.ErrorWindowShow() # 0과 1이 아닌수가 들어오면 오류창을 띄움
            return
        print("첫번째 행렬", matrix_1)
        print("두번째 행렬", matrix_2)
        for i in range(0, len(matrix_1)):
            for j in range(0, len(matrix_2[0])):
                a = '0' # 일단 '0'으로 초기화
                for m in range(0, len(matrix_1[0])):
                    if matrix_1[i][m] == '1' and matrix_2[m][j] == '1': # 둘다 1이면 '1'로 변경
                        a = '1'
                        break
                self.resultMat[i][j].setText(a) # resultMat의 text에 a값 넣기
        print("최종 행렬 ", self.resultMat)
        for i in range(len(matrix_1)):
            for j in range(len(matrix_2[0])):
                self.resultGrid.itemAtPosition(i, j).widget().setText(self.resultMat[i][j].text()) # resultGrid에 resultMat값 넣기

    def InverseMatrix(self):
        mat1ColCount = len(self.mat1[0])
        mat1RowCount = len(self.mat1)
        mat2ColCount = len(self.mat2[0])
        mat2RowCount = len(self.mat2)
        mat1List = list()
        mat2List = list()
        if not(mat1ColCount == mat1RowCount == mat2ColCount == mat2RowCount):
            self.ErrorWindowShow()
            return

        for rowTEdit in self.mat1:  # QTextEdit 배열임(2중)
            tempList = []
            for columnTEdit in rowTEdit:
                tempText = columnTEdit.text()
                # print("1",tempText)
                if not(self.isNumber(tempText)):  # 숫자가 아닌경우 걸러냄
                    columnTEdit.setText("숫자를 입력하십시오")
                    self.ErrorWindowShow()
                    return
                tempList.append(float(tempText))
            mat1List.append(tempList)
            # print("mat1List",mat1List)
        try:
            npMatrix = np.array(mat1List)
            inverseMatrix = np.linalg.inv(npMatrix)
            dotMatrix = np.dot(npMatrix, inverseMatrix)
            for rowIndex in range(mat1RowCount):
                for columnIndex in range(mat1ColCount):
                    mat2Str = "{:.2f}".format(inverseMatrix[rowIndex][columnIndex])
                    iMat2 = int(float(mat2Str))
                    if iMat2 == float(mat2Str):
                        mat2Str = str(iMat2)
                    self.mat2[rowIndex][columnIndex].setText(mat2Str)

                    resultStr = "{:.2f}".format(dotMatrix[rowIndex][columnIndex])
                    iResult = int(float(resultStr))
                    if iResult == float(resultStr):
                        resultStr = str(iResult)
                    self.resultMat[rowIndex][columnIndex].setText(resultStr)  # 이렇게 서식지정 안하면 부동소수로 굉장히 보기 힘들게 나옴
        except numpy.linalg.LinAlgError:
            self.mat2[0][0].setText("선형 대수 관련 조건에 오류가 있습니다. 혹은 역행렬이 없습니다.")



# 각 행렬의 값을 추출하는 방법, 좌측 행렬을 mat1, 우측 행렬을 mat2로 함.
# 그대로 복사해서 사용하지 않고 상황에 알맞게 사용할 것.
# mat1Data = []
# for i in self.mat1:
#     mat1RowData = []
#     for j in i:
#         mat1RowData.append(j.toPlainText())
#         mat1Data.append(mat1RowData)

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