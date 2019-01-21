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

/** @file
 * Render font setting preview with FreeType & Co.
 */

#include "freetype-renderer.h"

extern "C" {
#include <hb-ft.h>
}

#include <QMap>
#include <QPainter>
#include <QtMath>

/** FreeType divides a pixel into 64 parts */
#define PIXEL_FRACTION_FACTOR 64

/** FreeType uses typographic points defined as 1/72 inch */
#define TYPOGRAHIC_POINTS_PER_INCH 72.0

/******************/
/* FontManagement */
/******************/

FontManagement::FontManagement() : fontConfig{ nullptr }
{
    FcBool fontconfigInit = FcInit();
    if (fontconfigInit == FcTrue) {
        fontConfig = FcInitLoadConfigAndFonts();
    }
}

FontManagement::~FontManagement()
{
    if (fontConfig)
        free(fontConfig);
}

const char* FontManagement::retrievePath(const char* font)
{
    auto pattern = FcNameParse(reinterpret_cast<const FcChar8*>(font));
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    // actual font substitution
    FcResult fcResult;
    auto match = FcFontMatch(fontConfig, pattern, &fcResult);
    if (fcResult != FcResultMatch) {
        FcPatternDestroy(match);
        return "";
    }

    // grab font file path from result
    FcChar8* fontFacePath;
    if (FcPatternGetString(match, FC_FILE, 0, &fontFacePath) != FcResultMatch) {
        FcPatternDestroy(pattern);
        FcPatternDestroy(match);
        return "";
    }

    // pull out path
    auto path = copyFcString(fontFacePath);

    // and clean up
    FcPatternDestroy(pattern);
    FcPatternDestroy(match);

    return path;
}

const char* FontManagement::copyFcString(const FcChar8* fcString)
{
    const char* tmp = reinterpret_cast<const char*>(fcString);
    char* result = new char[strlen(tmp) + 1];
    strcpy(result, tmp);
    return result;
}

/**********************/
/* FreeTypeParameters */
/**********************/

FreeTypeParameters::FreeTypeParameters(KXftConfig options)
    : loadFlags(FT_LOAD_DEFAULT), renderMode(FT_RENDER_MODE_NORMAL)
{
    if (options.antialiasingSetting == KXftConfig::AntiAliasing::Disabled) {
        renderMode = FT_RENDER_MODE_MONO;
        loadFlags |= FT_LOAD_MONOCHROME;
        if (options.hintingSetting == KXftConfig::Hinting::Disabled
            || options.hintstyleSetting == KXftConfig::Hint::None) {
            loadFlags |= FT_LOAD_NO_HINTING;
        } else {
            loadFlags |= FT_LOAD_TARGET_MONO;
        }
    } else {
        // bitmap fonts are disabled when anti-aliasing is used
        loadFlags |= FT_LOAD_NO_BITMAP;
        switch (options.hintstyleSetting) {
        case KXftConfig::Hint::NotSet:
        case KXftConfig::Hint::None:
            loadFlags |= FT_LOAD_NO_HINTING;
            break;
        case KXftConfig::Hint::Slight:
        case KXftConfig::Hint::Medium:
            loadFlags |= FT_LOAD_TARGET_LIGHT;
            break;
        case KXftConfig::Hint::Full:
            // apply hinting appropriate for (sub-)pixel configuration
            switch (options.subpixelSetting) {
            case KXftConfig::SubPixel::NotSet:
            case KXftConfig::SubPixel::None:
                loadFlags |= FT_LOAD_TARGET_NORMAL;
                break;
            case KXftConfig::SubPixel::Rgb:
            case KXftConfig::SubPixel::Bgr:
                loadFlags |= FT_LOAD_TARGET_LCD;
                break;
            case KXftConfig::SubPixel::Vrgb:
            case KXftConfig::SubPixel::Vbgr:
                loadFlags |= FT_LOAD_TARGET_LCD_V;
                break;
            }
        }
        // set render mode for subpixel rendering
        switch (options.subpixelSetting) {
        case KXftConfig::SubPixel::Rgb:
        case KXftConfig::SubPixel::Bgr:
            renderMode = FT_RENDER_MODE_LCD;
            break;
        case KXftConfig::SubPixel::Vrgb:
        case KXftConfig::SubPixel::Vbgr:
            renderMode = FT_RENDER_MODE_LCD_V;
            break;
        default:
            break;
        }
    }
}

