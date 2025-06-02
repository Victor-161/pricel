#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSettings>
#include <QMainWindow>
#include <QVector>
#include <QMap>

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

private slots:
    // Слоты для обработки данных от датчиков
    void handleSensorData(Sensor* sensor, float velocity, qint64 timeSinceLast, int axleNum, float distance);

    // Слоты управления
    void onBlockButtonClicked();
    void onResetButtonClicked();
    void on_action_changed();

    // Слот для обновления визуализации
    void updateVisualization();

private:
    Ui::MainWindow *ui;

    // Основные компоненты системы
    QVector<Sensor*> sensors;  // Список датчиков
    QVector<Axle*> axles;      // Список осей
    bool isBlocked;            // Флаг блокировки системы

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
