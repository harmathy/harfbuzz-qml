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

#ifndef RENDERING_OPTIONS_H
#define RENDERING_OPTIONS_H

#include <QString>

class KXftConfig {
  public:
    const enum class AntiAliasing { NotSet, Disabled, Enabled } antialiasingSetting;
    const enum class Hinting { Disabled, Enabled } hintingSetting;
    const enum class Hint { NotSet, None, Slight, Medium, Full } hintstyleSetting;
    const enum class SubPixel { NotSet, None, Rgb, Bgr, Vrgb, Vbgr } subpixelSetting;

    const uint dpiH;
    const uint dpiV;

    KXftConfig(AntiAliasing antialiasingSetting,
               Hinting hintingSetting,
               Hint hintstyleSetting,
               SubPixel subpixelSetting,
               uint dpiH,
               uint dpiV);

    KXftConfig(AntiAliasing antialiasingSetting,
               Hinting hintingSetting,
               Hint hintstyleSetting,
               SubPixel subpixelSetting,
               uint dpi = 72);

    QString getAaState();
    QString getHintingState();
    QString getHintstyle();
    QString getUnifiedHintingState();
    QString getSubpixelState();
};

#endif // RENDERING_OPTIONS_H
