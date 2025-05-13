#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QFileDialog>
#include <limits>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // 设置窗口标题
    setWindowTitle("信号发生-传输-接收分析简易系统");
    
    // 初始化各模块
    setupSignalGenerator();
    setupChannelModule();
    setupReceiveAnalyzer();
    setupOscilloscope();
    
    // 设置图表
    setupCharts();
    
    // 连接信号与槽
    connectSignals();
}

MainWindow::~MainWindow()
{
    delete ui;
    
    // 清理资源
    delete m_signalGenerator;
    delete m_channelModule;
    delete m_receiveAnalyzer;
    delete m_oscilloscope;
}

// 信号发生器相关
void MainWindow::setupSignalGenerator()
{
    m_signalGenerator = new SignalGenerator(this);
}

void MainWindow::on_signalTypeComboBox_currentIndexChanged(int index)
{
    switch (index) {
        case 0: // 正弦波
            m_signalGenerator->setSignalType(SINE_WAVE);
            break;
        case 1: // 方波
            m_signalGenerator->setSignalType(SQUARE_WAVE);
            break;
        case 2: // 三角波
            m_signalGenerator->setSignalType(TRIANGLE_WAVE);
            break;
        case 3: // 文件数据
            on_loadFileButton_clicked();
            break;
    }
}

void MainWindow::on_frequencySpinBox_valueChanged(double value)
{
    m_signalGenerator->setFrequency(value);
}

void MainWindow::on_amplitudeSpinBox_valueChanged(double value)
{
    m_signalGenerator->setAmplitude(value);
}

void MainWindow::on_dcOffsetSpinBox_valueChanged(double value)
{
    m_signalGenerator->setDCOffset(value);
}

void MainWindow::on_samplingRateComboBox_currentIndexChanged(int index)
{
    switch (index) {
        case 0: // 1KHz
            m_signalGenerator->setSamplingRate(RATE_1KHZ);
            break;
        case 1: // 2KHz
            m_signalGenerator->setSamplingRate(RATE_2KHZ);
            break;
        case 2: // 4KHz
            m_signalGenerator->setSamplingRate(RATE_4KHZ);
            break;
        case 3: // 8KHz
            m_signalGenerator->setSamplingRate(RATE_8KHZ);
            break;
    }
}

void MainWindow::on_loadFileButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择数据文件", "", "文本文件 (*.txt);;所有文件 (*)");
    if (!filePath.isEmpty()) {
        ui->filePathEdit->setText(filePath);
        m_signalGenerator->setDataFromFile(filePath);
    }
}

void MainWindow::on_startGeneratorButton_clicked()
{
    m_signalGenerator->startGeneration();
    ui->startGeneratorButton->setEnabled(false);
    ui->stopGeneratorButton->setEnabled(true);
}

void MainWindow::on_stopGeneratorButton_clicked()
{
    m_signalGenerator->stopGeneration();
    ui->startGeneratorButton->setEnabled(true);
    ui->stopGeneratorButton->setEnabled(false);
}

// 信道控制相关
void MainWindow::setupChannelModule()
{
    m_channelModule = new ChannelModule(this);
}

void MainWindow::on_noiseAmplitudeSlider_valueChanged(int value)
{
    double amplitude = value / 10.0;
    ui->noiseAmplitudeLabel->setText(QString::number(amplitude));
    m_channelModule->setNoiseAmplitude(amplitude);
}

// 接收分析相关
void MainWindow::setupReceiveAnalyzer()
{
    m_receiveAnalyzer = new ReceiveAnalyzer(this);
}

void MainWindow::on_filterCutoffSpinBox_valueChanged(double value)
{
    m_receiveAnalyzer->setFilterCutoff(value);
}

void MainWindow::on_saveDataButton_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "保存数据", "", "文本文件 (*.txt);;所有文件 (*)");
    if (!filePath.isEmpty()) {
        if (m_receiveAnalyzer->saveDataToFile(m_receiveAnalyzer->getRawData(), filePath)) {
            QMessageBox::information(this, "保存成功", "数据已成功保存到文件！");
        } else {
            QMessageBox::warning(this, "保存失败", "保存数据时出错！");
        }
    }
}

// 示波器控制相关
void MainWindow::setupOscilloscope()
{
    m_oscilloscope = new Oscilloscope(this);
}

void MainWindow::on_timePerDivSpinBox_valueChanged(double value)
{
    m_oscilloscope->setTimebase(value);
}

void MainWindow::on_voltPerDivSpinBox_valueChanged(double value)
{
    // 暂时只设置通道0的值
    m_oscilloscope->setVoltPerDiv(value, 0);
}

void MainWindow::on_triggerLevelSpinBox_valueChanged(double value)
{
    m_oscilloscope->setTriggerLevel(value);
}

void MainWindow::on_triggerModeComboBox_currentIndexChanged(int index)
{
    m_oscilloscope->setTriggerMode(static_cast<Oscilloscope::TriggerMode>(index));
}

void MainWindow::on_startOscilloscopeButton_clicked()
{
    m_oscilloscope->startOscilloscope();
    ui->startOscilloscopeButton->setEnabled(false);
    ui->stopOscilloscopeButton->setEnabled(true);
}

void MainWindow::on_stopOscilloscopeButton_clicked()
{
    m_oscilloscope->stopOscilloscope();
    ui->startOscilloscopeButton->setEnabled(true);
    ui->stopOscilloscopeButton->setEnabled(false);
}

// 图表设置
void MainWindow::setupCharts()
{
    // 信号发生器图表
    m_generatorChart = new QChart();
    m_generatorChart->setTitle("信号发生器输出");
    m_generatorSeries = new QLineSeries();
    m_generatorChart->addSeries(m_generatorSeries);
    
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("时间(秒)");
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("幅度");
    
    m_generatorChart->addAxis(axisX, Qt::AlignBottom);
    m_generatorChart->addAxis(axisY, Qt::AlignLeft);
    m_generatorSeries->attachAxis(axisX);
    m_generatorSeries->attachAxis(axisY);
    
    m_generatorChartView = new QChartView(m_generatorChart);
    m_generatorChartView->setRenderHint(QPainter::Antialiasing);
    ui->generatorChartLayout->addWidget(m_generatorChartView);
    
    // 信道图表
    m_channelChart = new QChart();
    m_channelChart->setTitle("信道输出");
    m_channelSeries = new QLineSeries();
    m_channelChart->addSeries(m_channelSeries);
    
    axisX = new QValueAxis();
    axisX->setTitleText("时间(秒)");
    axisY = new QValueAxis();
    axisY->setTitleText("幅度");
    
    m_channelChart->addAxis(axisX, Qt::AlignBottom);
    m_channelChart->addAxis(axisY, Qt::AlignLeft);
    m_channelSeries->attachAxis(axisX);
    m_channelSeries->attachAxis(axisY);
    
    m_channelChartView = new QChartView(m_channelChart);
    m_channelChartView->setRenderHint(QPainter::Antialiasing);
    ui->channelChartLayout->addWidget(m_channelChartView);
    
    // 接收器图表
    m_receiverChart = new QChart();
    m_receiverChart->setTitle("滤波后信号");
    m_receiverSeries = new QLineSeries();
    m_receiverChart->addSeries(m_receiverSeries);
    
    axisX = new QValueAxis();
    axisX->setTitleText("时间(秒)");
    axisY = new QValueAxis();
    axisY->setTitleText("幅度");
    
    m_receiverChart->addAxis(axisX, Qt::AlignBottom);
    m_receiverChart->addAxis(axisY, Qt::AlignLeft);
    m_receiverSeries->attachAxis(axisX);
    m_receiverSeries->attachAxis(axisY);
    
    m_receiverChartView = new QChartView(m_receiverChart);
    m_receiverChartView->setRenderHint(QPainter::Antialiasing);
    ui->receiverChartLayout->addWidget(m_receiverChartView);
    
    // 频谱图表
    m_spectrumChart = new QChart();
    m_spectrumChart->setTitle("频谱分析");
    m_spectrumSeries = new QLineSeries();
    m_spectrumChart->addSeries(m_spectrumSeries);
    
    axisX = new QValueAxis();
    axisX->setTitleText("频率(Hz)");
    axisY = new QValueAxis();
    axisY->setTitleText("幅度");
    
    m_spectrumChart->addAxis(axisX, Qt::AlignBottom);
    m_spectrumChart->addAxis(axisY, Qt::AlignLeft);
    m_spectrumSeries->attachAxis(axisX);
    m_spectrumSeries->attachAxis(axisY);
    
    m_spectrumChartView = new QChartView(m_spectrumChart);
    m_spectrumChartView->setRenderHint(QPainter::Antialiasing);
    ui->spectrumChartLayout->addWidget(m_spectrumChartView);
    
    // 示波器图表
    m_oscilloscopeChart = new QChart();
    m_oscilloscopeChart->setTitle("示波器显示");
    m_oscilloscopeSeries.resize(2);
    
    // 通道1 (红色)
    m_oscilloscopeSeries[0] = new QLineSeries();
    m_oscilloscopeSeries[0]->setName("通道1");
    QPen pen1(Qt::red);
    pen1.setWidth(2);
    m_oscilloscopeSeries[0]->setPen(pen1);
    m_oscilloscopeChart->addSeries(m_oscilloscopeSeries[0]);
    
    // 通道2 (蓝色)
    m_oscilloscopeSeries[1] = new QLineSeries();
    m_oscilloscopeSeries[1]->setName("通道2");
    QPen pen2(Qt::blue);
    pen2.setWidth(2);
    m_oscilloscopeSeries[1]->setPen(pen2);
    m_oscilloscopeChart->addSeries(m_oscilloscopeSeries[1]);
    
    axisX = new QValueAxis();
    axisX->setTitleText("时间(秒)");
    axisY = new QValueAxis();
    axisY->setTitleText("幅度");
    
    m_oscilloscopeChart->addAxis(axisX, Qt::AlignBottom);
    m_oscilloscopeChart->addAxis(axisY, Qt::AlignLeft);
    m_oscilloscopeSeries[0]->attachAxis(axisX);
    m_oscilloscopeSeries[0]->attachAxis(axisY);
    m_oscilloscopeSeries[1]->attachAxis(axisX);
    m_oscilloscopeSeries[1]->attachAxis(axisY);
    
    m_oscilloscopeChartView = new QChartView(m_oscilloscopeChart);
    m_oscilloscopeChartView->setRenderHint(QPainter::Antialiasing);
    ui->oscilloscopeChartLayout->addWidget(m_oscilloscopeChartView);
}

// 连接信号和槽
void MainWindow::connectSignals()
{
    // 信号发生器到信道
    connect(m_signalGenerator, &SignalGenerator::signalGenerated, 
            m_channelModule, &ChannelModule::onSignalReceived);
    
    // 信道到接收分析器
    connect(m_channelModule, &ChannelModule::signalProcessed,
            m_receiveAnalyzer, &ReceiveAnalyzer::onSignalReceived);
    
    // 信号发生器到示波器 (通道0)
    connect(m_signalGenerator, &SignalGenerator::signalGenerated,
            [this](const QVector<double> &data) {
                m_oscilloscope->onSignalReceived(0, data);
            });
    
    // 信道到示波器 (通道1)
    connect(m_channelModule, &ChannelModule::signalProcessed,
            [this](const QVector<double> &data) {
                m_oscilloscope->onSignalReceived(1, data);
            });
    
    // 日志更新
    connect(m_signalGenerator, &SignalGenerator::logUpdated,
            this, &MainWindow::updateLogText);
    
    // UI 更新
    connect(m_signalGenerator, &SignalGenerator::signalGenerated,
            this, &MainWindow::updateGeneratorUI);
    
    connect(m_channelModule, &ChannelModule::signalProcessed,
            this, &MainWindow::updateChannelUI);
    
    connect(m_receiveAnalyzer, &ReceiveAnalyzer::filteredDataReady,
            this, &MainWindow::updateReceiverUI);
    
    connect(m_receiveAnalyzer, &ReceiveAnalyzer::spectrumDataReady,
            this, &MainWindow::updateSpectrumUI);
    
    connect(m_oscilloscope, &Oscilloscope::dataUpdated,
            this, &MainWindow::updateOscilloscopeUI);
    
    // 添加这些连接，确保示波器启动后能够获取现有数据
    connect(ui->startOscilloscopeButton, &QPushButton::clicked, [this]() {
        // 如果信号发生器已经生成了数据，立即发送给示波器
        if (m_signalGenerator->isGenerating()) {
            m_oscilloscope->onSignalReceived(0, m_signalGenerator->getGeneratedData());
            
            // 如果信道也有数据，发送给示波器通道1
            QVector<double> channelData = m_channelModule->processSignal(m_signalGenerator->getGeneratedData());
            m_oscilloscope->onSignalReceived(1, channelData);
        }
    });
}

