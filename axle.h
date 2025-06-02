#ifndef AXLE_H
#define AXLE_H

#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QSet>

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
    //    qint64 getLastDetectionTime() const;
    //    int getLastSensorId() const;

    // Setters
    void setPosition(float position);
    void setVelocity(float velocity);
    //  void updateDetection(qint64 time, int sensorId);
    void resetPosition(float newPosition);

    // Calculation
    float calculateDistanceTo(const Axle &other) const;

signals:
    void passedSensor(Sensor* sensor, float velocity, qint64 timeSinceLast, int axleNum, float distance);   //, float distance

private slots:
    void updatePosition();

private:
    int number;
    float position;
    float velocity;
    qint64 lastPassTime;
    QTimer moveTimer;
    QList<Sensor*> sensors;
    QSet<Sensor*> passedSensors; // Датчики, которые уже были пройдены
};

#endif // AXLE_H
