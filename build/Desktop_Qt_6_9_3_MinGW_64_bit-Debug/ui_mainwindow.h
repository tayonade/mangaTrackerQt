/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QLineEdit *lineEditSearch;
    QPushButton *pushButtonSearch;
    QListWidget *listWidgetManga;
    QListWidget *listWidgetChapter;
    QListWidget *listWidgetBookmarks;
    QLabel *labelCover;
    QLabel *label;
    QLabel *labelStatus;
    QPushButton *pushButtonLastRead;
    QPushButton *pushButtonDelete;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1082, 363);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName("gridLayout");
        lineEditSearch = new QLineEdit(centralwidget);
        lineEditSearch->setObjectName("lineEditSearch");

        gridLayout->addWidget(lineEditSearch, 0, 0, 1, 2);

        pushButtonSearch = new QPushButton(centralwidget);
        pushButtonSearch->setObjectName("pushButtonSearch");

        gridLayout->addWidget(pushButtonSearch, 0, 2, 1, 1);

        listWidgetManga = new QListWidget(centralwidget);
        listWidgetManga->setObjectName("listWidgetManga");

        gridLayout->addWidget(listWidgetManga, 1, 0, 1, 3);

        listWidgetChapter = new QListWidget(centralwidget);
        listWidgetChapter->setObjectName("listWidgetChapter");

        gridLayout->addWidget(listWidgetChapter, 1, 3, 1, 1);

        listWidgetBookmarks = new QListWidget(centralwidget);
        listWidgetBookmarks->setObjectName("listWidgetBookmarks");

        gridLayout->addWidget(listWidgetBookmarks, 1, 4, 1, 1);

        labelCover = new QLabel(centralwidget);
        labelCover->setObjectName("labelCover");
        labelCover->setMinimumSize(QSize(271, 181));

        gridLayout->addWidget(labelCover, 1, 5, 1, 1);

        label = new QLabel(centralwidget);
        label->setObjectName("label");

        gridLayout->addWidget(label, 2, 0, 1, 1);

        labelStatus = new QLabel(centralwidget);
        labelStatus->setObjectName("labelStatus");
        labelStatus->setMinimumSize(QSize(681, 0));

        gridLayout->addWidget(labelStatus, 2, 1, 1, 5);

        pushButtonLastRead = new QPushButton(centralwidget);
        pushButtonLastRead->setObjectName("pushButtonLastRead");

        gridLayout->addWidget(pushButtonLastRead, 3, 3, 1, 1);

        pushButtonDelete = new QPushButton(centralwidget);
        pushButtonDelete->setObjectName("pushButtonDelete");

        gridLayout->addWidget(pushButtonDelete, 3, 4, 1, 2);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1082, 25));
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
        pushButtonSearch->setText(QCoreApplication::translate("MainWindow", "go", nullptr));
#if QT_CONFIG(shortcut)
        pushButtonSearch->setShortcut(QCoreApplication::translate("MainWindow", "Return", nullptr));
#endif // QT_CONFIG(shortcut)
        labelCover->setText(QCoreApplication::translate("MainWindow", "Cover Image", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Status", nullptr));
        labelStatus->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        pushButtonLastRead->setText(QCoreApplication::translate("MainWindow", "Last read", nullptr));
        pushButtonDelete->setText(QCoreApplication::translate("MainWindow", "delete bookmark", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
