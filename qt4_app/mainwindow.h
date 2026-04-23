#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>

class QLabel;
class QPushButton;
class QProcess;
class QTabWidget;
class QTextEdit;
class QVBoxLayout;
class QWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void runPythonGenerator();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleProcessError(QProcess::ProcessError error);
    void readProcessOutput();

private:
    struct PlotEntry
    {
        QString imageFileName;
        QLabel *imageLabel;
    };

    QString generatedDirPath() const;
    QString projectRootPath() const;
    QString scriptsPath() const;
    QString detectPythonCommand() const;
    QWidget *createOverviewTab();
    QWidget *createPlotsTab();
    QWidget *createSinglePlotCard(const QString &title,
                                  const QString &description,
                                  const QString &imageFileName);
    QLabel *createImageLabel(const QString &imageFileName, bool registerLabel);
    QString readSummaryText() const;
    void refreshGeneratedContent();
    void appendProcessText(const QString &text);

    QTabWidget *m_tabs;
    QTextEdit *m_summaryText;
    QTextEdit *m_processOutput;
    QLabel *m_statusLabel;
    QPushButton *m_runButton;
    QProcess *m_process;
    QList<PlotEntry> m_plotEntries;
};

#endif
