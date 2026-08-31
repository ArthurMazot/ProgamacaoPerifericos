from PyQt5.QtCore import *
from PyQt5.QtGui import QPixmap
from PyQt5 import QtWidgets, QtCore
from PyQt5 import uic

import serial
import time

def getData():
    while True:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8').strip() #Pega uma string no serial
            break
    return data

#Implementar
def getTemp():
    ser.write(b'0') #Escreve na serial
    time.sleep(2)
    data = getData()
    #Tratar 'data'
    return data

#Implementar
def getUmid():
    #ser.write(b'1') #Escreve na serial
    time.sleep(2)
    data = getData()
    #Tratar 'data'
    return data

def getTempUmid():
    ser.write(b'2') #Escreve na serial
    time.sleep(2)
    data = getData()
    #Tratar 'data'
    return data #Deixar assim?

##=============================================================##

#Funções dos botões
def fun_connButton():
    if(True): #Criar uma variavel para colocar aqui
        print("Conectado")
        #Fazer a conexão
    else:
        print("Desconectado")
        #Desfazer a conexão

def fun_tempButton():
    print("Temperatura")
    temp = getTemp()
    janela.tempDisp.display(temp)

def fun_umidButton():
    print("Umidade")
    umid = getUmid()
    janela.umidDisp.setValue(umid)
    
def fun_tempUmidButton():
    print("Temperatura / Umidade")
    data = getTempUmid()
    #data tem que ter temperatura e umidade
    temp = 0
    umid = 0
    janela.tempDisp.display(temp)
    janela.umidDisp.setValue(umid)

ser = serial.Serial('COM10', 9600) #Mudar COMX
time.sleep(2)
app = QtWidgets.QApplication([])
janela = uic.loadUi("interface.ui")

#Botões
janela.connButton.clicked.connect(fun_connButton)
janela.tempButton.clicked.connect(fun_tempButton)
janela.umidButton.clicked.connect(fun_umidButton)
janela.tempUmidButton.clicked.connect(fun_tempUmidButton)

janela.show()
app.exec_()