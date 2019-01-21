/*
 * Copyright 2018 Max Harmathy
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "menupreview.h"
#include "freetype-renderer.h"

#include <QApplication>
#include <QGridLayout>
#include <QIcon>
#include <QPainter>
#include <QtMath>

namespace
{
static const int MAX_PREVIEW_WIDTH = 120;
static const int MAX_PREVIEW_HIGHT = 240;
}

PreviewParameters::PreviewParameters(const QString& fontFamily,
                                     double pointSize,
                                     KXftConfig options)
    : fontFamily(fontFamily), pointSize(pointSize), options(options)
{
}

PreviewParameters PreviewParameters::fromString(const QString& id, uint dpiH, uint dpiV)
{
    auto fragments = id.split("/");
    QString fontFamily = "";
    float pointSize = 10;
    auto antialiasingSetting = KXftConfig::AntiAliasing::Disabled;
    auto hintingSetting = KXftConfig::Hinting::Enabled;
    auto hintstyleSetting = KXftConfig::Hint::None;
    auto subpixelSetting = KXftConfig::SubPixel::None;

    if (fragments.length() <= 5) {
        fontFamily = fragments[0];
        pointSize = fragments[1].toFloat();
        antialiasingSetting = static_cast<KXftConfig::AntiAliasing>(fragments[2].toInt());
        hintstyleSetting = static_cast<KXftConfig::Hint>(fragments[3].toInt());
        subpixelSetting = static_cast<KXftConfig::SubPixel>(fragments[4].toInt());
    }
    if (hintstyleSetting == KXftConfig::Hint::None) {
        hintingSetting = KXftConfig::Hinting::Disabled;
    }
    return PreviewParameters(fontFamily, pointSize,
                             KXftConfig(antialiasingSetting, hintingSetting, hintstyleSetting,
                                        subpixelSetting, dpiH, dpiV));
}

QString PreviewParameters::toFormatetString()
{
    auto typeface = QString("%1 %2").arg(fontFamily).arg(pointSize);
    auto antialiasing = options.getAaState();
    auto hint = options.getHintstyle();
    auto subpixel = options.getSubpixelState();
    return QString("Typeface:\t%1\nAnti-Aliasing:\t%2\nHinting Style:\t%3\nSub-Pixel Order:\t%4")
        .arg(typeface, antialiasing, hint, subpixel);
}

EntryMockup::EntryMockup(const QString& label, const QString& iconName)
    : label{ label }, iconName{ iconName }
{
}

const QString& EntryMockup::getIconName() const
{
    return iconName;
}

const QString& EntryMockup::getLabel() const
{
    return label;
}

void MenuMockup::add(const EntryMockup& item)
{
    entries.append(item);
}

QString MenuMockup::getLabel(int index) const
{
    return entries.at(index).getLabel();
}

QString MenuMockup::getIconName(int index) const
{
    return entries.at(index).getIconName();
}

MenuMockup MenuMockup::basicExample()
{
    MenuMockup result;
    result.add(EntryMockup("Office", "applications-office"));
    result.add(EntryMockup("Internet", "applications-internet"));
    result.add(EntryMockup("Multimedia", "applications-multimedia"));
    result.add(EntryMockup("Graphics", "applications-graphics"));
    result.add(EntryMockup("Accessories", "applications-accessories"));
    result.add(EntryMockup("Development", "applications-development"));
    result.add(EntryMockup("Settings", "preferences-system"));
    result.add(EntryMockup("System", "applications-system"));
    result.add(EntryMockup("Utilities", "applications-utilities"));
    return result;
}

int MenuMockup::length() const
{
    return entries.length();
}

MenuPreviewRenderer::MenuPreviewRenderer(const QColor& background, int iconSize, int padding)
    : renderer(), iconSize(iconSize), padding(padding), background(background)
{
}

QImage MenuPreviewRenderer::getImage(const PreviewParameters& parameters)
{
    const auto menu = MenuMockup::basicExample();
    QList<QImage> lables;
    QList<QIcon> icons;
    QSize dimensions(0, 2 * padding);
    for (int i = 0; i < menu.length(); ++i) {
        auto image = renderer.renderText(menu.getLabel(i).toLocal8Bit(),
                                         parameters.fontFamily.toLocal8Bit(), parameters.pointSize,
                                         parameters.options, background, Qt::black);
        dimensions.rheight() += qMax(image.height(), iconSize) + 2 * padding;
        dimensions.setWidth(qMax(dimensions.width(), image.width()));
        lables.append(image);
        icons.append(QIcon::fromTheme(menu.getIconName(i)));
    }
    dimensions.rwidth() += iconSize + 4 * padding;
    QImage result(dimensions, QImage::Format_ARGB32);
    result.fill(background);
    QPainter p(&result);

    for (int i = 0, y = padding; i < menu.length(); ++i) {
        auto image = lables.at(i);
        auto icon = icons.at(i).pixmap(iconSize, iconSize);
        int heightOffset = (icon.height() - image.height()) / 2;
        bool iconIsSmaller = heightOffset < 0;
        if (iconIsSmaller) {
            heightOffset = (-heightOffset);
        }

        p.drawPixmap(QRectF(padding, y + (iconIsSmaller ? heightOffset : 0), iconSize, iconSize),
                     icon, QRectF(0, 0, iconSize, iconSize));

        p.drawImage(QRectF(iconSize + 3 * padding, y + (!iconIsSmaller ? heightOffset : 0),
                           image.width(), image.height()),
                    image, QRectF(0, 0, image.width(), image.height()));
        y += qMax(image.height(), iconSize) + 2 * padding;
    }
    p.end();
    return result;
}

#include "menupreview.moc"