/*******************/
/* FreeTypeLibrary */
/*******************/

FreeTypeLibrary::FreeTypeLibrary() : freetypeLib{ nullptr }
{
    FT_Error freetypeInit = FT_Init_FreeType(&freetypeLib);
    if (freetypeInit != 0) {
        // FIXME
    }
}

FreeTypeLibrary::~FreeTypeLibrary()
{
    FT_Done_FreeType(freetypeLib);
    freetypeLib = nullptr;
}

FT_Face FreeTypeLibrary::getFontFace(const char* path)
{
    FT_Face fontFace;
    auto error = FT_New_Face(freetypeLib, path, 0, &fontFace);
    if (error) {
        // FIXME
    }
    return fontFace;
}

long FreeTypeLibrary::convertPointSize(double point_size)
{
    return static_cast<long>(point_size * PIXEL_FRACTION_FACTOR);
}

/***************/
/* RasterGlyph */
/***************/

RasteredGlyph::RasteredGlyph(FT_Bitmap* bitmap, uint width, uint height)
    : pitch(bitmap->pitch)
    , rowLength(static_cast<unsigned int>(abs(pitch)))
    , width(width)
    , height(height)
{
}

unsigned int RasteredGlyph::getHeight() const
{
    return height;
}

unsigned int RasteredGlyph::getWidth() const
{
    return width;
}

/*******************/
/* MonochromeGlyph */
/*******************/

MonochromeGlyph::MonochromeGlyph(FT_Bitmap* bitmap)
    : RasteredGlyph(bitmap, bitmap->width, bitmap->rows)
{
    auto size = height * rowLength;
    this->bitmap = new unsigned char[size]();
    memcpy(this->bitmap, bitmap->buffer, size);
}

MonochromeGlyph::~MonochromeGlyph()
{
    delete[] bitmap;
}

inline int MonochromeGlyph::pixelAt(uint x, uint y, int pitch, const unsigned char* buffer)
{
    uint index = y * static_cast<uint>(abs(pitch)) + x / 8;
    uint position = 7 - x % 8;
    unsigned char byte = buffer[index];
    return byte >> position & 0x1;
}

void MonochromeGlyph::paint(QImage* canvas, int x, int y, const QColor& pen)
{
    for (uint glyphY = 0; glyphY < height; glyphY++) {
        for (uint glyphX = 0; glyphX < width; glyphX++) {
            if (pixelAt(glyphX, glyphY, pitch, bitmap)) {
                int cursorX = static_cast<int>(glyphX) + x;
                int cursorY = static_cast<int>(glyphY) + y;
                canvas->setPixel(cursorX, cursorY, pen.rgb());
            }
        }
    }
}

/*****************/
/* ByteDataGlyph */
/*****************/

ByteDataGlyph::ByteDataGlyph(FT_Bitmap* bitmap, uint bytesPerPixel, uint width, uint height)
    : RasteredGlyph(bitmap, width, height)
{
    const char* buffer = reinterpret_cast<const char*>(bitmap->buffer);
    auto size = static_cast<int>(height * rowLength * bytesPerPixel);
    bytemap = new QByteArray(buffer, size);
}

ByteDataGlyph::~ByteDataGlyph()
{
    delete bytemap;
}

/******************/
/* GrayScaleGlyph */
/******************/

GrayScaleGlyph::GrayScaleGlyph(FT_Bitmap* bitmap)
    : ByteDataGlyph(bitmap, 1, bitmap->width, bitmap->rows)
{
}

