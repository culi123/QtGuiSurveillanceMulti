/********************************************************************************
** Form generated from reading UI file 'QtGuiSurveillanceMulti.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTGUISURVEILLANCEMULTI_H
#define UI_QTGUISURVEILLANCEMULTI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtGuiSurveillanceMultiClass
{
public:
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtGuiSurveillanceMultiClass)
    {
        if (QtGuiSurveillanceMultiClass->objectName().isEmpty())
            QtGuiSurveillanceMultiClass->setObjectName(QStringLiteral("QtGuiSurveillanceMultiClass"));
        QtGuiSurveillanceMultiClass->resize(1229, 880);
        centralWidget = new QWidget(QtGuiSurveillanceMultiClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        QtGuiSurveillanceMultiClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QtGuiSurveillanceMultiClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1229, 23));
        QtGuiSurveillanceMultiClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QtGuiSurveillanceMultiClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        QtGuiSurveillanceMultiClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(QtGuiSurveillanceMultiClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        QtGuiSurveillanceMultiClass->setStatusBar(statusBar);

        retranslateUi(QtGuiSurveillanceMultiClass);

        QMetaObject::connectSlotsByName(QtGuiSurveillanceMultiClass);
    } // setupUi

    void retranslateUi(QMainWindow *QtGuiSurveillanceMultiClass)
    {
        QtGuiSurveillanceMultiClass->setWindowTitle(QApplication::translate("QtGuiSurveillanceMultiClass", "QtGuiSurveillanceMulti", 0));
    } // retranslateUi

};

namespace Ui {
    class QtGuiSurveillanceMultiClass: public Ui_QtGuiSurveillanceMultiClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTGUISURVEILLANCEMULTI_H
