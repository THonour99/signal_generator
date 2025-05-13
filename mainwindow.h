#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QFileDialog>
#include <QLabel>
#include <QGroupBox>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtMath>
#include <QMessageBox>
#include <QScrollBar>

#include "signalgenerator.h"
#include "channelmodule.h"
#include "receiveanalyzer.h"
#include "oscilloscope.h"

QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 信号发生器控制
    void on_signalTypeComboBox_currentIndexChanged(int index);
    void on_frequencySpinBox_valueChanged(double value);
    void on_amplitudeSpinBox_valueChanged(double value);
    void on_dcOffsetSpinBox_valueChanged(double value);
    void on_samplingRateComboBox_currentIndexChanged(int index);
    void on_loadFileButton_clicked();
    void on_startGeneratorButton_clicked();
    void on_stopGeneratorButton_clicked();
    
    // 信道控制
    void on_noiseAmplitudeSlider_valueChanged(int value);
    
    // 接收分析控制
    void on_filterCutoffSpinBox_valueChanged(double value);
    void on_saveDataButton_clicked();
    
    // 示波器控制
    void on_timePerDivSpinBox_valueChanged(double value);
    void on_voltPerDivSpinBox_valueChanged(double value);
    void on_triggerLevelSpinBox_valueChanged(double value);
    void on_triggerModeComboBox_currentIndexChanged(int index);
    void on_startOscilloscopeButton_clicked();
    void on_stopOscilloscopeButton_clicked();
    
    // 数据更新槽
    void updateGeneratorUI(const QVector<double> &data);
    void updateChannelUI(const QVector<double> &data);
    void updateReceiverUI(const QVector<double> &filteredData);
    void updateSpectrumUI(const QVector<double> &spectrumData, const QVector<double> &freqAxis);
    void updateOscilloscopeUI(int channel, const QVector<double> &data);
    void updateLogText(const QString &log);

private:
    Ui::MainWindow *ui;
    
    // 模块实例
    SignalGenerator *m_signalGenerator;
    ChannelModule *m_channelModule;
    ReceiveAnalyzer *m_receiveAnalyzer;
    Oscilloscope *m_oscilloscope;
    
    // 图表
    QChart *m_generatorChart;
    QChartView *m_generatorChartView;
    QLineSeries *m_generatorSeries;
    
    QChart *m_channelChart;
    QChartView *m_channelChartView;
    QLineSeries *m_channelSeries;
    
    QChart *m_receiverChart;
    QChartView *m_receiverChartView;
    QLineSeries *m_receiverSeries;
    
    QChart *m_spectrumChart;
    QChartView *m_spectrumChartView;
    QLineSeries *m_spectrumSeries;
    
    QChart *m_oscilloscopeChart;
    QChartView *m_oscilloscopeChartView;
    QVector<QLineSeries*> m_oscilloscopeSeries;
    
    // 初始化函数
    void setupSignalGenerator();
    void setupChannelModule();
    void setupReceiveAnalyzer();
    void setupOscilloscope();
    void setupCharts();
    void connectSignals();
};

#endif // MAINWINDOW_H
