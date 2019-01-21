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

#include "kxftconfig.h"

KXftConfig::KXftConfig(AntiAliasing antialiasingSetting,
                       Hinting hintingSetting,
                       Hint hintstyleSetting,
                       SubPixel subpixelSetting,
                       uint dpiH,
                       uint dpiV)
    : antialiasingSetting(antialiasingSetting),
      hintingSetting(hintingSetting),
      hintstyleSetting(hintstyleSetting),
      subpixelSetting(subpixelSetting),
      dpiH(dpiH),
      dpiV(dpiV){};

KXftConfig::KXftConfig(AntiAliasing antialiasingSetting,
                       Hinting hintingSetting,
                       Hint hintstyleSetting,
                       SubPixel subpixelSetting,
                       uint dpi)
    : KXftConfig(antialiasingSetting, hintingSetting, hintstyleSetting, subpixelSetting, dpi, dpi) {}

QString KXftConfig::getAaState() {
    if (antialiasingSetting == KXftConfig::AntiAliasing::Enabled) {
        return "enabled";
    }
    return "disabled";
}

QString KXftConfig::getHintingState() {
    if (hintingSetting == KXftConfig::Hinting::Enabled) {
        return "enabled";
    }
    return "disabled";
}

QString KXftConfig::getHintstyle() {
    if (hintstyleSetting == KXftConfig::Hint::Full) {
        return "full";
    }
    if (hintstyleSetting == KXftConfig::Hint::Medium) {
        return "medium";
    }
    if (hintstyleSetting == KXftConfig::Hint::Slight) {
        return "slight";
    }
    return "none";
}

QString KXftConfig::getUnifiedHintingState() {
    if (hintingSetting == KXftConfig::Hinting::Enabled) {
        return getHintstyle();
    }
    return getHintingState();
}

QString KXftConfig::getSubpixelState() {
    if (subpixelSetting == KXftConfig::SubPixel::Rgb) {
        return "rgb";
    }
    if (subpixelSetting == KXftConfig::SubPixel::Bgr) {
        return "bgr";
    }
    if (subpixelSetting == KXftConfig::SubPixel::Vrgb) {
        return "vrgb";
    }
    if (subpixelSetting == KXftConfig::SubPixel::Vbgr) {
        return "vbgr";
    }
    return "none";
}
