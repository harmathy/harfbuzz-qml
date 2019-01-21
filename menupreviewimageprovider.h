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

#ifndef RENERPREVIEWIMAGEPROVIDER_H
#define RENERPREVIEWIMAGEPROVIDER_H

#include "menupreview.h"

#include <QQuickImageProvider>

class MenuPreviewImageProvider : public QQuickImageProvider
{
private:
    MenuPreviewRenderer renderer;

public:
    MenuPreviewImageProvider();

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;
    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) override;
};

#endif // RENERPREVIEWIMAGEPROVIDER_H
