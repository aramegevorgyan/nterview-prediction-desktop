#include "mainwindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QPixmap>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextCursor>
#include <QVBoxLayout>

MainWindow::MainWindow()
    : m_tabs(0),
      m_summaryText(0),
      m_processOutput(0),
      m_statusLabel(0),
      m_runButton(0),
      m_process(0)
{
    setWindowTitle("Interview Prediction Project Viewer");
    resize(1100, 760);

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(createOverviewTab(), "Overview");
    m_tabs->addTab(createPlotsTab(), "Plots");
    setCentralWidget(m_tabs);

    refreshGeneratedContent();
}

QString MainWindow::generatedDirPath() const
{
    QDir appDir(QCoreApplication::applicationDirPath());
    if (appDir.dirName() == "debug" || appDir.dirName() == "release") {
        appDir.cdUp();
    }
    return appDir.absoluteFilePath("generated");
}

QString MainWindow::projectRootPath() const
{
    QDir appDir(QCoreApplication::applicationDirPath());
    if (appDir.dirName() == "debug" || appDir.dirName() == "release") {
        appDir.cdUp();
    }
    appDir.cdUp();
    return appDir.absolutePath();
}

QString MainWindow::scriptsPath() const
{
    return QDir(projectRootPath()).absoluteFilePath("scripts/generate_plots.py");
}

QString MainWindow::detectPythonCommand() const
{
    return "python";
}

QWidget *MainWindow::createOverviewTab()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    QLabel *title = new QLabel(
        "<h2>What this project is</h2>"
        "<p>This repository trains a machine learning model on <b>logatta.csv</b> "
        "to estimate whether a person will be accepted for an interview.</p>"
        "<p>The notebook cleans the data, converts text categories to numbers, "
        "trains a Random Forest model, and then visualizes the results.</p>"
        "<p><b>Important:</b> the current codebase is not a heart disease dataset yet. "
        "Your healthcare description is a valid example of how a binary prediction model works, "
        "but this repo is still using interview data.</p>");
    title->setWordWrap(true);

    QGroupBox *whyBox = new QGroupBox("Why a Qt window helps");
    QVBoxLayout *whyLayout = new QVBoxLayout(whyBox);
    QLabel *whyText = new QLabel(
        "A Qt desktop window gives the project a simple interface for classmates, teachers, "
        "or demo users who do not want to open Jupyter. It is useful for presenting the model "
        "summary, showing the charts in one place, and making the project feel like a small product "
        "instead of only a notebook.");
    whyText->setWordWrap(true);
    whyLayout->addWidget(whyText);

    QGroupBox *issuesBox = new QGroupBox("What issues it solves");
    QVBoxLayout *issuesLayout = new QVBoxLayout(issuesBox);
    QLabel *issuesText = new QLabel(
        "1. Easier demos: no need to walk through notebook cells during a presentation.\n"
        "2. Better readability: plots and summary text are grouped into tabs.\n"
        "3. Clearer communication: the app explains the project purpose and warns about the current dataset.\n"
        "4. Reusability: if you later switch to a heart disease CSV, the same UI can display that model's results.");
    issuesText->setWordWrap(true);
    issuesLayout->addWidget(issuesText);

    QGroupBox *summaryBox = new QGroupBox("Generated model summary");
    QVBoxLayout *summaryLayout = new QVBoxLayout(summaryBox);
    m_summaryText = new QTextEdit;
    m_summaryText->setReadOnly(true);
    m_summaryText->setPlainText(readSummaryText());
    summaryLayout->addWidget(m_summaryText);

    QGroupBox *runBox = new QGroupBox("Python integration");
    QVBoxLayout *runLayout = new QVBoxLayout(runBox);
    QLabel *runText = new QLabel(
        "This app can run the Python analysis script itself, collect the console output, "
        "and reload the generated summary and plots without leaving Qt.");
    runText->setWordWrap(true);
    m_statusLabel = new QLabel("Status: ready");
    m_statusLabel->setWordWrap(true);
    m_runButton = new QPushButton("Run Python analysis");
    m_processOutput = new QTextEdit;
    m_processOutput->setReadOnly(true);
    m_processOutput->setMinimumHeight(170);
    runLayout->addWidget(runText);
    runLayout->addWidget(m_statusLabel);
    runLayout->addWidget(m_runButton);
    runLayout->addWidget(m_processOutput);

    layout->addWidget(title);
    layout->addWidget(whyBox);
    layout->addWidget(issuesBox);
    layout->addWidget(summaryBox, 1);
    layout->addWidget(runBox);

    connect(m_runButton, SIGNAL(clicked()), this, SLOT(runPythonGenerator()));

    return page;
}