void GrayScaleGlyph::paint(QImage* canvas, int x, int y, const QColor& pen)
{
    int pen_r, pen_g, pen_b;
    pen.getRgb(&pen_r, &pen_g, &pen_b);
    for (int j = 0; static_cast<uint>(j) < height; ++j) {
        for (int i = 0; static_cast<uint>(i) < width; ++i) {
            int cursor_x = i + x;
            int cursor_y = j + y;

            auto value = static_cast<unsigned char>(bytemap->at(j * pitch + i));

            int backgound_r, backgound_g, backgound_b;
            canvas->pixelColor(cursor_x, cursor_y).getRgb(&backgound_r, &backgound_g, &backgound_b);

            int result_r = ((255 - value) * backgound_r + value * pen_r) / 255;
            int result_g = ((255 - value) * backgound_g + value * pen_g) / 255;
            int result_b = ((255 - value) * backgound_b + value * pen_b) / 255;
            canvas->setPixel(cursor_x, cursor_y, qRgb(result_r, result_g, result_b));
        }
    }
}

/*************************/
/* AbstractSubPixelGlyph */
/*************************/

AbstractSubPixelGlyph::AbstractSubPixelGlyph(
    FT_Bitmap* bitmap, uint bytesPerPixel, uint width, uint height, bool reversed)
    : ByteDataGlyph(bitmap, bytesPerPixel, width, height), reverse(reversed)
{
}

void AbstractSubPixelGlyph::paint(QImage* canvas, int x, int y, const QColor& pen)
{
    int pen_r, pen_g, pen_b;
    pen.getRgb(&pen_r, &pen_g, &pen_b);

    int offset_r = reverse ? 2 : 0;
    int offset_g = 1;
    int offset_b = reverse ? 0 : 2;

    for (int j = 0; static_cast<uint>(j) < height; ++j) {
        for (int i = 0; static_cast<uint>(i) < width; ++i) {
            int cursor_x = i + x;
            int cursor_y = j + y;

            int backgound_r, backgound_g, backgound_b;
            canvas->pixelColor(cursor_x, cursor_y).getRgb(&backgound_r, &backgound_g, &backgound_b);

            unsigned char value_r = getValue(j, i, offset_r);
            unsigned char value_g = getValue(j, i, offset_g);
            unsigned char value_b = getValue(j, i, offset_b);

            int result_r = ((255 - value_r) * backgound_r + value_r * pen_r) / 255;
            int result_g = ((255 - value_g) * backgound_g + value_g * pen_g) / 255;
            int result_b = ((255 - value_b) * backgound_b + value_b * pen_b) / 255;
            canvas->setPixel(cursor_x, cursor_y, qRgb(result_r, result_g, result_b));
        }
    }
}

/*****************/
/* SubPixelGlyph */
/*****************/

SubPixelGlyph::SubPixelGlyph(FT_Bitmap* bitmap, bool reversed)
    : AbstractSubPixelGlyph(bitmap, 3, bitmap->width / 3, bitmap->rows, reversed)
{
}

inline unsigned char SubPixelGlyph::getValue(int row, int column, int subPixelOffset)
{
    return static_cast<unsigned char>(bytemap->at(row * pitch + 3 * column + subPixelOffset));
}

/*************************/
/* VerticalSubPixelGlyph */
/*************************/

VerticalSubPixelGlyph::VerticalSubPixelGlyph(FT_Bitmap* bitmap, bool reversed)
    : AbstractSubPixelGlyph(bitmap, 3, bitmap->width, bitmap->rows / 3, reversed)
{
}

inline unsigned char VerticalSubPixelGlyph::getValue(int row, int column, int subPixelOffset)
{
    return static_cast<unsigned char>(bytemap->at((3 * row + subPixelOffset) * pitch + column));
}

/*************/
/* GlyphData */
/*************/

