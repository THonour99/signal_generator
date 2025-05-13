#include "signalgenerator.h"

SignalGenerator::SignalGenerator(QObject *parent) : QObject(parent),
    m_signalType(SINE_WAVE),
    m_frequency(100.0),
    m_amplitude(1.0),
    m_dcOffset(0.0),
    m_samplingRate(RATE_1KHZ),
    m_isGenerating(false)
{
    appendToLog("信号发生器已初始化");
}

SignalGenerator::~SignalGenerator()
{
    stopGeneration();
}

void SignalGenerator::setSignalType(SignalType type)
{
    m_signalType = type;
    appendToLog(QString("信号类型设置为: %1").arg(
                type == SINE_WAVE ? "正弦波" :
                type == SQUARE_WAVE ? "方波" :
                type == TRIANGLE_WAVE ? "三角波" : "文件数据"));
}

void SignalGenerator::setFrequency(double freq)
{
    m_frequency = freq;
    appendToLog(QString("频率设置为: %1 Hz").arg(freq));
}

void SignalGenerator::setAmplitude(double amp)
{
    m_amplitude = amp;
    appendToLog(QString("幅度设置为: %1").arg(amp));
}

void SignalGenerator::setDCOffset(double offset)
{
    m_dcOffset = offset;
    appendToLog(QString("DC偏置设置为: %1").arg(offset));
}

void SignalGenerator::setSamplingRate(SamplingRate rate)
{
    m_samplingRate = rate;
    appendToLog(QString("采样率设置为: %1 Hz").arg(static_cast<int>(rate)));
}

void SignalGenerator::setDataFromFile(const QString &filePath)
{
    m_filePath = filePath;
    m_signalType = FILE_DATA;
    appendToLog(QString("数据文件设置为: %1").arg(filePath));
}

void SignalGenerator::startGeneration()
{
    if (m_isGenerating) {
        return;
    }
    
    m_isGenerating = true;
    appendToLog("开始生成信号");
    
    // 根据当前设置生成信号数据
    int numSamples = static_cast<int>(m_samplingRate) * 2; // 生成2秒的数据
    
    switch (m_signalType) {
        case SINE_WAVE:
            m_generatedData = generateSineWave(numSamples);
            break;
        case SQUARE_WAVE:
            m_generatedData = generateSquareWave(numSamples);
            break;
        case TRIANGLE_WAVE:
            m_generatedData = generateTriangleWave(numSamples);
            break;
        case FILE_DATA:
            m_generatedData = loadDataFromFile();
            break;
    }
    
    // 发送生成的信号数据
    emit signalGenerated(m_generatedData);
}

void SignalGenerator::stopGeneration()
{
    if (!m_isGenerating) {
        return;
    }
    
    m_isGenerating = false;
    appendToLog("停止生成信号");
}

bool SignalGenerator::isGenerating() const
{
    return m_isGenerating;
}

QVector<double> SignalGenerator::getGeneratedData() const
{
    return m_generatedData;
}

void SignalGenerator::appendToLog(const QString &message)
{
    QString timeStamp = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz] ");
    QString logEntry = timeStamp + message;
    m_log.append(logEntry + "\n");
    emit logUpdated(m_log);
    qDebug() << logEntry;
}

QString SignalGenerator::getLog() const
{
    return m_log;
}

QVector<double> SignalGenerator::generateSineWave(int numSamples)
{
    QVector<double> data;
    data.reserve(numSamples);
    
    double timeStep = 1.0 / m_samplingRate;
    double angularFreq = 2.0 * M_PI * m_frequency;
    
    for (int i = 0; i < numSamples; ++i) {
        double time = i * timeStep;
        double value = m_amplitude * qSin(angularFreq * time) + m_dcOffset;
        
        // 将值限制在16位有符号整数范围内 (-32768 到 32767)
        value = qBound(-32768.0, value, 32767.0);
        
        data.append(value);
    }
    
    appendToLog(QString("生成正弦波信号, 样本数: %1").arg(numSamples));
    return data;
}

QVector<double> SignalGenerator::generateSquareWave(int numSamples)
{
    QVector<double> data;
    data.reserve(numSamples);
    
    double timeStep = 1.0 / m_samplingRate;
    double period = 1.0 / m_frequency;
    
    for (int i = 0; i < numSamples; ++i) {
        double time = i * timeStep;
        double phase = fmod(time, period) / period;
        double value = (phase < 0.5 ? m_amplitude : -m_amplitude) + m_dcOffset;
        
        // 将值限制在16位有符号整数范围内
        value = qBound(-32768.0, value, 32767.0);
        
        data.append(value);
    }
    
    appendToLog(QString("生成方波信号, 样本数: %1").arg(numSamples));
    return data;
}

QVector<double> SignalGenerator::generateTriangleWave(int numSamples)
{
    QVector<double> data;
    data.reserve(numSamples);
    
    double timeStep = 1.0 / m_samplingRate;
    double period = 1.0 / m_frequency;
    
    for (int i = 0; i < numSamples; ++i) {
        double time = i * timeStep;
        double phase = fmod(time, period) / period;
        
        double value;
        if (phase < 0.5) {
            value = (4.0 * phase - 1.0) * m_amplitude;
        } else {
            value = (3.0 - 4.0 * phase) * m_amplitude;
        }
        value += m_dcOffset;
        
        // 将值限制在16位有符号整数范围内
        value = qBound(-32768.0, value, 32767.0);
        
        data.append(value);
    }
    
    appendToLog(QString("生成三角波信号, 样本数: %1").arg(numSamples));
    return data;
}

QVector<double> SignalGenerator::loadDataFromFile()
{
    QVector<double> data;
    
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        appendToLog(QString("无法打开文件: %1").arg(m_filePath));
        return data;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        bool ok;
        double value = in.readLine().trimmed().toDouble(&ok);
        if (ok) {
            // 将值限制在16位有符号整数范围内
            value = qBound(-32768.0, value, 32767.0);
            data.append(value);
        }
    }
    
    file.close();
    appendToLog(QString("从文件加载数据, 样本数: %1").arg(data.size()));
    return data;
} 