QWidget *MainWindow::createPlotsTab()
{
    QWidget *container = new QWidget;
    QVBoxLayout *stack = new QVBoxLayout(container);

    stack->addWidget(createSinglePlotCard(
        "Feature Importance",
        "Shows which columns influence the model most strongly.",
        "feature_importance.png"));
    stack->addWidget(createSinglePlotCard(
        "Acceptance Distribution",
        "Shows how many rows are accepted versus rejected.",
        "acceptance_distribution.png"));
    stack->addWidget(createSinglePlotCard(
        "Age Distribution",
        "Compares ages for accepted and rejected groups.",
        "age_distribution.png"));
    stack->addWidget(createSinglePlotCard(
        "Sine Wave Example",
        "This comes from the notebook's generic plotting example.",
        "sine_wave.png"));
    stack->addWidget(createSinglePlotCard(
        "Scatter Plot Example",
        "This is the notebook's sample scatter visualization.",
        "scatter_plot.png"));
    stack->addStretch(1);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(container);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *page = new QWidget;
    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(scrollArea);
    return page;
}

QWidget *MainWindow::createSinglePlotCard(const QString &title,
                                          const QString &description,
                                          const QString &imageFileName)
{
    QGroupBox *box = new QGroupBox(title);
    QVBoxLayout *layout = new QVBoxLayout(box);

    QLabel *text = new QLabel(description);
    text->setWordWrap(true);

    layout->addWidget(text);
    layout->addWidget(createImageLabel(imageFileName, true));
    return box;
}

QLabel *MainWindow::createImageLabel(const QString &imageFileName, bool registerLabel)
{
    QLabel *label = new QLabel;
    label->setAlignment(Qt::AlignCenter);
    label->setMinimumHeight(280);
    label->setFrameShape(QFrame::StyledPanel);

    if (registerLabel) {
        PlotEntry entry;
        entry.imageFileName = imageFileName;
        entry.imageLabel = label;
        m_plotEntries.append(entry);
    }

    const QString fullPath = QDir(generatedDirPath()).absoluteFilePath(imageFileName);
    QPixmap pixmap(fullPath);
    if (!pixmap.isNull()) {
        label->setPixmap(pixmap.scaled(980, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        label->setText("Missing image: " + QFileInfo(fullPath).fileName() +
                       "\nUse the button above to run the Python analysis.");
    }
    return label;
}

QString MainWindow::readSummaryText() const
{
    const QString path = QDir(generatedDirPath()).absoluteFilePath("summary.txt");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "No generated summary found.\n\nUse the Python integration panel to generate it.";
    }

    return QString::fromUtf8(file.readAll());
}

void MainWindow::refreshGeneratedContent()
{
    if (m_summaryText) {
        m_summaryText->setPlainText(readSummaryText());
    }

    for (int index = 0; index < m_plotEntries.size(); ++index) {
        const QString fullPath = QDir(generatedDirPath()).absoluteFilePath(
            m_plotEntries.at(index).imageFileName);
        QPixmap pixmap(fullPath);
        QLabel *label = m_plotEntries.at(index).imageLabel;
        label->clear();
        if (!pixmap.isNull()) {
            label->setPixmap(
                pixmap.scaled(980, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            label->setText("Missing image: " +
                           QFileInfo(fullPath).fileName() +
                           "\nUse the button above to run the Python analysis.");
        }
    }
}

void MainWindow::appendProcessText(const QString &text)
{
    if (!m_processOutput || text.isEmpty()) {
        return;
    }
    m_processOutput->moveCursor(QTextCursor::End);
    m_processOutput->insertPlainText(text);
    m_processOutput->moveCursor(QTextCursor::End);
}

void MainWindow::runPythonGenerator()
{
    if (m_process) {
        return;
    }

    m_process = new QProcess(this);
    m_process->setWorkingDirectory(projectRootPath());
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readProcessOutput()));
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(handleProcessFinished(int,QProcess::ExitStatus)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(handleProcessError(QProcess::ProcessError)));

    if (m_processOutput) {
        m_processOutput->clear();
        appendProcessText("Starting Python analysis...\n");
    }
    if (m_statusLabel) {
        m_statusLabel->setText("Status: running python script");
    }
    if (m_runButton) {
        m_runButton->setEnabled(false);
    }

    QStringList arguments;
    arguments << scriptsPath();
    m_process->start(detectPythonCommand(), arguments);
}

void MainWindow::readProcessOutput()
{
    if (!m_process) {
        return;
    }

    appendProcessText(QString::fromLocal8Bit(m_process->readAllStandardOutput()));
    appendProcessText(QString::fromLocal8Bit(m_process->readAllStandardError()));
}

void MainWindow::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    readProcessOutput();

    if (m_statusLabel) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            m_statusLabel->setText("Status: python finished successfully");
        } else {
            m_statusLabel->setText(
                QString("Status: python finished with code %1").arg(exitCode));
        }
    }

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        refreshGeneratedContent();
    }

    if (m_runButton) {
        m_runButton->setEnabled(true);
    }

    m_process->deleteLater();
    m_process = 0;
}

void MainWindow::handleProcessError(QProcess::ProcessError error)
{
    QString message = "Python process error.";
    if (error == QProcess::FailedToStart) {
        message = "Python could not start. Make sure python is installed and available on PATH.";
    } else if (error == QProcess::Crashed) {
        message = "Python started but crashed before finishing.";
    }

    appendProcessText(message + "\n");
    if (m_statusLabel) {
        m_statusLabel->setText("Status: " + message);
    }
    if (m_runButton) {
        m_runButton->setEnabled(true);
    }

    if (m_process) {
        m_process->deleteLater();
        m_process = 0;
    }
}
