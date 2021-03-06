#include "wysiwyg/visupropertieshelper.h"
#include "visumisc.h"

#include <QFontDatabase>
#include <QSerialPortInfo>

const char* VisuPropertiesHelper::PROP_COLOR = "color";
const char* VisuPropertiesHelper::PROP_ROW = "row";
const char* VisuPropertiesHelper::PROP_KEY = "key";
const char* VisuPropertiesHelper::PROP_TYPE = "type";

double VisuPropertiesHelper::sliderToDouble(int slider)
{
    return (double)slider / SLIDER_FACTOR;
}

int VisuPropertiesHelper::doubleToSlider(double value)
{
    return (int)(value * SLIDER_FACTOR);
}

void VisuPropertiesHelper::setupTableWidget(
                                QTableWidget* table,
                                std::pair<QWidget*, const char*> widget,
                                std::pair<QWidget*, const char*> parent,
                                QString key,
                                VisuPropertyMeta::Type type,
                                int row)
{
    widget.first->setProperty(VisuPropertiesHelper::PROP_KEY, key);
    widget.first->setProperty(VisuPropertiesHelper::PROP_TYPE, type);
    widget.first->setProperty(VisuPropertiesHelper::PROP_ROW, row);

    table->setCellWidget(row, VisuPropertiesHelper::COLUMN_VALUE, widget.first);

    if (widget.first != nullptr &&
        widget.second != nullptr &&
        parent.first != nullptr &&
        parent.second != nullptr)
    {
        QObject::connect(widget.first, widget.second, parent.first, parent.second);
    }
}

std::pair<QComboBox*, const char*> VisuPropertiesHelper::setupEnumWidget(VisuPropertyMeta meta, QString value)
{
    QComboBox* box = new QComboBox();
    box->insertItems(0, meta.getEnumOptions());
    box->setCurrentIndex(value.toInt());
    return std::make_pair(box, SIGNAL(currentIndexChanged(int)));
}

std::pair<QComboBox*, const char*> VisuPropertiesHelper::setupFontWidget(VisuPropertyMeta meta, QString value)
{
    QComboBox* box = new QComboBox();
    QFontDatabase db;
    box->insertItems(0, db.families());
    box->setCurrentText(value);
    return std::make_pair(box, SIGNAL(currentIndexChanged(int)));
}

std::pair<QCheckBox*, const char*> VisuPropertiesHelper::setupBoolWidget(VisuPropertyMeta meta, QString value)
{
    QCheckBox* checkbox = new QCheckBox();
    checkbox->setChecked(value.toInt() == 1);
    return std::make_pair(checkbox, SIGNAL(stateChanged(int)));
}

std::pair<QComboBox*, const char*> VisuPropertiesHelper::setupSignalsWidget(VisuPropertyMeta meta, QString value)
{
    QComboBox* box = new QComboBox();
    int selected = 0;
    int intValue = value.toInt();
    QVector<QPointer<VisuSignal>> signalList = VisuConfiguration::get()->getSignals();
    for (VisuSignal* visuSignal : signalList)
    {
        if (visuSignal != nullptr)
        {
            box->addItem(visuSignal->getName(), QVariant(visuSignal->getId()));
            if (visuSignal->getId() == intValue)
            {
                selected = box->count()-1;
            }
        }
    }
    box->setCurrentIndex(selected);
    return std::make_pair(box, SIGNAL(currentIndexChanged(int)));
}

std::pair<QComboBox*, const char*> VisuPropertiesHelper::setupImagesWidget(VisuPropertyMeta meta, QString value)
{
    QComboBox* box = new QComboBox();
    box->addItem("Use color", QVariant(-1));

    QVector<QPointer<StaticImage> > list = VisuConfiguration::get()->getListOf<StaticImage>();
    int selected = 0;
    int intValue = value.toInt();
    for (StaticImage* image : list)
    {
        if (image != nullptr)
        {
            box->addItem(image->getName(), QVariant(image->getId()));

            if (image->getId() == intValue)
            {
                selected = box->count()-1;
            }
        }
    }
    box->setCurrentIndex(selected);
    return std::make_pair(box, SIGNAL(currentIndexChanged(int)));
}

std::pair<QComboBox*, const char*> VisuPropertiesHelper::setupSerialWidget(VisuPropertyMeta meta, QString value)
{
    QComboBox* box = new QComboBox();

    const auto portInfos = QSerialPortInfo::availablePorts();

    for (const auto& port : portInfos)
    {
        if (port.isValid() && !port.isBusy())
        {
            box->addItem(port.portName());
        }
    }
    if (value != "-")
    {
        box->setCurrentText(value);
    }
    return std::make_pair(box, SIGNAL(currentIndexChanged(int)));
}

