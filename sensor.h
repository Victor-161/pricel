#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>
#include <QPointF>
#include "axle.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QBrush>



class Sensor : public QObject
{
    Q_OBJECT
public:
    explicit Sensor(int id, QObject *parent = nullptr);

    int getId() const;
    QPointF getPosition() const;
    float getLastVelocity() const;
    qint64 getLastTimeSinceLast() const;
    int getLastAxleNum() const ;
    float getDistanceToNextAxle() const;
    void setPosition(const QPointF &pos);

    void handleAxlePass(Sensor* sensor, float velocity, qint64 timeSinceLast, int axleNum, float distance);

signals:
    void newData(int sensorId, float velocity, qint64 timeSinceLast, int axleNum, float distance);

private:
    int id;
    QPointF position;

    float lastVelocity;
    int lastTimePrev;
    int lastAxleNum;
    float distanceToNextAxle;
    qint64 lastTimeSinceLast;
    QMap<int, qint64> axlePassTimes;  // Время прохода каждой оси
    QMap<int, float> axleSpeed;   // Скорости осей при проходе

};

#endif // SENSOR_H
