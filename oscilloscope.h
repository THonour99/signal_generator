#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <QObject>
#include <QVector>
#include <QThread>
#include <QRecursiveMutex>
#include <QWaitCondition>
#include <QElapsedTimer>

class Oscilloscope : public QObject
{
    Q_OBJECT
public:
    explicit Oscilloscope(QObject *parent = nullptr);
    ~Oscilloscope();
    
    // 示波器配置
    void setTimebase(double timePerDiv);
    void setVoltPerDiv(double voltPerDiv, int channel);
    
    // 设置触发模式
    enum TriggerMode {
        AUTO,
        NORMAL,
        SINGLE
    };
    void setTriggerMode(TriggerMode mode);
    void setTriggerLevel(double level);
    
    // 启动/停止示波器
    void startOscilloscope();
    void stopOscilloscope();
    
    // 获取示波器数据
    QVector<double> getChannelData(int channel) const;
    QVector<double> getTimeAxis() const;
    
signals:
    void dataUpdated(int channel, const QVector<double> &data);
    void timeAxisUpdated(const QVector<double> &timeAxis);
    
public slots:
    void onSignalReceived(int channel, const QVector<double> &data);
    
private:
    QVector<QVector<double>> m_channelData; // 存储两个通道的数据
    QVector<double> m_timeAxis;
    
    double m_timePerDiv;
    QVector<double> m_voltPerDiv;
    
    TriggerMode m_triggerMode;
    double m_triggerLevel;
    
    bool m_isRunning;
    QThread m_thread;
    mutable QRecursiveMutex m_mutex;
    
    void processData(int channel);
    void updateTimeAxis();
    void alignSignals(); // 用于处理两个通道的时间同步
};

#endif // OSCILLOSCOPE_H 