std::pair<QComboBox*, const char*> VisuPropertiesHelper::setupSignalPlaceholderWidget(VisuPropertyMeta meta, QString value)
{
    QComboBox* box = new QComboBox();

    QRegularExpression expression = QRegularExpression(VisuConfiguration::get()->getSerialRegex());
    int captures = expression.captureCount();

    box->addItem(QObject::tr("Disabled"));

    for (int i = 0 ; i < captures ; ++i)
    {
        QString label = QObject::tr("Expression #%1").arg(i);
        box->addItem(label);
    }

    box->setCurrentIndex(value.toInt());

    return std::make_pair(box, SIGNAL(currentIndexChanged(int)));
}

std::pair<QPushButton*, const char*> VisuPropertiesHelper::setupColorWidget(VisuPropertyMeta meta, QString value)
{
    QPushButton* btn = new QPushButton(value);
    QColor color = VisuMisc::strToColor(value);
    VisuMisc::setBackgroundColor(btn, color);
    btn->setProperty(VisuPropertiesHelper::PROP_COLOR, color);
    return std::make_pair(btn, SIGNAL(clicked()));
}


std::pair<QSpinBox*, const char*> VisuPropertiesHelper::setupIntWidget(VisuPropertyMeta meta, QString value)
{
    QSpinBox* spinbox = new QSpinBox();
    spinbox->setMinimum(meta.min);
    spinbox->setMaximum(meta.max);
    spinbox->setValue(value.toInt());
    return std::make_pair(spinbox, SIGNAL(valueChanged(int)));
}

std::pair<QSlider*, const char*> VisuPropertiesHelper::setupSliderWidget(VisuPropertyMeta meta, QString value)
{
    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(VisuPropertiesHelper::doubleToSlider(meta.min));
    slider->setMaximum(VisuPropertiesHelper::doubleToSlider(meta.max));
    slider->setValue(VisuPropertiesHelper::doubleToSlider(value.toDouble()));
    return std::make_pair(slider, SIGNAL(valueChanged(int)));
}

std::pair<QLineEdit*, const char*> VisuPropertiesHelper::setupDefaultWidget(VisuPropertyMeta meta, QString value)
{
    QLineEdit* edit = new QLineEdit(value);

    if (meta.type == VisuPropertyMeta::FLOAT)
    {
        QValidator *validator = new QDoubleValidator(meta.min, meta.max, 3);
        edit->setValidator(validator);
    }
    return std::make_pair(edit, SIGNAL(editingFinished()));
}

std::pair<QLabel*, const char*> VisuPropertiesHelper::setupReadOnlyWidget(VisuPropertyMeta meta, QString value)
{
    QLabel* label = new QLabel(value);
    return std::make_pair(label, nullptr);
}

std::pair<QWidget*, const char*> VisuPropertiesHelper::controlFactory(VisuPropertyMeta meta, QString value)
{
    std::pair<QWidget*, const char*> widget;

    switch(meta.type)
    {
    case VisuPropertyMeta::ENUM:
        widget = VisuPropertiesHelper::setupEnumWidget(meta, value);
        break;
    case VisuPropertyMeta::FONT:
        widget = VisuPropertiesHelper::setupFontWidget(meta, value);
        break;
    case VisuPropertyMeta::BOOL:
        widget = VisuPropertiesHelper::setupBoolWidget(meta, value);
        break;
    case VisuPropertyMeta::INSTSIGNAL:
        widget = VisuPropertiesHelper::setupSignalsWidget(meta, value);
        break;
    case VisuPropertyMeta::IMAGE:
        widget = VisuPropertiesHelper::setupImagesWidget(meta, value);
        break;
    case  VisuPropertyMeta::COLOR:
        widget = VisuPropertiesHelper::setupColorWidget(meta, value);
        break;
    case VisuPropertyMeta::READ_ONLY:
        widget = VisuPropertiesHelper::setupReadOnlyWidget(meta, value);
        break;
    case VisuPropertyMeta::INT:
        widget = VisuPropertiesHelper::setupIntWidget(meta, value);
        break;
    case VisuPropertyMeta::SLIDER:
        widget = VisuPropertiesHelper::setupSliderWidget(meta, value);
        break;
    case VisuPropertyMeta::SERIAL:
        widget = VisuPropertiesHelper::setupSerialWidget(meta, value);
        break;
    case VisuPropertyMeta::SERIAL_PLACEHOLDER:
        widget = VisuPropertiesHelper::setupSignalPlaceholderWidget(meta, value);
        break;
    default:
        widget = VisuPropertiesHelper::setupDefaultWidget(meta, value);
    }

    return widget;
}

