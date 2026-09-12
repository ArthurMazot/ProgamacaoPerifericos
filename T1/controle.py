from PyQt5.QtCore import *
from PyQt5.QtGui import QPixmap
from PyQt5 import QtWidgets, QtCore
from PyQt5 import uic

import serial
import time

global ser  #Serial
global conn #Flag para conecção serial

#Talvez tenha que mudar
def getData():
    global ser
    while True:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8').strip() #Pega uma string no serial
            break
    return data

def getTemp():
    global ser
    ser.write(b'0') #Escreve na serial
    time.sleep(2)
    data = getData()
    #Tratar 'data' (dividir por 10)
    return data

def getUmid():
    global ser
    ser.write(b'1') #Escreve na serial
    time.sleep(2)
    data = getData()
    #Tratar 'data' (dividir por 10)
    return data

def getTempUmid():
    global ser
    ser.write(b'2') #Escreve na serial
    time.sleep(2)
    data = getData()
    #Tratar 'data' (Separar temp de umid e dividir por 10)
    temp = 0 #mudar
    umid = 0 #mudar
    return temp, umid

##=============================================================##

#Funções dos botões
def fun_connButton():
    global ser
    global conn
    if(conn == False):
        ser = serial.Serial('COM3', 9600)
        print("Conectado")
        conn = True
    else:
        ser.close()
        print("Desconectado")
        conn = False

def fun_tempButton():
    global conn
    if(conn == True):
        print("Temperatura")
        temp = getTemp()
        janela.tempDisp.display(temp)
    else: print('Sem Conecção')
        
def fun_umidButton():
    global conn
    if(conn == True):
        print("Umidade")
        umid = getUmid()
        janela.umidDisp.setValue(umid)
    else: print('Sem Conecção')
        
def fun_tempUmidButton():
    global conn
    if(conn == True):
        print("Temperatura / Umidade")
        temp, umid = getTempUmid()
        temp = 0
        umid = 0
        janela.tempDisp.display(temp)
        janela.umidDisp.setValue(umid)
    else: print('Sem Conecção')

conn = False
app = QtWidgets.QApplication([])
janela = uic.loadUi("interface.ui")

#Botões
janela.connButton.clicked.connect(fun_connButton)
janela.tempButton.clicked.connect(fun_tempButton)
janela.umidButton.clicked.connect(fun_umidButton)
janela.tempUmidButton.clicked.connect(fun_tempUmidButton)

janela.show()
app.exec_()
