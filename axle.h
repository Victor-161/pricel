#ifndef AXLE_H
#define AXLE_H

#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QSet>
#include <QPushButton>
#include <QDebug>

#define  koef_M     5.0

class Sensor;

class Axle : public QObject
{
    Q_OBJECT
public:
    explicit Axle(int number, QObject *parent = nullptr);

    void move(float distance);
    void registerSensor(Sensor* sensor);


    // Getters
    int getNumber() const;
    float getPosition() const;
    float getVelocity() const;
    bool isBlocked() const;


    // Setters
    void setPosition(float position);
    void setVelocity(float velocity);
    void setLastIsBlocked(bool lastIsBlocked);
    void resetPosition(float newPosition);


signals:
    void passedSensor(Sensor* sensor, float velocity, qint64 timeSinceLast, int axleNum, float distance);   //, float distance

public slots:
    void startTimer();
    void stopTimer();

private slots:
    void updatePosition();


private:
    bool lastIsBlocked;
    int number;
    float position;
    float velocity;
    qint64 lastPassTime;
    QTimer moveTimer;       // Таймер
    QPushButton *startButton;
    QList<Sensor*> sensors;
    QSet<Sensor*> passedSensors; // Датчики, которые уже были пройдены
};

#endif // AXLE_H
