#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMovie>
#include <QTimer>
#include <QRandomGenerator>
#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QMesh>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DRender/QCamera>
#include "reportdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_pushButton_clicked();
    void updateValues();
    void on_addReportBtn_clicked();
    void on_pushButton_5_clicked();

    void on_radioButton_8_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    QMovie *movie;
    QMovie *movie2;
    void engine_off();
    QTimer *dataTimer;

    // Chart members
    QLineSeries *series;
    QValueAxis  *axisX;
    QValueAxis  *axisY;
    int timeStep = 0;          // tracks X position
    static const int MAX_POINTS = 60;  // how many points before scrolling
    void setup3DView();
    Qt3DExtras::Qt3DWindow *view3D;
    QWidget                *container3D;
    Qt3DCore::QEntity      *rootEntity;
    Qt3DCore::QTransform   *meshTransform;

    // RPM + Frequency chart
    QLineSeries *rpmSeries;
    QLineSeries *freqSeries;
    QValueAxis  *rpmFreqAxisX;
    QValueAxis  *rpmAxisY;
    QValueAxis  *freqAxisY;
    int          rpmFreqTimeStep = 0;
    static const int MAX_SYNC_POINTS = 60;
};
#endif // MAINWINDOW_H
