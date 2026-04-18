#include "reportdialog.h"

ReportDialog::ReportDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Add Report Entry");
    setMinimumWidth(420);

    titleEdit       = new QLineEdit(this);
    descEdit        = new QTextEdit(this);
    severityBox     = new QComboBox(this);
    validationLabel = new QLabel(this);
    submitBtn       = new QPushButton("Submit", this);
    cancelBtn       = new QPushButton("Cancel", this);

    titleEdit->setPlaceholderText("Event title (3–80 characters)");
    descEdit->setPlaceholderText("Describe the event (min 10 characters)...");
    descEdit->setFixedHeight(100);

    severityBox->addItem("ℹ️  Info");
    severityBox->addItem("⚠️  Warning");
    severityBox->addItem("🔴  Error");

    validationLabel->setStyleSheet("color: #FF4D6A;");
    validationLabel->setWordWrap(true);
    validationLabel->clear();

    // Layout
    QFormLayout *form = new QFormLayout();
    form->addRow("Title:",       titleEdit);
    form->addRow("Severity:",    severityBox);
    form->addRow("Description:", descEdit);
    form->addRow("",             validationLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(submitBtn);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addLayout(btnLayout);
    setLayout(main);

    // Connections
    connect(titleEdit,  &QLineEdit::textChanged, this, &ReportDialog::liveValidate);
    connect(descEdit,   &QTextEdit::textChanged, this, &ReportDialog::liveValidate);
    connect(submitBtn,  &QPushButton::clicked,   this, &ReportDialog::validateAndAccept);
    connect(cancelBtn,  &QPushButton::clicked,   this, &ReportDialog::reject);
}

QString ReportDialog::title()       const { return titleEdit->text().trimmed(); }
QString ReportDialog::description() const { return descEdit->toPlainText().trimmed(); }
QString ReportDialog::severity()    const { return severityBox->currentText(); }

void ReportDialog::liveValidate()
{
    validate();  // updates label live, but doesn't block
}

bool ReportDialog::validate()
{
    QString t = titleEdit->text().trimmed();
    QString d = descEdit->toPlainText().trimmed();

    if (t.isEmpty()) {
        validationLabel->setText("⚠ Title cannot be empty.");
        return false;
    }
    if (t.length() < 3) {
        validationLabel->setText("⚠ Title too short (min 3 characters).");
        return false;
    }
    if (t.length() > 80) {
        validationLabel->setText("⚠ Title too long (max 80 characters).");
        return false;
    }
    if (d.isEmpty() || d.length() < 10) {
        validationLabel->setText("⚠ Description too short (min 10 characters).");
        return false;
    }

    validationLabel->clear();
    return true;
}

void ReportDialog::validateAndAccept()
{
    if (validate())
        accept();  // only closes dialog if valid
}