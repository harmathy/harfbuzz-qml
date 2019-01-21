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

#ifndef MENUPREVIEW_H
#define MENUPREVIEW_H

#include "freetype-renderer.h"
#include "kxftconfig.h"

#include <QButtonGroup>
#include <QImage>
#include <QList>
#include <QPushButton>
#include <QString>

/**
 * @brief The PreviewParameters is a helper class for communication between qml and
 * QQuickImageProvider.
 */
class PreviewParameters
{
public:
    QString fontFamily;
    double pointSize;
    KXftConfig options;

    PreviewParameters(const QString& fontFamily, double pointSize, KXftConfig options);
    static PreviewParameters fromString(const QString& id, uint dpiH = 72, uint dpiV = 72);
    QString toFormatetString();
};

class EntryMockup
{
    QString label;
    QString iconName;

public:
    EntryMockup(const QString& label, const QString& iconName);

    const QString& getIconName() const;
    const QString& getLabel() const;
};

class MenuMockup
{
    QList<EntryMockup> entries;

public:
    void add(const EntryMockup& item);
    QString getLabel(int index) const;
    QString getIconName(int index) const;
    static MenuMockup basicExample();
    int length() const;
};

class MenuPreviewArea;

class MenuPreviewRenderer
{
private:
    FreeTypeFontPreviewRenderer renderer;
    const int iconSize;
    const int padding;
    const QColor background;

public:
    MenuPreviewRenderer(const QColor& background, int iconSize = 16, int padding = 2);
    QImage getImage(const PreviewParameters& parameters);
};

#endif // MENUPREVIEW_H
