#include "oscilloscope.h"

Oscilloscope::Oscilloscope(QObject *parent) : QObject(parent),
    m_timePerDiv(0.1),
    m_triggerMode(AUTO),
    m_triggerLevel(0.0),
    m_isRunning(false)
{
    // 初始化两个通道
    m_channelData.resize(2);
    m_voltPerDiv.resize(2);
    m_voltPerDiv[0] = 1.0;
    m_voltPerDiv[1] = 1.0;
    
    // 初始化时间轴
    updateTimeAxis();
    
    // 移动到独立线程
    moveToThread(&m_thread);
}

Oscilloscope::~Oscilloscope()
{
    stopOscilloscope();
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait();
    }
}

void Oscilloscope::setTimebase(double timePerDiv)
{
    m_mutex.lock();
    m_timePerDiv = timePerDiv;
    updateTimeAxis();
    m_mutex.unlock();
}

void Oscilloscope::setVoltPerDiv(double voltPerDiv, int channel)
{
    if (channel < 0 || channel >= m_voltPerDiv.size()) {
        return;
    }
    
    m_mutex.lock();
    m_voltPerDiv[channel] = voltPerDiv;
    m_mutex.unlock();
}

void Oscilloscope::setTriggerMode(TriggerMode mode)
{
    m_mutex.lock();
    m_triggerMode = mode;
    m_mutex.unlock();
}

void Oscilloscope::setTriggerLevel(double level)
{
    m_mutex.lock();
    m_triggerLevel = level;
    m_mutex.unlock();
}

void Oscilloscope::startOscilloscope()
{
    if (!m_isRunning) {
        m_isRunning = true;
        if (!m_thread.isRunning()) {
            m_thread.start();
        }
        
        // 发出一个空信号，强制更新UI
        // 即使没有新数据，也会显示已有数据
        for (int i = 0; i < m_channelData.size(); ++i) {
            if (!m_channelData[i].isEmpty()) {
                emit dataUpdated(i, m_channelData[i]);
            }
        }
        emit timeAxisUpdated(m_timeAxis);
    }
}

void Oscilloscope::stopOscilloscope()
{
    m_isRunning = false;
}

QVector<double> Oscilloscope::getChannelData(int channel) const
{
    if (channel < 0 || channel >= m_channelData.size()) {
        return QVector<double>();
    }
    
    QVector<double> result;
    m_mutex.lock();
    result = m_channelData[channel];
    m_mutex.unlock();
    return result;
}

QVector<double> Oscilloscope::getTimeAxis() const
{
    QVector<double> result;
    m_mutex.lock();
    result = m_timeAxis;
    m_mutex.unlock();
    return result;
}

void Oscilloscope::onSignalReceived(int channel, const QVector<double> &data)
{
    if (channel < 0 || channel >= m_channelData.size()) {
        return;
    }
    
    m_mutex.lock();
    
    // 存储通道数据
    m_channelData[channel] = data;
    
    // 处理数据
    processData(channel);
    
    // 如果两个通道都有数据，则处理时间同步
    if (!m_channelData[0].isEmpty() && !m_channelData[1].isEmpty()) {
        alignSignals();
    }
    
    // 保存数据副本，在mutex外发送信号
    QVector<double> channelDataCopy = m_channelData[channel];
    QVector<double> timeAxisCopy = m_timeAxis;
    
    m_mutex.unlock();
    
    // 只有在示波器运行时才发送更新信号
    if (m_isRunning) {
        // 发送更新信号
        emit dataUpdated(channel, channelDataCopy);
        emit timeAxisUpdated(timeAxisCopy);
    }
}

void Oscilloscope::processData(int channel)
{
    // 根据触发条件处理数据
    if (m_channelData[channel].isEmpty()) {
        return;
    }
    
    // 查找触发点
    int triggerIndex = -1;
    
    if (m_triggerMode != AUTO) {
        for (int i = 1; i < m_channelData[channel].size(); ++i) {
            // 上升沿触发
            if (m_channelData[channel][i-1] < m_triggerLevel && 
                m_channelData[channel][i] >= m_triggerLevel) {
                triggerIndex = i;
                break;
            }
        }
        
        // 如果找到触发点，重新排列数据
        if (triggerIndex > 0) {
            QVector<double> temp = m_channelData[channel];
            m_channelData[channel].clear();
            
            // 从触发点开始重排数据
            for (int i = triggerIndex; i < temp.size(); ++i) {
                m_channelData[channel].append(temp[i]);
            }
            
            // 补充剩余的数据
            for (int i = 0; i < triggerIndex; ++i) {
                m_channelData[channel].append(temp[i]);
            }
        }
    }
}

void Oscilloscope::updateTimeAxis()
{
    // 创建时间轴
    m_timeAxis.clear();
    
    // 假设一个屏幕上显示10个时间格
    int numPoints = 1000; // 每个通道的采样点数
    double totalTime = 10 * m_timePerDiv;
    double timeStep = totalTime / numPoints;
    
    for (int i = 0; i < numPoints; ++i) {
        m_timeAxis.append(i * timeStep);
    }
}

void Oscilloscope::alignSignals()
{
    // 只有当两个通道都有足够的数据时才进行同步
    if (m_channelData[0].size() < 10 || m_channelData[1].size() < 10) {
        return;
    }
    
    // 为了简化，我们假设两个通道的采样率相同，
    // 只是通过简单地对齐起始点来同步
    
    // 寻找两个信号中的第一个显著特征点（例如，最大值位置）
    int maxIndex0 = 0;
    int maxIndex1 = 0;
    double maxVal0 = m_channelData[0][0];
    double maxVal1 = m_channelData[1][0];
    
    for (int i = 1; i < qMin(m_channelData[0].size(), m_channelData[1].size()); ++i) {
        if (m_channelData[0][i] > maxVal0) {
            maxVal0 = m_channelData[0][i];
            maxIndex0 = i;
        }
        
        if (m_channelData[1][i] > maxVal1) {
            maxVal1 = m_channelData[1][i];
            maxIndex1 = i;
        }
    }
    
    // 计算偏移量
    int offset = maxIndex1 - maxIndex0;
    
    // 如果偏移量显著，则调整第二个通道的数据
    if (qAbs(offset) > 5) {
        QVector<double> temp = m_channelData[1];
        m_channelData[1].clear();
        
        if (offset > 0) {
            // 第二个通道需要向左移动
            for (int i = offset; i < temp.size(); ++i) {
                m_channelData[1].append(temp[i]);
            }
            
            // 用零填充末尾
            for (int i = 0; i < offset; ++i) {
                m_channelData[1].append(0.0);
            }
        } else {
            // 第二个通道需要向右移动
            offset = -offset;
            
            // 用零填充开头
            for (int i = 0; i < offset; ++i) {
                m_channelData[1].append(0.0);
            }
            
            // 添加剩余数据
            for (int i = 0; i < temp.size() - offset; ++i) {
                m_channelData[1].append(temp[i]);
            }
        }
    }
} 