GlyphData::GlyphData(hb_glyph_position_t* glyphPos,
                     FT_GlyphSlotRec* glyphData,
                     bool reversedSubpixel)
    : offsetX(static_cast<float>(glyphPos->x_offset) / PIXEL_FRACTION_FACTOR)
    , offsetY(static_cast<float>(glyphPos->y_offset) / PIXEL_FRACTION_FACTOR)
    , advanceX(static_cast<float>(glyphPos->x_advance) / PIXEL_FRACTION_FACTOR)
    , advanceY(static_cast<float>(glyphPos->y_advance) / PIXEL_FRACTION_FACTOR)
    , bearingLeft(glyphData->bitmap_left)
    , bearingTop(glyphData->bitmap_top)
    , pixelData{ nullptr }
{

    switch (glyphData->bitmap.pixel_mode) {
    case FT_PIXEL_MODE_MONO:
        pixelData = new MonochromeGlyph(&glyphData->bitmap);
        break;
    case FT_PIXEL_MODE_GRAY:
        pixelData = new GrayScaleGlyph(&glyphData->bitmap);
        break;
    case FT_PIXEL_MODE_LCD:
        pixelData = new SubPixelGlyph(&glyphData->bitmap, reversedSubpixel);
        break;
    case FT_PIXEL_MODE_LCD_V:
        pixelData = new VerticalSubPixelGlyph(&glyphData->bitmap, reversedSubpixel);
        break;
    case FT_PIXEL_MODE_BGRA:
        // TODO color emoji support
        // Hint: bitmap would be pre-multiplied sRGB image in BGRA order
        //       see FT_PIXEL_MODE_BGRA in FreeType docs
        break;
    }
}

float GlyphData::getOffsetX() const
{
    return offsetX;
}

float GlyphData::getOffsetY() const
{
    return offsetY;
}

float GlyphData::getBearingLeft() const
{
    return bearingLeft;
}

float GlyphData::getBearingTop() const
{
    return bearingTop;
}

float GlyphData::getAdvanceX() const
{
    return advanceX;
}

float GlyphData::getAdvanceY() const
{
    return advanceY;
}

unsigned int GlyphData::getWidth() const
{
    if (pixelData == nullptr)
        return 0;
    return pixelData->getWidth();
}

unsigned int GlyphData::getHeight() const
{
    if (pixelData == nullptr)
        return 0;
    return pixelData->getHeight();
}

void GlyphData::paint(QImage* canvas, int x, int y, const QColor& pen)
{
    if (pixelData == nullptr)
        return;
    pixelData->paint(canvas, x, y, pen);
}

/***************/
/* FontShaping */
/***************/

inline bool _subpixel_reverse(KXftConfig options)
{
    if (options.subpixelSetting == KXftConfig::SubPixel::Bgr)
        return true;
    if (options.subpixelSetting == KXftConfig::SubPixel::Vbgr)
        return true;
    return false;
}

