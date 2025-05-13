#include "channelmodule.h"

ChannelModule::ChannelModule(QObject *parent) : QObject(parent),
    m_noiseAmplitude(1.0)
{
}

void ChannelModule::setNoiseAmplitude(double amplitude)
{
    m_noiseAmplitude = amplitude;
}

double ChannelModule::getNoiseAmplitude() const
{
    return m_noiseAmplitude;
}

QVector<double> ChannelModule::processSignal(const QVector<double> &inputSignal)
{
    QVector<double> result;
    result.reserve(inputSignal.size());
    
    // 为每个信号样本添加噪声
    for (double sample : inputSignal) {
        double noise = generateGaussianNoise() * m_noiseAmplitude;
        double processedSample = sample + noise;
        
        // 确保值在16位范围内
        processedSample = qBound(-32768.0, processedSample, 32767.0);
        
        result.append(processedSample);
    }
    
    m_processedData = result;
    return result;
}

void ChannelModule::onSignalReceived(const QVector<double> &inputSignal)
{
    QVector<double> processedSignal = processSignal(inputSignal);
    emit signalProcessed(processedSignal);
}

double ChannelModule::generateGaussianNoise()
{
    // 使用Box-Muller变换生成高斯白噪声
    static bool hasSpare = false;
    static double spare;
    
    if (hasSpare) {
        hasSpare = false;
        return spare;
    }
    
    hasSpare = true;
    
    double u1, u2;
    do {
        u1 = QRandomGenerator::global()->generateDouble();
        u2 = QRandomGenerator::global()->generateDouble();
    } while (u1 <= 1e-7); // 避免对数函数的参数为0
    
    double z1 = qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * M_PI * u2);
    spare = qSqrt(-2.0 * qLn(u1)) * qSin(2.0 * M_PI * u2);
    
    return z1;
} 