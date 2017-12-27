#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <random>
#include <fstream>
#include "omnislimtreescene.h"

typedef double coord;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    vector<QPointF> GenerarPuntos(coord x1, coord x2, coord y1, coord y2, int n);
    void AgregarPuntosQuery(vector<QPointF> &puntos);
private slots:
    void on_query(QPointF Center, double Radio);
    void on_newElement(QPointF Pos);
    void on_GenerarPuntosButton_clicked();

    void on_LimpiarButton_clicked();

    void on_SlimDownButton_clicked();

    void on_FociButton_clicked();

    void on_GenDataButton_clicked();

private:
    Ui::MainWindow *ui;
    SlimTreeScene * scene;
    SlimTree<QPointF> * ST;

};

#endif // MAINWINDOW_H
