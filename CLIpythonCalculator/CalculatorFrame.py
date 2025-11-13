import CalculatorFunction

class CalculatorMain :
   
    def __init__(self):
        self.result = 0
        self.cList = list()
        self.name = ""
        print("계산기 생성 완료!")
        
        
    def showUI(self):
        choice = input(""" ------- 메뉴 -------- :
1) 부피계산

2) 넓이 계산

3) 겉넓이 계산

4) 종료

계산기 Version alpha

항목을(를) 선택하세요 >>> """)
        return choice

    
    def cListManager(self,calName):
        if(self.result == "기록확인"):
            calName = "겉넓이 :기록"
        if (len(self.cList) >= 10 ):
            self.cList.remove(self.cList[0])
            self.cList.append((calName,self.result))
        else :
            self.cList += [(calName,self.result)]
            
    def showList(self):
        
        print("최근 10개의 기록")
        for i in self.cList:
            print(i)
        return "기록확인"

    # def changeMod(self) :
        
    
    def main(self):
        while(True):
            
            print("-"*30)
            choice = self.showUI()
            print("-"*30)

            
            if choice == "부피" or choice == "1" :
                cv = CalculatorFunction.CalVolume()
                while(True) :
                    choice = cv.UI()
                    
                    print("\n ****현재 현택한 메뉴" +choice+ "********* \n")
                    
                    if(choice == "1" or choice == "구"):
                        radius = float(input("반지름을 입력하세요>>>"))
                        self.result = CalculatorFunction.CalVolume.sphere(cv,radius)
                        print("결과값",self.result)
                        
                    elif (choice =="2" or choice == "원기둥"):
                        radius,height = (input("반지름과 높이를 입력하세요>>>").split())
                        radius = float(radius)
                        height = float(height)
                        self.result = cv.cylinder(radius,height)
                        print("결과값",self.result)
                        
                    elif (choice =="3" or choice == "원뿔"):
                        radius,height = (input("반지름과 높이를 입력하세요>>>").split())
                        radius = float(radius)
                        height = float(height)
                        self.result = cv.cone(radius,height)
                        print("결과값",self.result)
                        
                    elif (choice =="4" or choice == "육면체"):
                        a,b,c = (input("세 변의 길이를 입력하세요 >>>").split())
                        a = float(a)
                        b = float(b)
                        c=float(c)
                        self.result = cv.cuboid(a,b,c)
                        print("결과값",self.result)

                    elif (choice =="5" or choice == "정사면체"):
                        chek = input("높이를 알고 있나요? y or n >>>")
                        self.result = cv.regPolyhedron4(chek)
                        print("결과값",self.result)

                    elif (choice =="6" or choice == "삼각뿔"):
                        self.result = cv.triPyramid()
                        print("결과",self.result)
                        
                    elif (choice =="7" or choice == "정사각뿔"):
                        a,h = (input("밑면의 모서리 길이와 높이를 입력하세요").split())
                        a = float(a)
                        h = float(h)
                        self.result = cv.regFourPyramid(a,h)
                        print("결과",self.result)

                    elif (choice =="8" or choice == "종료"):
                        print("넓이 계산을 종료합니다.")
                        break
                del cv

            elif choice == "넓이" or choice == "2" :
                ca = CalculatorFunction.CalArea()
                menuChoice = {
                    "1" : ca.tri,
                    "2" : ca.cir,
                    "3" : ca.rect,
                    "4" : ca.dia,
                    "5" : ca.trapezoid,
                    "6" : ca.regPentagon,
                    "7" : ca.regHexagon,
                    }
                    
                while(True) :
                    choice = ca.UI()
                    print("\n ****현재 현택한 메뉴" +choice+ "********* \n")
                    if(choice == "8") :
                        print("넓이 계산을 종료합니다.\n\n\n\n")
                        break
                    self.result = menuChoice[choice]()
                    print("결과값",self.result)
                del ca


            elif choice == "겉넓이" or choice == "3" :
                csa = CalculatorFunction.CalSurfaceArea()
                csaMenu = {
                    "구" : csa.sphere,
                    "원기둥" : csa.cylinder,
                    "원뿔" : csa.cone,
                    "육면체" : csa.cuboid,
                    "정사면체" : csa.regPolyhedron4,
                    "정사각뿔" : csa.regQuadPyramid,
                    "모드선택" : csa.changeMod,
                    "기록" : self.showList
                    }
                while(True) :
                    
                    choice = csa.UI()
                    print("\n *****현재 현택한 메뉴 : " +choice+ "********* \n")
                    if( choice == "종료" ):
                        break
                    if(choice in csaMenu.keys()) : #이 문장을 통해서 혹시라도 잘못 입력된다면 꺼지지 않고 이행가능
                        self.result = csaMenu[choice]()
                        print("\n결과값",self.result,"\n\n")
                        self.cListManager(csa.sName)
                    else :
                        print("잘못된 입력입니다.") 
                del csa
                
            elif choice == "4" or choice == "종료":
                print("종료합니다.\n'del 클래스객체명' 을 통해서 객체를 제거할 수 있습니다. \n기본 객체명 : cm")
                break

            else :
                print("메뉴에 없는 입력입니다. 다시 입력하세요")


cm = CalculatorMain()
cm.main()      
                
            
                    
                    
                    

            
                