FontShaping::FontShaping(FreeTypeLibrary* freetypeLib,
                         FontManagement* fontManagement,
                         const char* text,
                         const char* font,
                         double pointSize,
                         KXftConfig options)
{
    auto harfbuzzBuffer = hb_buffer_create();

    //  we don't want to compute the length of an Unicode string
    // -1 delegates length recognition to harfbuzz
    hb_buffer_add_utf8(harfbuzzBuffer, text, -1, 0, -1);
    hb_buffer_guess_segment_properties(harfbuzzBuffer);

    path = fontManagement->retrievePath(font);
    auto fontFace = freetypeLib->getFontFace(path);

    auto ftSize = FreeTypeLibrary::convertPointSize(pointSize);
    FT_Set_Char_Size(fontFace, 0, ftSize, 96, 96);
    // TODO DPI

    auto parameters = FreeTypeParameters(options);
    auto loadFlags = parameters.loadFlags;
    auto renderMode = parameters.renderMode;

    auto hbFont = hb_ft_font_create(fontFace, nullptr);

    bool is_hinted = options.hintstyleSetting != KXftConfig::Hint::None;
    hb_font_set_ppem(hbFont, is_hinted ? fontFace->size->metrics.x_ppem : 0,
                     is_hinted ? fontFace->size->metrics.y_ppem : 0);

    hb_shape(hbFont, harfbuzzBuffer, nullptr, 0);

    glyphCount = 0;
    hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(harfbuzzBuffer, &glyphCount);
    hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(harfbuzzBuffer, &glyphCount);

    glyphs = new GlyphData*[glyphCount];

    // assume we have horizontal writing
    baseLineOffset = 0;
    float bottomExtend = 0;

    float width = 0;

    for (unsigned int i = 0; i < glyphCount; ++i) {
        unsigned int glyphIndex = glyphInfo[i].codepoint;
        FT_Load_Glyph(fontFace, glyphIndex, loadFlags);
        auto glyphData = fontFace->glyph;
        FT_Render_Glyph(glyphData, renderMode);

        glyphs[i] = new GlyphData(&glyphPos[i], glyphData, _subpixel_reverse(options));

        auto bearingTop = glyphs[i]->getBearingTop();
        if (bearingTop >= 0) {
            if (bearingTop > baseLineOffset) {
                baseLineOffset = bearingTop;
            }
        }

        auto absBearing = abs(bearingTop);
        auto glyphBottomExtend = glyphs[i]->getHeight() - absBearing;
        if (glyphBottomExtend > bottomExtend) {
            bottomExtend = glyphBottomExtend;
        }

        width += glyphs[i]->getAdvanceX();
    }
    // add width of last glyph
    if (glyphCount) {
        width += glyphs[glyphCount - 1]->getWidth();
    }
    boundingBox = QRectF(0, 0, width, baseLineOffset + bottomExtend);

    // tidy up
    hb_font_destroy(hbFont);
    hb_buffer_destroy(harfbuzzBuffer);
    FT_Done_Face(fontFace);
}

FontShaping::~FontShaping()
{
    for (unsigned int i = 0; i < glyphCount; ++i) {
        delete glyphs[i];
    }
    delete[] glyphs;
}

unsigned int FontShaping::getGlyphCount() const
{
    return glyphCount;
}

GlyphData** FontShaping::getGlyphs() const
{
    return glyphs;
}

unsigned int FontShaping::getBaseLineOffset() const
{
    return baseLineOffset;
}

QRectF FontShaping::getBoundingBox() const
{
    return boundingBox;
}

/*******************************/
/* FreeTypeFontPreviewRenderer */
/*******************************/

QImage FreeTypeFontPreviewRenderer ::renderText(const char* text,
                                                const char* font,
                                                double pointSize,
                                                KXftConfig options,
                                                QColor background,
                                                QColor pen)
{
    FontShaping fontShaping(freeTypeLibrary, fontManagement, text, font, pointSize, options);

    auto width = fontShaping.getBoundingBox().width();
    auto height = fontShaping.getBoundingBox().height();

    QImage canvas(ceill(width), ceill(height), QImage::Format_RGB888);
    canvas.fill(background);

    float x = 0;
    float y = 0;

    for (unsigned int i = 0; i < fontShaping.getGlyphCount(); ++i) {
        GlyphData* data = fontShaping.getGlyphs()[i];
        auto offset = fontShaping.getBaseLineOffset() - data->getBearingTop() + data->getOffsetY();

        data->paint(&canvas, rint(x + data->getBearingLeft()), rint(offset), pen);

        x += data->getAdvanceX();
    }

    return canvas;
}

FreeTypeFontPreviewRenderer::FreeTypeFontPreviewRenderer()
{
    freeTypeLibrary = new FreeTypeLibrary();
    fontManagement = new FontManagement();
}

FreeTypeFontPreviewRenderer::~FreeTypeFontPreviewRenderer()
{
    delete fontManagement;
    delete freeTypeLibrary;
}
