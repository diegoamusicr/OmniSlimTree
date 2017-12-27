#include "mainwindow.h"
#include "ui_mainwindow.h"

int OmniBuildDistCalcs, SlimBuildDistCalcs, DistCalcs;
int Data_OmniBuildDistCalcs, Data_SlimBuildDistCalcs, Data_DistCalcs;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ST = nullptr;
    scene = new SlimTreeScene(this);
    scene->setSceneRect(ui->slimtreeView->rect());
    ui->slimtreeView->setRenderHint(QPainter::Antialiasing);
    ui->slimtreeView->setScene(scene);
    SlimBuildDistCalcs = 0;
    OmniBuildDistCalcs = 0;
    DistCalcs = 0;

    connect(scene, SIGNAL(QueryDrew(QPointF,double)),
            this, SLOT(on_query(QPointF,double)));
    connect(scene, SIGNAL(PointCreated(QPointF)),
            this, SLOT(on_newElement(QPointF)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

double L2Distance(QPointF A, QPointF B)
{
    DistCalcs++;
    Data_DistCalcs++;
    return sqrt( pow(A.rx() - B.rx(),2) + pow(A.ry() - B.ry(),2) );
}

double L2DistanceAux(QPointF A, QPointF B)
{
    return sqrt( pow(A.rx() - B.rx(),2) + pow(A.ry() - B.ry(),2) );
}

vector<QPointF> MainWindow::GenerarPuntos(coord x1, coord x2, coord y1, coord y2, int n)
{
    vector<QPointF> Q;
    for (int i=0; i < n; i++)
    {
        double x = x1 + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(x2-x1)));;
        double y = y1 + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(y2-y1)));
        Q.push_back(QPointF(x,y));
    }
    return Q;
}

void MainWindow::AgregarPuntosQuery(vector<QPointF> &puntos)
{
    for (auto it = puntos.begin(); it != puntos.end(); it++)
    {
        QString PuntoString;
        PuntoString.append(QString::number(it->rx()));
        PuntoString.append(", ");
        PuntoString.append(QString::number(it->ry()));
        ui->queryList->addItem(PuntoString);
    }
    QString TotalPuntos("Cantidad de Puntos = ");
    TotalPuntos.append(QString::number(puntos.size()));
    ui->queryList->addItem(TotalPuntos);
}

void MainWindow::on_query(QPointF Center, double Radio)
{
    qDebug() << "Center: " << Center.rx() << "," << Center.ry();
    qDebug() << "Radio: " << Radio;
    if (ST)
    {
        vector<QPointF> PuntosQuery = ST->RangeQuery(Center, Radio);
        scene->PintarPuntosQuery(PuntosQuery);
        scene->DrawFociRadios(ST, Center, Radio);
        this->AgregarPuntosQuery(PuntosQuery);
        ui->PostEdit->setText(QString::number(DistCalcs));
    }
}

void MainWindow::on_newElement(QPointF Pos)
{
    if (ST)
    {
        ST->AddElement(Pos);
        scene->clear();
        scene->update();
        scene->DrawTree(ST);
    }
    else
    {
        ST = new SlimTree<QPointF>(ui->CapacitySpinBox->value(), L2Distance, L2DistanceAux);
        ST->AddElement(Pos);
        scene->DrawTree(ST);
    }
}

void MainWindow::on_GenerarPuntosButton_clicked()
{
    scene->clear();
    ui->queryList->clear();
    scene->update();
    int n = ui->NumPuntosSpinBox->value();
    scene->setSceneRect(ui->slimtreeView->rect());
    scene->setSceneRect(0, 0, scene->sceneRect().width() - 5, scene->sceneRect().height() - 5);
    coord x1 = scene->sceneRect().left();
    coord x2 = scene->sceneRect().right();
    coord y1 = scene->sceneRect().top();
    coord y2 = scene->sceneRect().bottom();
    vector<QPointF> puntos = GenerarPuntos(x1,x2,y1,y2,n);
    ST = new SlimTree<QPointF>(ui->CapacitySpinBox->value(), L2Distance, L2DistanceAux);
    for (int i = 0; i < puntos.size(); i++)
    {
        ST->AddElement(puntos[i]);
    }
    scene->DrawTree(ST);
    SlimBuildDistCalcs = DistCalcs;
    DistCalcs = 0;
    ui->BuildEdit->setText(QString::number(SlimBuildDistCalcs));
    ui->PostEdit->setText(QString::number(DistCalcs));
}

