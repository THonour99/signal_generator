#include "receiveanalyzer.h"

ReceiveAnalyzer::ReceiveAnalyzer(QObject *parent) : QObject(parent),
    m_filterCutoff(500.0),
    m_samplingRate(8000)
{
}

QVector<double> ReceiveAnalyzer::applyLowPassFilter(const QVector<double> &inputSignal, double cutoffFrequency, int samplingRate)
{
    if (inputSignal.isEmpty()) {
        return QVector<double>();
    }
    
    // 简单的移动平均滤波器
    int windowSize = static_cast<int>(samplingRate / cutoffFrequency);
    if (windowSize < 3) windowSize = 3;
    if (windowSize % 2 == 0) windowSize++; // 确保窗口大小为奇数
    
    int halfWindow = windowSize / 2;
    QVector<double> filteredSignal;
    filteredSignal.reserve(inputSignal.size());
    
    for (int i = 0; i < inputSignal.size(); ++i) {
        double sum = 0.0;
        int count = 0;
        
        for (int j = -halfWindow; j <= halfWindow; ++j) {
            int index = i + j;
            if (index >= 0 && index < inputSignal.size()) {
                sum += inputSignal[index];
                count++;
            }
        }
        
        filteredSignal.append(sum / count);
    }
    
    return filteredSignal;
}

QVector<double> ReceiveAnalyzer::calculateFFT(const QVector<double> &inputSignal, int samplingRate)
{
    if (inputSignal.isEmpty()) {
        return QVector<double>();
    }
    
    // 找到最接近的2的幂次方
    int fftSize = 1;
    while (fftSize < inputSignal.size()) {
        fftSize <<= 1;
    }
    
    // 将信号转换为复数格式并填充零
    QVector<std::complex<double>> complexSignal;
    complexSignal.reserve(fftSize);
    
    for (int i = 0; i < fftSize; ++i) {
        if (i < inputSignal.size()) {
            complexSignal.append(std::complex<double>(inputSignal[i], 0.0));
        } else {
            complexSignal.append(std::complex<double>(0.0, 0.0));
        }
    }
    
    // 执行FFT
    fft(complexSignal);
    
    // 计算幅度谱
    QVector<double> magnitudeSpectrum;
    magnitudeSpectrum.reserve(fftSize / 2);
    
    for (int i = 0; i < fftSize / 2; ++i) {
        double magnitude = std::abs(complexSignal[i]) * 2.0 / fftSize;
        magnitudeSpectrum.append(magnitude);
    }
    
    // 创建频率轴
    m_frequencyAxis = getFrequencyAxis(fftSize, samplingRate);
    
    return magnitudeSpectrum;
}

QVector<double> ReceiveAnalyzer::getFrequencyAxis(int fftSize, int samplingRate)
{
    QVector<double> freqAxis;
    freqAxis.reserve(fftSize / 2);
    
    for (int i = 0; i < fftSize / 2; ++i) {
        double frequency = static_cast<double>(i) * samplingRate / fftSize;
        freqAxis.append(frequency);
    }
    
    return freqAxis;
}

bool ReceiveAnalyzer::saveDataToFile(const QVector<double> &data, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件进行写入:" << filePath;
        return false;
    }
    
    QTextStream out(&file);
    
    // 写入时间戳
    out << "# 时间戳: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    out << "# 数据点数: " << data.size() << "\n";
    
    // 写入数据
    for (int i = 0; i < data.size(); ++i) {
        out << data[i] << "\n";
    }
    
    file.close();
    return true;
}

QVector<double> ReceiveAnalyzer::getRawData() const
{
    return m_rawData;
}

QVector<double> ReceiveAnalyzer::getFilteredData() const
{
    return m_filteredData;
}

QVector<double> ReceiveAnalyzer::getSpectrumData() const
{
    return m_spectrumData;
}

QVector<double> ReceiveAnalyzer::getFrequencyData() const
{
    return m_frequencyAxis;
}

void ReceiveAnalyzer::onSignalReceived(const QVector<double> &signal)
{
    m_rawData = signal;
    emit dataReceived(m_rawData);
    processReceivedData();
}

void ReceiveAnalyzer::processReceivedData()
{
    // 应用低通滤波
    m_filteredData = applyLowPassFilter(m_rawData, m_filterCutoff, m_samplingRate);
    emit filteredDataReady(m_filteredData);
    
    // 执行频谱分析
    m_spectrumData = calculateFFT(m_rawData, m_samplingRate);
    emit spectrumDataReady(m_spectrumData, m_frequencyAxis);
}

void ReceiveAnalyzer::setFilterCutoff(double cutoffFrequency)
{
    m_filterCutoff = cutoffFrequency;
    if (!m_rawData.isEmpty()) {
        processReceivedData();
    }
}

void ReceiveAnalyzer::fft(QVector<std::complex<double>> &x)
{
    const size_t N = x.size();
    if (N <= 1) return;
    
    // 分治法: 分离偶数和奇数下标
    QVector<std::complex<double>> even(N/2), odd(N/2);
    for (size_t i = 0; i < N/2; ++i) {
        even[i] = x[2*i];
        odd[i] = x[2*i + 1];
    }
    
    // 递归地处理子问题
    fft(even);
    fft(odd);
    
    // 合并结果
    for (size_t k = 0; k < N/2; ++k) {
        std::complex<double> t = std::polar(1.0, -2 * M_PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N/2] = even[k] - t;
    }
} 