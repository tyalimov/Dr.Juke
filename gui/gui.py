import re
import sys
import json
import os
from PyQt5.QtWidgets import QMainWindow, QApplication, QPushButton, QWidget, QAction, QTabWidget,QVBoxLayout, QCheckBox, QComboBox, QGridLayout, QGroupBox, QHBoxLayout, QLabel, QLineEdit, QTreeView, QMessageBox, QHeaderView
from PyQt5.QtGui import QIcon, QPalette, QColor, QStandardItemModel
from PyQt5.QtCore import pyqtSlot, QDate, QDateTime, QRegExp, QSortFilterProxyModel, Qt, QTime
from PyQt5 import QtCore

import avservice.ipclib_pyport

def ProcessRequest(message):
    # message = json
    avservice.ipclib_pyport.PushMessage(message)
    # responce = json
    responce = avservice.ipclib_pyport.PopMessage()
    # result = dict
    print("Got responce: " + responce)

    return json.loads(responce)

class App(QMainWindow):

    def __init__(self):
        super().__init__()
        self.title = 'Dr.Juke'
        self.left = 0
        self.top = 0
        self.width = 600
        self.height = 200
        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)
        
        self.table_widget = MyTableWidget(self)
        self.setCentralWidget(self.table_widget)
        
        self.show()
    
