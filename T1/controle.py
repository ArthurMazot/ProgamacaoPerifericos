from PyQt5.QtCore import *
from PyQt5.QtGui import QPixmap
from PyQt5 import QtWidgets, QtCore
from PyQt5 import uic

import serial
import time

global ser  #Serial
global conn #Flag para conecção serial

#Sophia -> Corrigido comunicação, e estágio inicial

def getData():
    global ser
    while True:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8').strip()
            print(f"Recebido: {data}")
            break
    return data

def getTemp():
    global ser
    ser.write(b'0') #Escreve na serial
    time.sleep(2)
    data = getData()
    data = int(data) / 10
    return data

def getUmid():
    global ser
    ser.write(b'1') #Escreve na serial
    time.sleep(2)
    data = getData()
    data = int(data) / 10
    return data

def getTempUmid():
    global ser
    ser.write(b'2') #Escreve na serial
    time.sleep(2)
    data = getData()

    temp, umid = data.split(':')

    temp = int(temp) / 10
    umid = int(umid) / 10

    return temp, umid


#Funções dos botões
def fun_connButton():
    global ser
    global conn


# Atenção !!! Porta Serial depende do PC, verificar USB
# Linux -> lsusb
    if(conn == False):
        ser = serial.Serial('COM9', 9600)
        time.sleep(2)
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

    else:
        print('Sem Conecção')

def fun_umidButton():
    global conn

    if(conn == True):
        print("Umidade")
        umid = getUmid()
        janela.umidDisp.setValue(int(umid))

    else:
        print('Sem Conecção')

def fun_tempUmidButton():
    global conn

    if(conn == True):
        print("Temperatura / Umidade")
        temp, umid = getTempUmid()

        janela.tempDisp.display(temp)
        janela.umidDisp.setValue(int(umid))

    else:
        print('Sem Conecção')

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

if conn == True:
    ser.close()