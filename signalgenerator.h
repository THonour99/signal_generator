#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include <QObject>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QtMath>
#include <QRandomGenerator>

// 信号类型枚举
enum SignalType {
    SINE_WAVE,      // 正弦波
    SQUARE_WAVE,    // 方波
    TRIANGLE_WAVE,  // 三角波
    FILE_DATA       // 文件数据
};

// 采样频率枚举
enum SamplingRate {
    RATE_1KHZ = 1000,
    RATE_2KHZ = 2000,
    RATE_4KHZ = 4000,
    RATE_8KHZ = 8000
};

class SignalGenerator : public QObject
{
    Q_OBJECT
public:
    explicit SignalGenerator(QObject *parent = nullptr);
    ~SignalGenerator();

    // 设置信号参数
    void setSignalType(SignalType type);
    void setFrequency(double freq);
    void setAmplitude(double amp);
    void setDCOffset(double offset);
    void setSamplingRate(SamplingRate rate);
    void setDataFromFile(const QString &filePath);

    // 开始/停止生成信号
    void startGeneration();
    void stopGeneration();

    // 获取当前状态和数据
    bool isGenerating() const;
    QVector<double> getGeneratedData() const;
    void appendToLog(const QString &message);
    QString getLog() const;

signals:
    void signalGenerated(const QVector<double> &data);
    void logUpdated(const QString &log);

private:
    // 生成数据的函数
    QVector<double> generateSineWave(int numSamples);
    QVector<double> generateSquareWave(int numSamples);
    QVector<double> generateTriangleWave(int numSamples);
    QVector<double> loadDataFromFile();

    // 属性
    SignalType m_signalType;
    double m_frequency;
    double m_amplitude;
    double m_dcOffset;
    SamplingRate m_samplingRate;
    QString m_filePath;
    
    bool m_isGenerating;
    QVector<double> m_generatedData;
    QString m_log;
};

#endif // SIGNALGENERATOR_H 