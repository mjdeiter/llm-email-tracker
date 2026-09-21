// ============================================================================
// Email Status Tracker (LLM Email Tracker)
// Version: 1.2.0
// Date:    2026-09-21
//
// Changelog:
//   1.2.0  Live save: every edit is persisted automatically (debounced), and
//          any pending change is flushed when the window closes or the app
//          quits, so the app reopens exactly as it was left.
//          - Atomic writes via QSaveFile (no truncated JSON on crash)
//          - Unparseable data file is moved aside instead of overwritten
//          - Email addresses are trimmed on load and save
//   1.1.1  Reset All saves immediately
//   1.1.0  Per-row Copy button
//   1.0.0  Initial release
// ============================================================================

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QList>
#include <QStringList>
#include <QIcon>
#include <QClipboard>
#include <QGuiApplication>
#include <QCloseEvent>
#include <QTimer>

class EmailRow : public QWidget {
    Q_OBJECT
public:
    QLineEdit* emailEdit;
    QRadioButton* usedButton;
    QLabel* timestampLabel;
    QLineEdit* manualDateTimeEdit;
    QPushButton* copyButton;
    QString rawTimestamp;

    EmailRow(QWidget* parent = nullptr) : QWidget(parent) {
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 4, 0, 4);
        layout->setSpacing(10);

        emailEdit = new QLineEdit(this);
        emailEdit->setPlaceholderText("Enter email address...");
        emailEdit->setFixedWidth(300);

        usedButton = new QRadioButton("Used", this);
        usedButton->setAutoExclusive(false);
        usedButton->setFixedWidth(80);

        timestampLabel = new QLabel(this);
        timestampLabel->setFixedWidth(200);
        timestampLabel->setFont(QFont("monospace", 10));

        manualDateTimeEdit = new QLineEdit(this);
        manualDateTimeEdit->setFixedWidth(180);
        manualDateTimeEdit->setStyleSheet(
            "QLineEdit {"
            "  color: #9c36b5;"
            "  font-family: monospace;"
            "  border: 1px solid #ced4da;"
            "  border-radius: 4px;"
            "  padding: 2px;"
            "}"
            "QLineEdit:focus {"
            "  border: 1px solid #9c36b5;"
            "}"
        );

        copyButton = new QPushButton("Copy", this);
        copyButton->setFixedWidth(60);

        layout->addWidget(emailEdit);
        layout->addWidget(usedButton);
        layout->addWidget(timestampLabel);
        layout->addWidget(manualDateTimeEdit);
        layout->addWidget(copyButton);
        layout->addStretch();

        // Order matters only for clarity: handleToggled sets the timestamp,
        // then changed() schedules the (debounced) save.
        connect(usedButton, &QRadioButton::toggled, this, &EmailRow::handleToggled);
        connect(copyButton, &QPushButton::clicked, this, &EmailRow::copyEmailToClipboard);

        // Anything the user can edit on this row counts as a change (live save)
        connect(emailEdit, &QLineEdit::textChanged, this, &EmailRow::changed);
        connect(manualDateTimeEdit, &QLineEdit::textChanged, this, &EmailRow::changed);
        connect(usedButton, &QRadioButton::toggled, this, &EmailRow::changed);
    }

    void displayTimestamp() {
        if (rawTimestamp.isEmpty()) {
            timestampLabel->clear();
            return;
        }

        QStringList parts = rawTimestamp.split(" ");
        if (parts.size() == 2) {
            QString htmlText = QString("<span style='color: #d9480f; font-weight: bold;'>%1</span> "
            "<span style='color: #2b8a3e;'>%2</span>").arg(parts[0], parts[1]);
            timestampLabel->setText(htmlText);
        } else {
            timestampLabel->setText(rawTimestamp);
        }
    }

    // Resets Status radio, Auto Timestamp, and Resets field for this row
    void resetRow() {
        usedButton->blockSignals(true);
        usedButton->setChecked(false);
        usedButton->blockSignals(false);

        rawTimestamp.clear();
        displayTimestamp();

        manualDateTimeEdit->clear();
    }

signals:
    void changed();

private slots:
    void handleToggled(bool checked) {
        if (checked) {
            QString dateStr = QDateTime::currentDateTime().toString("MM/dd/yyyy");
            QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss");
            rawTimestamp = dateStr + " " + timeStr;
        } else {
            rawTimestamp.clear();
        }
        displayTimestamp();
    }

    void copyEmailToClipboard() {
        QGuiApplication::clipboard()->setText(emailEdit->text());
        copyButton->setText("Copied!");
        QTimer::singleShot(1000, this, [this]() {
            copyButton->setText("Copy");
        });
    }
};

class MainWidget : public QWidget {
    Q_OBJECT
private:
    QVBoxLayout* rowsLayout;
    QWidget* scrollContainer;
    QList<EmailRow*> rows;
    QString savePath;
    QTimer* saveTimer;      // debounce timer for live save
    bool loading = false;   // true while populating rows from disk

public:
    MainWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setWindowTitle("Email Status Tracker");
        resize(850, 500);

