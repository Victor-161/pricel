#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSettings>
#include <QMainWindow>
#include <QVector>
#include <QMap>
#include <QPushButton>

#define  koef_M     5.0

class Sensor;
class Axle;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void processSensorData(const QString &data);

public slots:




private slots:
    // Слоты для обработки данных от датчиков
    void handleSensorData(Sensor* sensor, float velocity, qint64 timeSinceLast, int axleNum, float distance);

    // Слоты управления
    void onBlockButtonClicked();
    void onResetButtonClicked();
    void on_action_changed();
    void on_loadDataButton_clicked();       // Слот для обработки нажатия кнопки "Загрузить данные"

    // Слот для обновления визуализации
    void updateVisualization();



    void on_actionLoadConfig();

private:
    Ui::MainWindow *ui;
    QPushButton *startButton;
    QPushButton *resetButton;
    Axle *axle;

    // Основные компоненты системы
    QVector<Sensor*> sensors;  // Список датчиков
    QVector<Axle*> axles;      // Список осей

    std::vector<int> sensorId_t;
    std::vector<float> velocity_t;
    std::vector<int> timePrev_t;
    std::vector<int> axleNum_t;

    bool isBlocked;            // Флаг блокировки системы
    bool lastIsBlocked;         // Флаг предыдущего состояния системы
    bool isLoadDataActive = 0;      // Флаг активности кнопки "Загрузить данные"

    void raschetKolBas();

    // Вспомогательные методы
    void setupSystem();        // Инициализация системы
    void initSensorTable();    // Настройка таблицы данных
    void setAxlesVelocity(float velocity); // Управление скоростью осей

    // Графические элементы
    void drawTrack();          // Отрисовка пути
    void drawSensors();        // Отрисовка датчиков
    void drawAxles();          // Отрисовка осей
};

#endif // MAINWINDOW_H
