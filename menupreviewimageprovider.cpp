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

#include "menupreviewimageprovider.h"
#include "freetype-renderer.h"
#include "kxftconfig.h"

MenuPreviewImageProvider::MenuPreviewImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image),
      renderer(Qt::white)
{
}

QImage
MenuPreviewImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    auto parameters = PreviewParameters::fromString(id);
    auto result = renderer.getImage(parameters);
    size->setHeight(result.height());
    size->setWidth(result.width());
    return result;
}

QPixmap
MenuPreviewImageProvider::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize)
{
    auto result = this->requestImage(id, size, requestedSize);
    return QPixmap::fromImage(result);
}