// UI 更新函数
void MainWindow::updateGeneratorUI(const QVector<double> &data)
{
    if (data.isEmpty()) return;
    
    m_generatorSeries->clear();
    
    double timeStep = 1.0 / static_cast<double>(
        ui->samplingRateComboBox->currentIndex() == 0 ? RATE_1KHZ :
        ui->samplingRateComboBox->currentIndex() == 1 ? RATE_2KHZ :
        ui->samplingRateComboBox->currentIndex() == 2 ? RATE_4KHZ : RATE_8KHZ);
    
    // 查找最大值和最小值
    double minValue = data[0];
    double maxValue = data[0];
    
    for (int i = 0; i < qMin(1000, data.size()); ++i) {
        m_generatorSeries->append(i * timeStep, data[i]);
        
        if (data[i] < minValue) minValue = data[i];
        if (data[i] > maxValue) maxValue = data[i];
    }
    
    // 添加一些边距
    double margin = (maxValue - minValue) * 0.1;
    if (margin < 1.0) margin = 1.0;  // 确保至少有一些边距
    
    m_generatorChart->axes(Qt::Horizontal).first()->setRange(0, qMin(1000, data.size()) * timeStep);
    // 设置Y轴范围为数据的最小值和最大值，加上一些边距
    m_generatorChart->axes(Qt::Vertical).first()->setRange(minValue - margin, maxValue + margin);
}

void MainWindow::updateChannelUI(const QVector<double> &data)
{
    if (data.isEmpty()) return;
    
    m_channelSeries->clear();
    
    double timeStep = 1.0 / static_cast<double>(
        ui->samplingRateComboBox->currentIndex() == 0 ? RATE_1KHZ :
        ui->samplingRateComboBox->currentIndex() == 1 ? RATE_2KHZ :
        ui->samplingRateComboBox->currentIndex() == 2 ? RATE_4KHZ : RATE_8KHZ);
    
    // 查找最大值和最小值
    double minValue = data[0];
    double maxValue = data[0];
    
    for (int i = 0; i < qMin(1000, data.size()); ++i) {
        m_channelSeries->append(i * timeStep, data[i]);
        
        if (data[i] < minValue) minValue = data[i];
        if (data[i] > maxValue) maxValue = data[i];
    }
    
    // 添加一些边距
    double margin = (maxValue - minValue) * 0.1;
    if (margin < 1.0) margin = 1.0;  // 确保至少有一些边距
    
    m_channelChart->axes(Qt::Horizontal).first()->setRange(0, qMin(1000, data.size()) * timeStep);
    // 设置Y轴范围为数据的最小值和最大值，加上一些边距
    m_channelChart->axes(Qt::Vertical).first()->setRange(minValue - margin, maxValue + margin);
}