        savePath = QDir::homePath() + "/.config/email_tracker_data.json";

        // Live save: edits restart this timer; when it fires, data is written.
        saveTimer = new QTimer(this);
        saveTimer->setSingleShot(true);
        saveTimer->setInterval(400);
        connect(saveTimer, &QTimer::timeout, this, &MainWidget::saveData);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QHBoxLayout* headerLayout = new QHBoxLayout();
        headerLayout->setContentsMargins(10, 5, 10, 5);
        headerLayout->setSpacing(10);

        QString headerStyle = "font-weight: bold; color: #ffffff; font-size: 12px;";

        QLabel* emailHeader = new QLabel("Email Address", this);
        emailHeader->setFixedWidth(300);
        emailHeader->setStyleSheet(headerStyle);

        QLabel* statusHeader = new QLabel("Status", this);
        statusHeader->setFixedWidth(80);
        statusHeader->setStyleSheet(headerStyle);

        QLabel* timeHeader = new QLabel("Auto Timestamp (Date / Time)", this);
        timeHeader->setFixedWidth(200);
        timeHeader->setStyleSheet(headerStyle);

        QLabel* resetsHeader = new QLabel("Resets", this);
        resetsHeader->setFixedWidth(180);
        resetsHeader->setStyleSheet(headerStyle);

        QLabel* actionsHeader = new QLabel("", this);
        actionsHeader->setFixedWidth(60);
        actionsHeader->setStyleSheet(headerStyle);

        headerLayout->addWidget(emailHeader);
        headerLayout->addWidget(statusHeader);
        headerLayout->addWidget(timeHeader);
        headerLayout->addWidget(resetsHeader);
        headerLayout->addWidget(actionsHeader);
        headerLayout->addStretch();

        mainLayout->addLayout(headerLayout);

        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);

        scrollContainer = new QWidget();
        rowsLayout = new QVBoxLayout(scrollContainer);
        rowsLayout->setAlignment(Qt::AlignTop);
        rowsLayout->setContentsMargins(10, 0, 10, 0);
        scrollContainer->setLayout(rowsLayout);
        scrollArea->setWidget(scrollContainer);

        mainLayout->addWidget(scrollArea);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* addButton = new QPushButton("Add Row", this);
        QPushButton* saveButton = new QPushButton("Save Config", this);
        saveButton->setStyleSheet("font-weight: bold;");

        QPushButton* resetAllButton = new QPushButton("Reset All", this);
        resetAllButton->setStyleSheet(
            "QPushButton {"
            "  color: #c92a2a;"
            "  font-weight: bold;"
            "  border: 1px solid #c92a2a;"
            "  border-radius: 4px;"
            "  padding: 4px 12px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #c92a2a;"
            "  color: #ffffff;"
            "}"
            "QPushButton:pressed {"
            "  background-color: #a61e1e;"
            "  color: #ffffff;"
            "}"
        );

        buttonLayout->addWidget(addButton);
        buttonLayout->addWidget(saveButton);
        buttonLayout->addStretch();
        buttonLayout->addWidget(resetAllButton);
        mainLayout->addLayout(buttonLayout);

        connect(addButton, &QPushButton::clicked, this, &MainWidget::addNewRow);
        connect(saveButton, &QPushButton::clicked, this, &MainWidget::saveData);  // manual save still works
        connect(resetAllButton, &QPushButton::clicked, this, &MainWidget::resetAll);

        loadData();
    }

    // Creates a row, wires it into live save, and appends it to the list
    EmailRow* createRow() {
        EmailRow* row = new EmailRow(scrollContainer);
        connect(row, &EmailRow::changed, this, &MainWidget::scheduleSave);
        rowsLayout->addWidget(row);
        rows.append(row);
        return row;
    }

    void addNewRow() {
        createRow();  // a blank row has nothing to persist until it is edited
    }

    // Restart the debounce timer; the actual write happens once edits pause
    void scheduleSave() {
        if (!loading) {
            saveTimer->start();
        }
    }

    bool saveData() {
        saveTimer->stop();  // any pending debounced save is superseded by this one

        QJsonArray jsonArray;
        for (EmailRow* row : rows) {
            const QString email = row->emailEdit->text().trimmed();
            if (email.isEmpty() &&
                !row->usedButton->isChecked() &&
                row->manualDateTimeEdit->text().isEmpty()) {
                continue;
            }
            QJsonObject rowObject;
            rowObject["email"] = email;
            rowObject["used"] = row->usedButton->isChecked();
            rowObject["timestamp"] = row->rawTimestamp;
            rowObject["resets"] = row->manualDateTimeEdit->text();
            jsonArray.append(rowObject);
        }

        QDir().mkpath(QFileInfo(savePath).absolutePath());

        // QSaveFile writes to a temp file and renames on commit, so a crash or
        // power loss mid-write can never leave a half-written data file behind.
        QSaveFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning("EmailTracker: cannot open %s for writing: %s",
                     qPrintable(savePath), qPrintable(file.errorString()));
            return false;
        }
        file.write(QJsonDocument(jsonArray).toJson());
        if (!file.commit()) {
            qWarning("EmailTracker: failed to save %s: %s",
                     qPrintable(savePath), qPrintable(file.errorString()));
            return false;
        }
        return true;
    }

    void loadData() {
        loading = true;   // programmatic setText/setChecked below must not trigger saves
        readRowsFromDisk();
        if (rows.isEmpty()) {
            createRow();
        }
        loading = false;
    }

