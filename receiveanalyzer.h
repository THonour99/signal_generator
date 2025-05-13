#ifndef RECEIVEANALYZER_H
#define RECEIVEANALYZER_H

#include <QObject>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <complex>
#include <QFileDialog>

class ReceiveAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit ReceiveAnalyzer(QObject *parent = nullptr);
    
    // 滤波处理
    QVector<double> applyLowPassFilter(const QVector<double> &inputSignal, double cutoffFrequency, int samplingRate);
    
    // 频谱分析
    QVector<double> calculateFFT(const QVector<double> &inputSignal, int samplingRate);
    QVector<double> getFrequencyAxis(int fftSize, int samplingRate);
    
    // 保存接收数据到文件
    bool saveDataToFile(const QVector<double> &data, const QString &filePath);
    
    // 获取当前数据
    QVector<double> getRawData() const;
    QVector<double> getFilteredData() const;
    QVector<double> getSpectrumData() const;
    QVector<double> getFrequencyData() const;

signals:
    void dataReceived(const QVector<double> &data);
    void filteredDataReady(const QVector<double> &filteredData);
    void spectrumDataReady(const QVector<double> &spectrumData, const QVector<double> &freqAxis);

public slots:
    void onSignalReceived(const QVector<double> &signal);
    void processReceivedData();
    void setFilterCutoff(double cutoffFrequency);

private:
    // 快速傅里叶变换
    void fft(QVector<std::complex<double>> &x);
    
    QVector<double> m_rawData;
    QVector<double> m_filteredData;
    QVector<double> m_spectrumData;
    QVector<double> m_frequencyAxis;
    double m_filterCutoff;
    int m_samplingRate;
};

#endif // RECEIVEANALYZER_H 