class MyTableWidget(QWidget):
    def __init__(self, parent):
        super(QWidget, self).__init__(parent)
        self.layout = QVBoxLayout(self)
        
        # Initialize tab screen
        self.tabs = QTabWidget()
        self.tab0 = QWidget()
        self.tab1 = QWidget()
        self.tab2 = QWidget()
        self.tab3 = QWidget()
        self.tab4 = QWidget()
        self.tab5 = QWidget()
        self.tabs.resize(300,200)
        
        # Add tabs
        self.tabs.addTab(self.tab0,"Сканирование")
        self.tabs.addTab(self.tab5,"Облачное сканирование")
        self.tabs.addTab(self.tab1,"Защищаемые файлы")
        self.tabs.addTab(self.tab2,"Защищаемые процессы")
        self.tabs.addTab(self.tab3,"Защищаемые ключи")
        self.tabs.addTab(self.tab4,"Правила МЭ")
        
        self.create_tab0()
        self.create_tab5()
        self.create_tab1()
        self.create_tab2()
        self.create_tab3()
        self.create_tab4()

        self.layout.addWidget(self.tabs)
        self.setLayout(self.layout)

    @pyqtSlot()
    def on_click(self):
        #print("\n")
        for currentQTableWidgetItem in self.tableWidget.selectedItems():
            print(currentQTableWidgetItem.row(), currentQTableWidgetItem.column(), currentQTableWidgetItem.text())

    PATH, MASK = 0, 1

    def clear_data(self, model):
        model.removeRows(0, model.rowCount())

    ################################### 1 tab

    def create_tab1(self):
        self.tab1.layout = QVBoxLayout(self)
        
        self.createBoxLayout1()
        self.createGridLayout1()

        self.tab1.layout.addWidget(self.dataGroupBox1)
        self.tab1.layout.addWidget(self.horizontalGroupBox1)

        self.tab1.setLayout(self.tab1.layout)

    def createBoxLayout1(self):
        self.dataGroupBox1 = QGroupBox("Файлы")
        
        self.dataView1 = QTreeView()
        self.dataView1.setRootIsDecorated(False)
        self.dataView1.setAlternatingRowColors(True)
        
        dataLayout1 = QHBoxLayout()
        dataLayout1.addWidget(self.dataView1)
        self.dataGroupBox1.setLayout(dataLayout1)

        model1 = self.createTableModel(self)
        self.dataView1.setModel(model1)
        self.add_file_data(model1)
        self.dataView1.resizeColumnToContents(0)
        self.dataView1.resizeColumnToContents(1)

    def createGridLayout1(self):
        self.horizontalGroupBox1 = QGroupBox()
        layout = QGridLayout()
        
        self.textbox11 = QLineEdit(self)
        self.textbox11.setPlaceholderText("Имя файла")
        self.textbox12 = QLineEdit(self)
        self.textbox12.setPlaceholderText("Маска доступа")
        
        buttonAdd = QPushButton('Добавить', self)
        buttonAdd.clicked.connect(self.on_click_add_file)
        buttonDel = QPushButton('Удалить', self)
        buttonDel.clicked.connect(self.on_click_del_file)

        layout.addWidget(self.textbox11, 0, 0)
        layout.addWidget(self.textbox12, 0, 1)
        layout.addWidget(buttonAdd, 1, 0)
        layout.addWidget(buttonDel, 1, 1)
        self.horizontalGroupBox1.setLayout(layout)

    def add_file_data(self, model):
        
        request = """
        {
            "task" : "get_filesystem_rules",
            "parameters"  : 
            {
            }
        }
        """
        proc = ProcessRequest(request)

        #proc = {'file1': '0101', 'file2' : '1111', 'file3':'0000', 'file4': '1000'}
        for name, data in proc:
            model.insertRow(0)
            model.setData(model.index(0, self.PATH), name)
            model.setData(model.index(0, self.MASK), data)

    @pyqtSlot()
    def on_click_add_file(self):
        path_to_file = self.textbox11.text()
        proc_mask = self.textbox12.text()
        if not path_to_file or not proc_mask:
            QMessageBox.critical(self, 'Отсутствуют значения', "Необходимо ввести пусть к файлу и маску для защиты", QMessageBox.Ok, QMessageBox.Ok)
        else:
            request = """
            {{
                "task" : "add_protected_object",
                "parameters"  : 
                {{
                    "path"    : "{0}",
                    "type"    :  0,
                    "access_mask" :  {1}
                }}
            }}""".format(path_to_file, proc_mask)
            resp = ProcessRequest(request)
            model = self.dataView1.model()
            self.clear_data(model)
            self.add_file_data(model)
            
        self.textbox11.setText("")
        self.textbox12.setText("")

    @pyqtSlot()
    def on_click_del_file(self):
        path_to_file = self.textbox11.text()
        if not path_to_file:
            QMessageBox.critical(self, 'Отсутствуют значения', "Необходимо ввести пусть к файлу", QMessageBox.Ok, QMessageBox.Ok)
        else:
            request = """
            {{
                "task" : "del_protected_object",
                "parameters"  : 
                {{
                    "path"    : "{0}",
                    "type"    :  0
                }}
            }}""".format(path_to_file)
            resp = ProcessRequest(request)
            model = self.dataView1.model()
            self.clear_data(model)
            self.add_file_data(model)
            
        self.textbox11.setText("")
        self.textbox12.setText("")

    ################################### 2 tab

    def createTableModel(self,parent):
        model = QStandardItemModel(0, 2, parent)
        model.setHeaderData(self.PATH, Qt.Horizontal, "Путь")
        model.setHeaderData(self.MASK, Qt.Horizontal, "Маска")
        return model

    def create_tab2(self):
        self.tab2.layout = QVBoxLayout(self)
        
        self.createBoxLayout2()
        self.createGridLayout2()

        self.tab2.layout.addWidget(self.dataGroupBox2)
        self.tab2.layout.addWidget(self.horizontalGroupBox2)

        self.tab2.setLayout(self.tab2.layout)

    def createBoxLayout2(self):
        self.dataGroupBox2 = QGroupBox("Процессы")
        
        self.dataView2 = QTreeView()
        self.dataView2.setRootIsDecorated(False)
        self.dataView2.setAlternatingRowColors(True)
        
        dataLayout2 = QHBoxLayout()
        dataLayout2.addWidget(self.dataView2)
        self.dataGroupBox2.setLayout(dataLayout2)

        model2 = self.createTableModel(self)
        self.dataView2.setModel(model2)
        self.add_data(model2)
        self.dataView2.resizeColumnToContents(0)
        self.dataView2.resizeColumnToContents(1)

    def createGridLayout2(self):
        self.horizontalGroupBox2 = QGroupBox()
        layout = QGridLayout()
        
        self.textbox1 = QLineEdit(self)
        self.textbox1.setPlaceholderText("Имя процесса")
        self.textbox2 = QLineEdit(self)
        self.textbox2.setPlaceholderText("Маска доступа")
        
        buttonAdd = QPushButton('Добавить', self)
        buttonAdd.clicked.connect(self.on_click_add_proc)
        buttonDel = QPushButton('Удалить', self)
        buttonDel.clicked.connect(self.on_click_del_proc)

        layout.addWidget(self.textbox1, 0, 0)
        layout.addWidget(self.textbox2, 0, 1)
        layout.addWidget(buttonAdd, 1, 0)
        layout.addWidget(buttonDel, 1, 1)
        self.horizontalGroupBox2.setLayout(layout)

    def add_data(self, model):

        request = """
        {
            "task" : "get_process_rules",
            "parameters"  : 
            {
            }
        }
        """
        proc = ProcessRequest(request)

        # TODO получить json с процессами и масками
        #proc = {'proc1111111111111111111111111111111111111111111111111111111111111111111111111': '0101', 'proc2' : '1111', 'proc3':'0000', 'proc4': '1000'}
        for name, data in proc:
            model.insertRow(0)
            model.setData(model.index(0, self.PATH), name)
            model.setData(model.index(0, self.MASK), data)

    @pyqtSlot()
    def on_click_add_proc(self):
        path_to_proc = self.textbox1.text()
        proc_mask = self.textbox2.text()
        if not path_to_proc or not proc_mask:
            QMessageBox.critical(self, 'Отсутствуют значения', "Необходимо ввести пусть к процессу и маску для защиты", QMessageBox.Ok, QMessageBox.Ok)
        else:
            
            request = """
            {{
                "task" : "add_protected_object",
                "parameters"  : 
                {{
                    "path"    : "{0}",
                    "type"    :  2,
                    "access_mask" :  {1}
                }}
            }}
            """.format(path_to_proc, proc_mask)
            resp = ProcessRequest(request)

            model = self.dataView2.model()
            self.clear_data(model)
            self.add_data(model)
            
        self.textbox1.setText("")
        self.textbox2.setText("")

    @pyqtSlot()
    def on_click_del_proc(self):
        path_to_proc = self.textbox1.text()
        if not path_to_proc:
            QMessageBox.critical(self, 'Отсутствуют значения', "Необходимо ввести пусть к процессу", QMessageBox.Ok, QMessageBox.Ok)
        else:
            request = """
            {{
                "task" : "del_protected_object",
                "parameters"  : 
                {{
                    "path"    : "{0}",
                    "type"    :  2
                }}
            }}""".format(path_to_proc)
            resp = ProcessRequest(request)
            model = self.dataView2.model()
            self.clear_data(model)
            self.add_data(model)
            
        self.textbox1.setText("")
        self.textbox2.setText("")

    ################################### 3 tab

    def create_tab3(self):
        self.tab3.layout = QVBoxLayout(self)
        
        self.createBoxLayout3()
        self.createGridLayout3()

        self.tab3.layout.addWidget(self.dataGroupBox3)
        self.tab3.layout.addWidget(self.horizontalGroupBox3)

        self.tab3.setLayout(self.tab3.layout)

    def createBoxLayout3(self):
        self.dataGroupBox3 = QGroupBox("Ключи реестра")
        
        self.dataView3 = QTreeView()
        self.dataView3.setRootIsDecorated(False)
        self.dataView3.setAlternatingRowColors(True)
        
        dataLayout3 = QHBoxLayout()
        dataLayout3.addWidget(self.dataView3)
        self.dataGroupBox3.setLayout(dataLayout3)

        model3 = self.createTableModel(self)
        self.dataView3.setModel(model3)
        self.add_key_data(model3)
        self.dataView3.resizeColumnToContents(0)
        self.dataView3.resizeColumnToContents(1)

    def createGridLayout3(self):
        self.horizontalGroupBox3 = QGroupBox()
        layout = QGridLayout()
        
        self.textbox31 = QLineEdit(self)
        self.textbox31.setPlaceholderText("Имя ключа")
        self.textbox32 = QLineEdit(self)
        self.textbox32.setPlaceholderText("Маска доступа")
        
        buttonAdd = QPushButton('Добавить', self)
        buttonAdd.clicked.connect(self.on_click_add_key)
        buttonDel = QPushButton('Удалить', self)
        buttonDel.clicked.connect(self.on_click_del_key)

        layout.addWidget(self.textbox31, 0, 0)
        layout.addWidget(self.textbox32, 0, 1)
        layout.addWidget(buttonAdd, 1, 0)
        layout.addWidget(buttonDel, 1, 1)
        self.horizontalGroupBox3.setLayout(layout)

    def add_key_data(self, model):
        # TODO получить json с ключами и масками
        request = """
        {
            "task" : "get_registry_rules",
            "parameters"  : 
            {
            }
        }
        """
        proc = ProcessRequest(request)

        #proc = {'key1': '0101', 'key2' : '1111', 'key3':'0000', 'key4': '1000'}
        for name, data in proc:
            model.insertRow(0)
            model.setData(model.index(0, self.PATH), name)
            model.setData(model.index(0, self.MASK), data)

    @pyqtSlot()
    def on_click_add_key(self):
        path_to_key = self.textbox31.text()
        key_mask = self.textbox32.text()
        if not path_to_key or not key_mask:
            QMessageBox.critical(self, 'Отсутствуют значения', "Необходимо ввести пусть к ключу и маску для защиты", QMessageBox.Ok, QMessageBox.Ok)
        else:
            request = """
            {{
                "task" : "add_protected_object",
                "parameters"  : 
                {{
                    "path"    : "{0}",
                    "type"    :  1,
                    "access_mask" :  {1}
                }}
            }}
            """.format(path_to_key, key_mask)
            resp = ProcessRequest(request)
            model = self.dataView3.model()
            self.clear_data(model)
            self.add_key_data(model)
            
        self.textbox31.setText("")
        self.textbox32.setText("")

    @pyqtSlot()
    def on_click_del_key(self):
        path_to_key = self.textbox31.text()
        if not path_to_key:
            QMessageBox.critical(self, 'Отсутствуют значения', "Необходимо ввести пусть к ключу", QMessageBox.Ok, QMessageBox.Ok)
        else:
            request = """
            {{
                "task" : "del_protected_object",
                "parameters"  : 
                {{
                    "path"    : "{0}",
                    "type"    :  1
                }}
            }}""".format(path_to_key)
            resp = ProcessRequest(request)
            model = self.dataView3.model()
            self.clear_data(model)
            self.add_key_data(model)
            
        self.textbox31.setText("")
        self.textbox32.setText("")


    ################################### 4 tab

    def create_tab4(self):
        self.tab4.layout = QVBoxLayout(self)
        
        self.createBoxLayout4()
        self.createGridLayout4()

        self.tab4.layout.addWidget(self.dataGroupBox4)
        self.tab4.layout.addWidget(self.horizontalGroupBox4)

        self.tab4.setLayout(self.tab4.layout)

    def createBoxLayout4(self):
        self.dataGroupBox4 = QGroupBox("Правила межсетевого экрана")
        
        self.dataView4 = QTreeView()
        self.dataView4.setRootIsDecorated(False)
        self.dataView4.setAlternatingRowColors(True)
        
        dataLayout4 = QHBoxLayout()
        dataLayout4.addWidget(self.dataView4)
        self.dataGroupBox4.setLayout(dataLayout4)

        model4 = self.createTableModel(self)
        self.dataView4.setModel(model4)
        self.add_rule_data(model4)
        self.dataView4.resizeColumnToContents(0)
        self.dataView4.resizeColumnToContents(1)

    def createGridLayout4(self):
        self.horizontalGroupBox4 = QGroupBox()
        layout = QGridLayout()
        
        self.textbox41 = QLineEdit(self)
        self.textbox41.setPlaceholderText("Правило")
        self.textbox42 = QLineEdit(self)
        self.textbox42.setPlaceholderText("Содержимое")
        
        buttonAdd = QPushButton('Добавить', self)
        buttonAdd.clicked.connect(self.on_click_add_rule)
        buttonDel = QPushButton('Удалить', self)
        buttonDel.clicked.connect(self.on_click_del_rule)

        layout.addWidget(self.textbox41, 0, 0)
        layout.addWidget(self.textbox42, 0, 1)
        layout.addWidget(buttonAdd, 1, 0)
        layout.addWidget(buttonDel, 1, 1)
        self.horizontalGroupBox4.setLayout(layout)

    def add_rule_data(self, model):

        request = """
        {
            "task" : "get_firewall_rules",
            "parameters"  : 
            {
            }
        }
        """
        proc = ProcessRequest(request)

        # TODO получить json с правилами и масками
        #proc = {'rule1': '0101', 'rule2' : '1111', 'rule3':'0000', 'rule4': '1000'}
        for name, data in proc:
            model.insertRow(0)
            model.setData(model.index(0, self.PATH), name)
            model.setData(model.index(0, self.MASK), data)

    @pyqtSlot()
    def on_click_add_rule(self):
        rule = self.textbox41.text()
        content = self.textbox42.text()
        if not rule or not content:
            QMessageBox.critical(self, 'Отсутствуют значения', "Необходимо ввести имя правила и его содержимое", QMessageBox.Ok, QMessageBox.Ok)
        else:

            request = """
            {{
                "task"        : "add_firewall_rule",
                "parameters"  : 
                {{
                    "name"    : "{0}",
                    "content" : "{1}"
                }}
            }}
            """.format(rule, content)
            resp = ProcessRequest(request)
            # TODO вызвать добавление, обработать ошибки
            model = self.dataView4.model()
            self.clear_data(model)
            self.add_rule_data(model)
            
        self.textbox41.setText("")
        self.textbox42.setText("")

    @pyqtSlot()
    def on_click_del_rule(self):
        rule = self.textbox41.text()
        if not rule:
            QMessageBox.critical(self, 'Отсутствуют значения', "Необходимо ввести имя правила", QMessageBox.Ok, QMessageBox.Ok)
        else:
            request = """
            {{
                "task"        : "remove_firewall_rule",
                "parameters"  : 
                {{
                    "name"    : "{0}"
                }}
            }}
            """.format(rule)
            resp = ProcessRequest(request)
            model = self.dataView4.model()
            self.clear_data(model)
            self.add_rule_data(model)
            
        self.textbox41.setText("")
        self.textbox42.setText("")

    ######################################## tab 0

    def create_tab0(self):
        
        self.tab0.layout = QVBoxLayout(self)

        self.textbox00 = QLineEdit(self)
        self.textbox00.setPlaceholderText("Введите путь к файлу")
        
        button01 = QPushButton('Сканировать', self)
        button01.clicked.connect(self.on_click_scan)

        self.horizontalGroupBoxYARA = QGroupBox()
        layout = QHBoxLayout()
        self.textboxYARA0 = QLabel(self)
        self.textboxYARA0.setText("YARA\t\t")
        self.textboxYARA1 = QLineEdit(self)
        layout.addWidget(self.textboxYARA0)
        layout.addWidget(self.textboxYARA1)
        self.horizontalGroupBoxYARA.setLayout(layout)

        self.horizontalGroupBoxClam = QGroupBox()
        layout = QHBoxLayout()
        self.textboxClam0 = QLabel(self)
        self.textboxClam0.setText("ClamAV\t\t")
        self.textboxClam1 = QLineEdit(self)
        layout.addWidget(self.textboxClam0)
        layout.addWidget(self.textboxClam1)
        self.horizontalGroupBoxClam.setLayout(layout)

        self.horizontalGroupBoxPackers = QGroupBox()
        layout = QHBoxLayout()
        self.textboxPackers0 = QLabel(self)
        self.textboxPackers0.setText("Упаковщики\t")
        self.textboxPackers1 = QLineEdit(self)
        layout.addWidget(self.textboxPackers0)
        layout.addWidget(self.textboxPackers1)
        self.horizontalGroupBoxPackers.setLayout(layout)

        self.horizontalGroupBoxCert = QGroupBox()
        layout = QHBoxLayout()
        self.textboxCert0 = QLabel(self)
        self.textboxCert0.setText("Сертификаты\t")
        self.textboxCert1 = QLineEdit(self)
        layout.addWidget(self.textboxCert0)
        layout.addWidget(self.textboxCert1)
        self.horizontalGroupBoxCert.setLayout(layout)


        self.tab0.layout.addWidget(self.textbox00)
        self.tab0.layout.addWidget(button01)
        self.tab0.layout.addWidget(self.horizontalGroupBoxYARA)
        self.tab0.layout.addWidget(self.horizontalGroupBoxClam)
        self.tab0.layout.addWidget(self.horizontalGroupBoxPackers)
        self.tab0.layout.addWidget(self.horizontalGroupBoxCert)

        self.tab0.setLayout(self.tab0.layout)

    @pyqtSlot()
    def on_click_scan(self):
        path = self.textbox00.text()
        if not os.path.exists(path):
            QMessageBox.critical(self, 'Ошибка файла', "Такого файла не существует!", QMessageBox.Ok, QMessageBox.Ok)
        else:
            request = """
            {{
                "task"        : "scan",
                "parameters"  : 
                {{
                    "path"    : "{0}"
                }}
            }}
            """.format(path)
            request = request.replace('\\', '\\\\')
            resp = ProcessRequest(request)
            for analyzer in resp:
                if analyzer["name"] == "Yara":
                    self.textboxYARA1.setText(str(analyzer["infected"]))
                if analyzer["name"] == "ClamAV":
                    self.textboxClam1.setText(str(analyzer["infected"]))
                if analyzer["name"] == "Signature":
                    self.textboxCert1.setText(str(analyzer["infected"]))
                if analyzer["name"] == "Packers":
                    self.textboxPackers1.setText(str(analyzer["infected"]))

        self.textbox00.setText('')
        
 ######################################## tab 5

    def create_tab5(self):
        
        self.tab5.layout = QVBoxLayout(self)

        self.textbox50 = QLineEdit(self)
        self.textbox50.setPlaceholderText("Введите путь к файлу")
        
        button51 = QPushButton('Сканировать', self)
        button51.clicked.connect(self.on_click_cloud)

        self.horizontalGroupBoxResult = QGroupBox()
        layout = QHBoxLayout()
        self.textboxResult0 = QLabel(self)
        self.textboxResult0.setText("Результат\t\t")
        self.textboxResult1 = QLineEdit(self)
        layout.addWidget(self.textboxResult0)
        layout.addWidget(self.textboxResult1)
        self.horizontalGroupBoxResult.setLayout(layout)

        self.tab5.layout.addWidget(self.textbox50)
        self.tab5.layout.addWidget(button51)
        self.tab5.layout.addWidget(self.horizontalGroupBoxResult)


        self.tab5.setLayout(self.tab5.layout)

    @pyqtSlot()
    def on_click_cloud(self):
        path = self.textbox50.text()
        if not os.path.exists(path):
            QMessageBox.critical(self, 'Ошибка файла', "Такого файла не существует!", QMessageBox.Ok, QMessageBox.Ok)
        else:
            request = """
            {{
                "task"        : "cloud_scan",
                "parameters"  : 
                {{
                    "path"    : "{0}"
                }}
            }}
            """.format(path)
            request = request.replace('\\', '\\\\')
            resp = ProcessRequest(request)
            self.textboxResult1.setText(str(resp["infected"]))