void MainWindow::updateReceiverUI(const QVector<double> &filteredData)
{
    if (filteredData.isEmpty()) return;
    
    m_receiverSeries->clear();
    
    double timeStep = 1.0 / static_cast<double>(
        ui->samplingRateComboBox->currentIndex() == 0 ? RATE_1KHZ :
        ui->samplingRateComboBox->currentIndex() == 1 ? RATE_2KHZ :
        ui->samplingRateComboBox->currentIndex() == 2 ? RATE_4KHZ : RATE_8KHZ);
    
    // 查找最大值和最小值
    double minValue = filteredData[0];
    double maxValue = filteredData[0];
    
    for (int i = 0; i < qMin(1000, filteredData.size()); ++i) {
        m_receiverSeries->append(i * timeStep, filteredData[i]);
        
        if (filteredData[i] < minValue) minValue = filteredData[i];
        if (filteredData[i] > maxValue) maxValue = filteredData[i];
    }
    
    // 添加一些边距
    double margin = (maxValue - minValue) * 0.1;
    if (margin < 1.0) margin = 1.0;  // 确保至少有一些边距
    
    m_receiverChart->axes(Qt::Horizontal).first()->setRange(0, qMin(1000, filteredData.size()) * timeStep);
    // 设置Y轴范围为数据的最小值和最大值，加上一些边距
    m_receiverChart->axes(Qt::Vertical).first()->setRange(minValue - margin, maxValue + margin);
}

void MainWindow::updateSpectrumUI(const QVector<double> &spectrumData, const QVector<double> &freqAxis)
{
    if (spectrumData.isEmpty() || freqAxis.isEmpty()) return;
    
    m_spectrumSeries->clear();
    
    double maxMagnitude = 0;
    
    for (int i = 0; i < qMin(spectrumData.size(), freqAxis.size()); ++i) {
        m_spectrumSeries->append(freqAxis[i], spectrumData[i]);
        if (spectrumData[i] > maxMagnitude) {
            maxMagnitude = spectrumData[i];
        }
    }
    
    m_spectrumChart->axes(Qt::Horizontal).first()->setRange(0, freqAxis.last());
    m_spectrumChart->axes(Qt::Vertical).first()->setRange(0, maxMagnitude * 1.1);
}

void MainWindow::updateOscilloscopeUI(int channel, const QVector<double> &data)
{
    if (data.isEmpty() || channel < 0 || channel >= m_oscilloscopeSeries.size()) return;
    
    m_oscilloscopeSeries[channel]->clear();
    
    QVector<double> timeAxis = m_oscilloscope->getTimeAxis();
    
    int numPoints = qMin(data.size(), timeAxis.size());
    
    // 仅当channel为0时更新Y轴范围
    if (channel == 0) {
        // 查找所有通道数据的最大值和最小值
        double minValue = std::numeric_limits<double>::max();
        double maxValue = std::numeric_limits<double>::lowest();
        
        // 检查当前通道的数据
        for (int i = 0; i < numPoints; ++i) {
            if (data[i] < minValue) minValue = data[i];
            if (data[i] > maxValue) maxValue = data[i];
        }
        
        // 如果通道1的数据也可用，也检查它
        if (channel == 0 && !m_oscilloscopeSeries[1]->points().isEmpty()) {
            const auto& points = m_oscilloscopeSeries[1]->points();
            for (const auto& point : points) {
                if (point.y() < minValue) minValue = point.y();
                if (point.y() > maxValue) maxValue = point.y();
            }
        }
        
        // 添加一些边距
        double margin = (maxValue - minValue) * 0.1;
        if (margin < 1.0) margin = 1.0;  // 确保至少有一些边距
        
        m_oscilloscopeChart->axes(Qt::Horizontal).first()->setRange(0, timeAxis.last());
        // 设置Y轴范围为数据的最小值和最大值，加上一些边距
        m_oscilloscopeChart->axes(Qt::Vertical).first()->setRange(minValue - margin, maxValue + margin);
    }
    
    // 填充数据点
    for (int i = 0; i < numPoints; ++i) {
        m_oscilloscopeSeries[channel]->append(timeAxis[i], data[i]);
    }
}

void MainWindow::updateLogText(const QString &log)
{
    ui->logTextEdit->setPlainText(log);
    ui->logTextEdit->verticalScrollBar()->setValue(ui->logTextEdit->verticalScrollBar()->maximum());
}