public slots:
    void resetAll() {
        for (EmailRow* row : rows) {
            row->resetRow();
        }
        saveData();  // persist immediately so the reset survives closing the app
    }

    // Write any not-yet-saved edits. Called on window close and app quit.
    void flushSave() {
        if (saveTimer->isActive()) {
            saveData();
        }
    }

protected:
    void closeEvent(QCloseEvent* event) override {
        flushSave();
        QWidget::closeEvent(event);
    }

private:
    void readRowsFromDisk() {
        QFile file(savePath);
        if (!file.open(QIODevice::ReadOnly)) {
            return;  // first run: no data file yet
        }

        QByteArray data = file.readAll();
        file.close();
        if (data.trimmed().isEmpty()) {
            return;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
            // Don't let live save silently overwrite a file we couldn't read:
            // move it aside so the original data can still be recovered.
            const QString backup = savePath + ".corrupt-" +
                QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
            QFile::rename(savePath, backup);
            qWarning("EmailTracker: %s is not valid tracker data; moved to %s",
                     qPrintable(savePath), qPrintable(backup));
            return;
        }

        QJsonArray jsonArray = doc.array();
        for (int i = 0; i < jsonArray.size(); ++i) {
            QJsonObject rowObject = jsonArray[i].toObject();
            EmailRow* row = createRow();

            row->emailEdit->setText(rowObject["email"].toString().trimmed());

            row->usedButton->blockSignals(true);
            row->usedButton->setChecked(rowObject["used"].toBool());
            row->usedButton->blockSignals(false);

            row->rawTimestamp = rowObject["timestamp"].toString();
            row->displayTimestamp();

            row->manualDateTimeEdit->setText(rowObject["resets"].toString());
        }
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWidget window;

    // Cmd+Q / Dock "Quit" / logout quit the app without necessarily closing the
    // window first, so flush any pending edit here as well as in closeEvent.
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &window, &MainWidget::flushSave);

    // --- Embedded Custom Colored Icon ---
    QString iconPath = QDir::homePath() + "/.config/email_tracker_icon.svg";
    if (!QFile::exists(iconPath)) {
        QFile iconFile(iconPath);
        if (iconFile.open(QIODevice::WriteOnly)) {
            // Standard escaped string literal to avoid parsing errors
            QString svgContent =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 64 64\">\n"
            "  <rect x=\"4\" y=\"4\" width=\"56\" height=\"56\" rx=\"12\" fill=\"#1e1e2f\"/>\n"
            "  <rect x=\"4\" y=\"4\" width=\"56\" height=\"56\" rx=\"12\" fill=\"url(#grad)\" opacity=\"0.9\"/>\n"
            "  <defs>\n"
            "    <linearGradient id=\"grad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\">\n"
            "      <stop offset=\"0%\" stop-color=\"#3b82f6\" />\n"
            "      <stop offset=\"100%\" stop-color=\"#8b5cf6\" />\n"
            "    </linearGradient>\n"
            "  </defs>\n"
            "  <rect x=\"24\" y=\"18\" width=\"28\" height=\"4\" rx=\"2\" fill=\"#ffffff\" opacity=\"0.9\"/>\n"
            "  <rect x=\"24\" y=\"30\" width=\"24\" height=\"4\" rx=\"2\" fill=\"#ffffff\" opacity=\"0.9\"/>\n"
            "  <rect x=\"24\" y=\"42\" width=\"20\" height=\"4\" rx=\"2\" fill=\"#ffffff\" opacity=\"0.9\"/>\n"
            "  <path d=\"M12 18 l4 4 l8 -8\" fill=\"none\" stroke=\"#22c55e\" stroke-width=\"4\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n"
            "  <path d=\"M12 30 l4 4 l8 -8\" fill=\"none\" stroke=\"#22c55e\" stroke-width=\"4\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n"
            "  <circle cx=\"16\" cy=\"44\" r=\"4\" fill=\"none\" stroke=\"#94a3b8\" stroke-width=\"3\"/>\n"
            "</svg>";
            iconFile.write(svgContent.toUtf8());
            iconFile.close();
        }
    }

    window.setWindowIcon(QIcon(iconPath));
    // ------------------------------------

    window.show();
    return app.exec();
}

#include "main.moc"
