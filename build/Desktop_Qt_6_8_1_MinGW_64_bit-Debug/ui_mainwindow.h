/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QTableWidget *tableWidget;
    QPushButton *addInitialButton;
    QWidget *taskListArea;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QPushButton *deleteTaskButton;
    QPushButton *editTaskButton;
    QWidget *taskInputArea_2;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *taskInput;
    QPushButton *addTaskButton;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        tableWidget = new QTableWidget(centralwidget);
        tableWidget->setObjectName("tableWidget");
        tableWidget->setGeometry(QRect(50, 40, 331, 251));
        addInitialButton = new QPushButton(centralwidget);
        addInitialButton->setObjectName("addInitialButton");
        addInitialButton->setGeometry(QRect(70, 50, 21, 24));
        taskListArea = new QWidget(centralwidget);
        taskListArea->setObjectName("taskListArea");
        taskListArea->setGeometry(QRect(60, 180, 311, 80));
        verticalLayout = new QVBoxLayout(taskListArea);
        verticalLayout->setObjectName("verticalLayout");
        label = new QLabel(taskListArea);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        deleteTaskButton = new QPushButton(taskListArea);
        deleteTaskButton->setObjectName("deleteTaskButton");
        deleteTaskButton->setMinimumSize(QSize(239, 24));

        verticalLayout->addWidget(deleteTaskButton);

        editTaskButton = new QPushButton(taskListArea);
        editTaskButton->setObjectName("editTaskButton");

        verticalLayout->addWidget(editTaskButton);

        taskInputArea_2 = new QWidget(centralwidget);
        taskInputArea_2->setObjectName("taskInputArea_2");
        taskInputArea_2->setGeometry(QRect(60, 80, 160, 80));
        verticalLayout_2 = new QVBoxLayout(taskInputArea_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        taskInput = new QLineEdit(taskInputArea_2);
        taskInput->setObjectName("taskInput");

        verticalLayout_2->addWidget(taskInput);

        addTaskButton = new QPushButton(taskInputArea_2);
        addTaskButton->setObjectName("addTaskButton");

        verticalLayout_2->addWidget(addTaskButton);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        addInitialButton->setText(QCoreApplication::translate("MainWindow", "\357\274\213", nullptr));
        label->setText(QString());
        deleteTaskButton->setText(QCoreApplication::translate("MainWindow", "\345\211\212\351\231\244", nullptr));
        editTaskButton->setText(QCoreApplication::translate("MainWindow", "\345\244\211\346\233\264", nullptr));
        addTaskButton->setText(QCoreApplication::translate("MainWindow", "\350\277\275\345\212\240", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