void VisuPropertiesHelper::updateTable(QTableWidget* table,
                           const QMap<QString, QString>& properties,
                           const QMap<QString, VisuPropertyMeta>& metaProperties,
                           std::pair<QWidget*, const char*> object)
{
    int cnt = 0;
    int maxCnt = properties.size();

    table->setEnabled(true);
    table->clearContents();
    table->setRowCount(maxCnt);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(QStringList{"Property", "Value"});

    for (auto i = properties.begin(); i != properties.end(); ++i)
    {
        QString key = i.key();
        QString value = i.value();

        VisuPropertyMeta meta;
        if (metaProperties.contains(key))
        {
            meta = metaProperties[key];
        }
        else
        {
            qDebug() << "Missing meta for key " << key;
            continue;
        }

        if (meta.type != VisuPropertyMeta::HIDDEN)
        {
            QString labelText = meta.label.isEmpty() ? key : meta.label;
            QTableWidgetItem* label = new QTableWidgetItem(labelText);
            label->setFlags(label->flags() & ~Qt::ItemIsEditable);
            label->setToolTip(meta.description);

            table->setItem(meta.order, VisuPropertiesHelper::COLUMN_PROPERTY, label);

            std::pair<QWidget*, const char*> widget = VisuPropertiesHelper::controlFactory(meta, value);

            if (widget.first != nullptr)
            {

                VisuPropertiesHelper::setupTableWidget(table,
                                                       widget,
                                                       object,
                                                       key,
                                                       meta.type,
                                                       meta.order);

                widget.first->setEnabled(meta.isEnabled(properties));
                ++cnt;
            }
        }
        else
        {
            table->setRowCount(table->rowCount() - 1);
        }
    }

    if (cnt < maxCnt)
    {
        table->setRowCount(cnt);
    }

    updateWidgetsState(table, properties, metaProperties);
}

QString VisuPropertiesHelper::getKeyString(QTableWidget* table, int row)
{
    QWidget* w = table->cellWidget(row, VisuPropertiesHelper::COLUMN_VALUE);
    QString ret;

    if (w != nullptr)
    {
        ret = w->property(VisuPropertiesHelper::PROP_KEY).toString();
    }

    return ret;
}

QString VisuPropertiesHelper::getValueString(QTableWidget* table, int row)
{
    QString value;
    QComboBox* box;
    QLineEdit* edit;
    QPushButton* button;
    QSpinBox* spinbox;
    QSlider* slider;
    QCheckBox* checkbox;

    if ( (box = qobject_cast<QComboBox*>(table->cellWidget(row, 1))) != nullptr)
    {
        int type = box->property(VisuPropertiesHelper::PROP_TYPE).toInt();
        switch (type)
        {
        case VisuPropertyMeta::FONT:
        case VisuPropertyMeta::SERIAL:
            value = box->currentText();
            break;
        case VisuPropertyMeta::ENUM:
        case VisuPropertyMeta::SERIAL_PLACEHOLDER:
            value = QString("%1").arg(box->currentIndex());
            break;
        default:
            value = QString("%1").arg(box->currentData().toInt());
        }
    }
    else if ( (spinbox = qobject_cast<QSpinBox*>(table->cellWidget(row, 1))) != nullptr)
    {
        value = QString("%1").arg(spinbox->value());
    }
    else if ( (checkbox = qobject_cast<QCheckBox*>(table->cellWidget(row, 1))) != nullptr)
    {
        value = QString("%1").arg(checkbox->isChecked());
    }
    else if ( (edit = qobject_cast<QLineEdit*>(table->cellWidget(row, 1))) != nullptr )
    {
        value = edit->text();
    }
    else if ( (button = qobject_cast<QPushButton*>(table->cellWidget(row, 1))) != nullptr )
    {
        value = button->text();
    }
    else if (  (slider = qobject_cast<QSlider*>(table->cellWidget(row, 1))) != nullptr )
    {
        value = QString("%1").arg(VisuPropertiesHelper::sliderToDouble(slider->value()));
    }
    else
    {
        value = table->item(row,1)->text();
    }

    return value;
}

void VisuPropertiesHelper::updateWidgetsState(QTableWidget* table,
                                              const QMap<QString, QString>& properties,
                                              const QMap<QString, VisuPropertyMeta>& propertiesMeta)
{
    for (int i = 0 ; i < table->rowCount() ; ++i)
    {
        QString key = VisuPropertiesHelper::getKeyString(table, i);
        if (propertiesMeta.contains(key))
        {
            bool enabled = propertiesMeta.value(key).isEnabled(properties);
            QWidget* w = table->cellWidget(i, VisuPropertiesHelper::COLUMN_VALUE);
            w->setEnabled(enabled);
        }
    }
}

int VisuPropertiesHelper::updateWidgetProperty(QObject* sender, QWidget* parent)
{
    int row = sender->property(VisuPropertiesHelper::PROP_ROW).toInt();

    QPushButton* btn;

    if ( (btn = qobject_cast<QPushButton*>(sender)) != nullptr )
    {
        QColor oldColor = btn->property(VisuPropertiesHelper::PROP_COLOR).value<QColor>();
        QColor newColor = QColorDialog::getColor(oldColor,
                                                 parent,
                                                 QObject::tr("Set color"),
                                                 QColorDialog::ShowAlphaChannel);

        if (newColor.isValid())
        {
            QString colorString = QString("%1,%2,%3,%4").arg(newColor.red())
                    .arg(newColor.green()).arg(newColor.blue()).arg(newColor.alpha());
            btn->setText(colorString);
            VisuMisc::setBackgroundColor(btn, newColor);
        }
    }

    return row;
}
