#ifndef CHANNELMODULE_H
#define CHANNELMODULE_H

#include <QObject>
#include <QVector>
#include <QRandomGenerator>
#include <QtMath>

class ChannelModule : public QObject
{
    Q_OBJECT
public:
    explicit ChannelModule(QObject *parent = nullptr);
    
    // 设置噪声参数
    void setNoiseAmplitude(double amplitude);
    double getNoiseAmplitude() const;
    
    // 处理信号并添加噪声
    QVector<double> processSignal(const QVector<double> &inputSignal);

signals:
    void signalProcessed(const QVector<double> &processedData);

public slots:
    void onSignalReceived(const QVector<double> &inputSignal);

private:
    // 生成高斯白噪声
    double generateGaussianNoise();
    
    double m_noiseAmplitude;
    QVector<double> m_processedData;
};

#endif // CHANNELMODULE_H 