void MainWindow::on_LimpiarButton_clicked()
{
    delete ST;
    ST = nullptr;
    scene->clear();
    ui->queryList->clear();
    scene->update();
}

void MainWindow::on_SlimDownButton_clicked()
{
    if (ST)
    {
        scene->clear();
        ui->queryList->clear();
        scene->update();
        ST->SlimDownLeaves();
        scene->DrawTree(ST);
        SlimBuildDistCalcs += DistCalcs;
        DistCalcs = 0;
        ui->BuildEdit->setText(QString::number(SlimBuildDistCalcs));
    }
}

void MainWindow::on_FociButton_clicked()
{
    if (ST)
    {
        scene->clear();
        ui->queryList->clear();
        scene->update();
        ST->FindFoci(ui->FociSpinBox->value());
        scene->DrawTree(ST);
        OmniBuildDistCalcs = SlimBuildDistCalcs + DistCalcs;
        DistCalcs = 0;
        ui->BuildEdit->setText(QString::number(OmniBuildDistCalcs));
    }
}

void ClearData(string file)
{
    ofstream data;
    data.open(file);
    data << "";
    data.close();
}

void MainWindow::on_GenDataButton_clicked()
{
    QVector<int> TestSizes = {100, 1000, 10000, 50000, 100000};
    QVector<int> TestCapacities = {20, 50, 100, 100, 150};
    int BuildTests = 10;
    int QueryTests = 10;
    qreal x1 = -500;
    qreal x2 = 500;
    qreal y1 = -500;
    qreal y2 = 500;
    string BuildFileName = "BuildData.csv";
    ClearData(BuildFileName);
    for (int T = 0; T < TestSizes.size(); T++)
    {
        SlimTree<QPointF> * TestTree;
        double PromSlimBuildDist = 0;
        double PromOmniBuildDist = 0;
        for (int Test = 0; Test < BuildTests; Test++)
        {
            Data_DistCalcs = 0;
            TestTree = new SlimTree<QPointF>(TestCapacities[T], L2Distance, L2DistanceAux);
            vector<QPointF> puntos = GenerarPuntos(x1, x2, y1, y2, TestSizes[T]);
            for (int p = 0; p < puntos.size(); p++)
            {
                TestTree->AddElement(puntos[p]);
            }
            TestTree->SlimDownLeaves();
            PromSlimBuildDist += Data_DistCalcs;
            TestTree->FindFoci();
            PromOmniBuildDist += Data_DistCalcs;
        }
        PromSlimBuildDist /= BuildTests;
        PromOmniBuildDist /= BuildTests;
        QVector<double> PromSlimQueryDistances;
        QVector<double> PromOmniQueryDistances;
        qreal MaxRadio;
        if (abs(x1-x2) > abs(y1-y2)) MaxRadio = abs(x1-x2)/2;
        else MaxRadio = abs(y1-y2)/2;
        int TestRadios = 20;
        double dxRadio = MaxRadio/TestRadios;
        for (int R = 0; R <= TestRadios; R++)
        {
            qreal TestRadio = R*dxRadio;
            vector<QPointF> TestCenters = GenerarPuntos(x1, x2, y1, y2, QueryTests);
            double PromSlimQueryDist = 0;
            double PromOmniQueryDist = 0;
            for (int i = 0; i < TestCenters.size(); i++)
            {
                Data_DistCalcs = 0;
                TestTree->RangeQuery(TestCenters[i],TestRadio,0);
                PromSlimQueryDist += Data_DistCalcs;
                Data_DistCalcs = 0;
                TestTree->RangeQuery(TestCenters[i],TestRadio,1);
                PromOmniQueryDist += Data_DistCalcs;
            }
            PromSlimQueryDistances.push_back(PromSlimQueryDist/QueryTests);
            PromOmniQueryDistances.push_back(PromOmniQueryDist/QueryTests);
        }
        string QueryFileName = "QueryData";
        QueryFileName.append(std::to_string(TestSizes[T]));
        QueryFileName.append(".csv");
        ofstream QueryData;
        QueryData.open(QueryFileName);
        for (int i = 0; i <= TestRadios; i++)
        {
            float Radio = i/TestRadios;
            QueryData << Radio << ";" << PromSlimQueryDistances[i] << ";" << PromOmniQueryDistances[i] << std::endl;
        }
        QueryData.close();
        ofstream BuildData;
        BuildData.open(BuildFileName, ios::app);
        BuildData << TestSizes[T] << ";" << PromSlimBuildDist << ";" << PromOmniBuildDist << std::endl;
        BuildData.close();
    }
}