if __name__ == '__main__':
    app = QApplication(sys.argv)
    # Force the style to be the same on all OSs:
    app.setStyle("Fusion")

    # Now use a palette to switch to dark colors:
    palette = QPalette()
    palette.setColor(QPalette.Window, QColor(53, 53, 53))
    palette.setColor(QPalette.WindowText, QtCore.Qt.white)
    palette.setColor(QPalette.Base, QColor(25, 25, 25))
    palette.setColor(QPalette.AlternateBase, QColor(53, 53, 53))
    palette.setColor(QPalette.ToolTipBase, QtCore.Qt.white)
    palette.setColor(QPalette.ToolTipText, QtCore.Qt.white)
    palette.setColor(QPalette.Text, QtCore.Qt.white)
    palette.setColor(QPalette.Button, QColor(53, 53, 53))
    palette.setColor(QPalette.ButtonText, QtCore.Qt.white)
    palette.setColor(QPalette.BrightText, QtCore.Qt.red)
    palette.setColor(QPalette.Link, QColor(42, 130, 218))
    palette.setColor(QPalette.Highlight, QColor(42, 130, 218))
    palette.setColor(QPalette.HighlightedText, QtCore.Qt.black)
    app.setPalette(palette)

    ex = App()
    sys.exit(app.exec_())