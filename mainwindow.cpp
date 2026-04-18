#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMovie>
#include <QTimer>
#include <QRandomGenerator>
#include <kled.h>
#include <QtCharts/QChart>
#include "reportdialog.h"
#include <QDateTime>
#include <QListWidgetItem>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QMesh>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QDirectionalLight>
#include <QVBoxLayout>
#include <Qt3DExtras/QForwardRenderer>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , movie(new QMovie(":/icons/grn.gif", QByteArray(), this))   // ← init here
    , movie2(new QMovie(":/icons/red.gif", QByteArray(), this))  // ← init here
    , dataTimer(new QTimer(this))
    , series(new QLineSeries())
    , axisX(new QValueAxis())
    , axisY(new QValueAxis())
{
    ui->setupUi(this);

    setup3DView();
    // --- Chart setup ---
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Active Power Flow (kW)");
    chart->legend()->hide();
    chart->setBackgroundBrush(QColor("#0E1117")); // match your dark theme
    chart->setTitleBrush(QColor("#E2E8F0"));

    // X axis — time steps
    axisX->setRange(0, MAX_POINTS);
    axisX->setLabelFormat("%d");
    axisX->setTitleText("Time (s)");
    axisX->setLabelsColor(QColor("#94A3B8"));
    axisX->setTitleBrush(QColor("#94A3B8"));
    axisX->setGridLineColor(QColor("#1E2A3A"));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Y axis — value range
    axisY->setRange(0, 1000);
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Power");
    axisY->setLabelsColor(QColor("#94A3B8"));
    axisY->setTitleBrush(QColor("#94A3B8"));
    axisY->setGridLineColor(QColor("#1E2A3A"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Line colour — electric cyan to match your palette
    series->setColor(QColor("#00C8FF"));

    // Attach chart to the promoted QChartView widget
    ui->widget->setChart(chart);
    ui->widget->setRenderHint(QPainter::Antialiasing);

    if (!movie->isValid()) {
        qDebug() << "GIF invalid or not found!";
        return;
    }

    //ui->label_4->setMovie(movie);
    //ui->label_4->setScaledContents(true);
    movie->start();
    // Fire updateValues() every 1000ms
    connect(dataTimer, &QTimer::timeout, this, &MainWindow::updateValues);
    dataTimer->start(1000);

    QColor barColor = QColor(255, 0, 0);
    QString style = QString("QProgressBar::chunk {    background-color: %1;}").arg(barColor.name());



    // ── RPM / Frequency sync chart setup ──────────────────

    rpmSeries  = new QLineSeries();
    freqSeries = new QLineSeries();

    rpmSeries->setName("RPM");
    freqSeries->setName("Frequency (Hz)");

    // Default colours — will change dynamically in updateValues()
    rpmSeries->setColor(QColor("#00C8FF"));   // cyan
    freqSeries->setColor(QColor("#FBBF24"));  // amber

    QChart *syncChart = new QChart();
    syncChart->addSeries(rpmSeries);
    syncChart->addSeries(freqSeries);
    syncChart->setTitle("Engine RPM & Frequency");
    syncChart->setBackgroundBrush(QColor("#0E1117"));
    syncChart->setTitleBrush(QColor("#E2E8F0"));
    syncChart->legend()->setLabelColor(QColor("#94A3B8"));

    // X axis — shared time axis
    rpmFreqAxisX = new QValueAxis();
    rpmFreqAxisX->setRange(0, MAX_SYNC_POINTS);
    rpmFreqAxisX->setLabelFormat("%d");
    rpmFreqAxisX->setTitleText("Time (s)");
    rpmFreqAxisX->setLabelsColor(QColor("#94A3B8"));
    rpmFreqAxisX->setTitleBrush(QColor("#94A3B8"));
    rpmFreqAxisX->setGridLineColor(QColor("#1E2A3A"));
    syncChart->addAxis(rpmFreqAxisX, Qt::AlignBottom);

    // Y axis LEFT — RPM (0–2000)
    rpmAxisY = new QValueAxis();
    rpmAxisY->setRange(0, 2000);
    rpmAxisY->setLabelFormat("%d");
    rpmAxisY->setTitleText("Engine RPM");
    rpmAxisY->setLabelsColor(QColor("#00C8FF"));
    rpmAxisY->setTitleBrush(QColor("#00C8FF"));
    rpmAxisY->setGridLineColor(QColor("#1E2A3A"));
    syncChart->addAxis(rpmAxisY, Qt::AlignLeft);

    // Y axis RIGHT — Frequency (40–60 Hz)
    freqAxisY = new QValueAxis();
    freqAxisY->setRange(40, 60);
    freqAxisY->setLabelFormat("%.1f");
    freqAxisY->setTitleText("Hz");
    freqAxisY->setLabelsColor(QColor("#FBBF24"));
    freqAxisY->setTitleBrush(QColor("#FBBF24"));
    freqAxisY->setGridLineColor(QColor("#1E2A3A"));
    syncChart->addAxis(freqAxisY, Qt::AlignRight);

    // Attach series to axes
    rpmSeries->attachAxis(rpmFreqAxisX);
    rpmSeries->attachAxis(rpmAxisY);
    freqSeries->attachAxis(rpmFreqAxisX);
    freqSeries->attachAxis(freqAxisY);

    // Attach to widget
    ui->syncChartView->setChart(syncChart);
    ui->syncChartView->setRenderHint(QPainter::Antialiasing);
}

MainWindow::~MainWindow()
{
    delete ui;
    // movie and movie2 are parented to `this`, Qt cleans them up automatically
}

void MainWindow::on_pushButton_clicked()
{
    engine_off();  // no need for MainWindow:: prefix inside the class
}


void MainWindow::updateValues()
{
    int rpm = QRandomGenerator::global()->bounded(1400u, 1601u); // tight range around 1500
    //ui->label->setText(QString::number(rpm) + " RPM");
    //ui->progressBar->setValue(QRandomGenerator::global()->bounded(0u, 101u));
    int lcdRaw = QRandomGenerator::global()->bounded(0u, 1000u);
    //ui->lcdNumber->display(lcdRaw / 10.0);
    // ── RPM line colour — red if out of sync ───────────
    // Synchronous = 1500 RPM ± 30 tolerance
    bool rpmInSync = (rpm >= 1470 && rpm <= 1530);
    rpmSeries->setColor(rpmInSync ? QColor("#00C8FF") : QColor("#FF4D6A"));

    // ── Frequency derived from RPM (50Hz at 1500RPM) ───
    // f = (RPM × poles) / 120  →  for 4-pole: f = RPM / 30
    double freq = rpm / 30.0;

    // Frequency line colour — red if out of 49.5–50.5 Hz band
    bool freqInSync = (freq >= 49.5 && freq <= 50.5);
    freqSeries->setColor(freqInSync ? QColor("#FBBF24") : QColor("#7a1f00"));

    // ── Append to chart ────────────────────────────────
    rpmSeries->append(rpmFreqTimeStep, rpm);
    freqSeries->append(rpmFreqTimeStep, freq);

    // Scroll X once full
    if (rpmFreqTimeStep > MAX_SYNC_POINTS) {
        rpmFreqAxisX->setRange(rpmFreqTimeStep - MAX_SYNC_POINTS, rpmFreqTimeStep);
    }

    rpmFreqTimeStep++;



    // --- QLabel: shows a value like RPM, temp, voltage etc ---
    double labelVal = QRandomGenerator::global()->bounded(4900,5200)/100;
    ui->freqlcd->display(labelVal);

    // --- QProgressBar: expects a value between its min/max (default 0–100) --


    double lcdVal = QRandomGenerator::global()->bounded(800, 875) / 1.0; // 0.0–99.9
    ui->lcdkW->display(lcdVal);
    ui->loadprogressBar_4->setValue(lcdVal);

    double phaseValR = QRandomGenerator::global()->bounded(2100,2420) / 10.0;
    double phaseValY = QRandomGenerator::global()->bounded(2100,2420) / 10.0;
    double phaseValB = QRandomGenerator::global()->bounded(2100,2420) / 10.0;
    ui->VOLTR->display(phaseValR);
    ui->VOLTG->display(phaseValY);
    ui->VOLTY->display(phaseValB);
    ui->VOLTR_4->display(phaseValR);
    ui->VOLTG_4->display(phaseValY);
    ui->VOLTY_4->display(phaseValB);
    ui->VOLTR_5->display(phaseValR);
    ui->VOLTG_5->display(phaseValY);
    ui->VOLTY_5->display(phaseValB);







    // Random value in range
    //int rpm = QRandomGenerator::global()->bounded(0u, 8000u);

    // Add point to chart
    series->append(timeStep, lcdVal);

    // Scroll X axis once we hit MAX_POINTS
    if (timeStep > MAX_POINTS) {
        axisX->setRange(timeStep - MAX_POINTS, timeStep);
    }

    timeStep++;
}


void MainWindow::setup3DView()
{
    // ── 3D window & container ──────────────────────────
    view3D     = new Qt3DExtras::Qt3DWindow();
    container3D = QWidget::createWindowContainer(view3D, this);
    //container3D->setMinimumSize(400, 300);
    container3D->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Embed into the placeholder widget from Designer
    QVBoxLayout *layout = new QVBoxLayout(ui->generatorViewContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(container3D);

    // ── Root entity ────────────────────────────────────
    rootEntity = new Qt3DCore::QEntity();

    // ── Mesh ───────────────────────────────────────────
    Qt3DCore::QEntity  *meshEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QMesh  *mesh       = new Qt3DRender::QMesh(meshEntity);

    // Point this to your .stl or .obj file
    // For a resource file:   "qrc:/models/generator.stl"
    // For an absolute path:  "file:///mnt/LIN_DATA/DG_SET/models/generator.obj"
    mesh->setSource(QUrl("qrc:/models/original.stl"));

    // ── Material ───────────────────────────────────────
    Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial(meshEntity);
    material->setDiffuse(QColor("#1E2A3A"));   // dark steel — matches your palette
    material->setSpecular(QColor("#00C8FF"));  // cyan specular highlight
    material->setShininess(80.0f);

    // ── Transform ──────────────────────────────────────
    meshTransform = new Qt3DCore::QTransform(meshEntity);
    meshTransform->setScale(0.05f);      // adjust scale to fit your model
    meshTransform->setRotationX(-90.0f); // STL files are often Z-up, flip to Y-up

    meshEntity->addComponent(mesh);
    meshEntity->addComponent(material);
    meshEntity->addComponent(meshTransform);

    // ── Lighting ───────────────────────────────────────
    Qt3DCore::QEntity          *lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QDirectionalLight *light    = new Qt3DRender::QDirectionalLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(2.5f);
    Qt3DCore::QTransform       *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(QVector3D(10.0f, 10.0f, 10.0f));
    lightEntity->addComponent(light);
    lightEntity->addComponent(lightTransform);

    // ── Camera ─────────────────────────────────────────
    Qt3DRender::QCamera *camera = view3D->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0.0f, 5.0f, 10.0f));
    camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    // ── Orbit controller (mouse drag to rotate) ────────
    Qt3DExtras::QOrbitCameraController *camController =
        new Qt3DExtras::QOrbitCameraController(rootEntity);
    camController->setCamera(camera);
    camController->setLinearSpeed(10.0f);
    camController->setLookSpeed(180.0f);

    // ── Background colour ──────────────────────────────
    view3D->defaultFrameGraph()->setClearColor(QColor("#0E1117")); // your dark bg

    view3D->setRootEntity(rootEntity);
}


void MainWindow::on_addReportBtn_clicked()
{
    ReportDialog dialog(this);

    if (dialog.exec() != QDialog::Accepted)
        return;   // user cancelled, do nothing

    QString timestamp = QDateTime::currentDateTime()
                            .toString("yyyy-MM-dd  hh:mm:ss");
    QString severity  = dialog.severity();
    QString title     = dialog.title();
    QString desc      = dialog.description();

    QString entry = QString("[%1]  %2  |  %3\n    %4")
                        .arg(timestamp)
                        .arg(severity)
                        .arg(title)
                        .arg(desc);

    QListWidgetItem *item = new QListWidgetItem(entry);

    if (severity.contains("Error")) {
        item->setForeground(QColor("#FF4D6A"));  // red
    } else if (severity.contains("Warning")) {
        item->setForeground(QColor("#FBBF24"));  // amber
    } else {
        item->setForeground(QColor("#00C8FF"));  // cyan
    }

    ui->reportList->insertItem(0, item);  // newest at top
}



void MainWindow::engine_off()
{
    ui->engstat->setMovie(movie2);
    ui->engstat->setScaledContents(true);
    ui->avrstat->setMovie(movie2);
    ui->avrstat->setScaledContents(true);
    movie2->start();
    dataTimer->stop();
    ui->VOLTR->display(0);
    ui->VOLTG->display(0);
    ui->VOLTY->display(0);
    ui->rpkled->off();
    ui->ypkled_2->off();
    ui->blkled_3->off();

    QString timestamp = QDateTime::currentDateTime()
                            .toString("yyyy-MM-dd  hh:mm:ss");

    QString entry = QString("[%1]  🔴 ERROR  |  Emergency\n"
                            "    Emergency Button pressed! "
                            "Halting fuel pump power supply and generator CB.")
                        .arg(timestamp);

    QListWidgetItem *item = new QListWidgetItem(entry);
    item->setIcon(QIcon::fromTheme("dialog-error"));
    item->setForeground(QColor("#FF4D6A"));

    ui->reportList->insertItem(0, item);


}
void MainWindow::on_pushButton_5_clicked()
{
    ui->engstat->setMovie(movie);
    ui->engstat->setScaledContents(true);
    ui->avrstat->setMovie(movie);
    ui->avrstat->setScaledContents(true);
    movie2->start();
    dataTimer->start();

    ui->rpkled->on();
    ui->ypkled_2->on();
    ui->blkled_3->on();
    QString timestamp = QDateTime::currentDateTime()
                            .toString("yyyy-MM-dd  hh:mm:ss");

    QString entry = QString("[%1]  ℹ️ INFO  |  RESET\n"
                            "    Resetting systems after emergency! "
                            "Generator fuel pump relay on. Alternator CB on. Starting engine after emergency.")
                        .arg(timestamp);

    QListWidgetItem *item = new QListWidgetItem(entry);
    item->setIcon(QIcon::fromTheme("dialog-error"));
    item->setForeground(QColor("#00C8FF"));

    ui->reportList->insertItem(0, item);
}


void MainWindow::on_radioButton_8_clicked(bool checked)
{
    if(!checked)
        ui->label_59->setText("0 A");
    else
        ui->label_59->setText("22 A");
}

