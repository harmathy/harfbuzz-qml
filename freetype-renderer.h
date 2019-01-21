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

#ifndef FREETYPE_RENDERER_H
#define FREETYPE_RENDERER_H

#include "kxftconfig.h"

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>
#include <hb.h>
}

#include <QBitArray>
#include <QByteArray>
#include <QColor>
#include <QImage>
#include <QRectF>

/**
 * @brief The FontManagement class is a wrapper around the Fontconfig library.
 *
 * Fontconfig is commonly used to manage font files on the system and has a complex configuration
 * system. In particular it handles font substitution. It therefore can provide a font file for a
 * given font specification, which is used in the retrievePath method.
 */
class FontManagement
{
public:
    /**
     * FontManagement constructor initializes the Fontconfig library.
     */
    FontManagement();

    ~FontManagement();

    FontManagement& operator=(const FontManagement&) = delete;
    FontManagement(const FontManagement&) = delete;

    /**
     * @brief retrievePath fetches the path of a specified font.
     *
     * Here the Fontconfig library is used to substitute the font according to the system settings
     * and to locate the font file. The font file path can be used to load the font in FreeType. The
     * path is copied from the Fontconfig data structures which are destroyed after this method
     * call.
     *
     * @param font name to specify the font
     * @return path to the font file
     */
    const char* retrievePath(const char* font);

    /**
     * @brief copyFcString helper method to retrieve the font path from Fontconfig data structures
     * @param fcString pointer to the path in the data structure
     * @return string copy of the path
     */
    static inline const char* copyFcString(const FcChar8* fcString);

private:
    FcConfig* fontConfig;
};

/**
 * Hold parameters for FreeType, which influence the rendering result.
 */
class FreeTypeParameters
{
public:
    /**
     * Convert rendering options from the commonly used Fontconfig style into parameters for
     * FreeType. FreeTypeParameters does not take ownership of the KXftConfig object.
     */
    FreeTypeParameters(KXftConfig* options);

    int loadFlags;
    FT_Render_Mode renderMode;
    // FT_Library_SetLcdFilter
};

/**
 * @brief The FreeTypeLibrary class
 */
class FreeTypeLibrary
{
private:
    FT_Library freetypeLib;

public:
    FreeTypeLibrary();
    virtual ~FreeTypeLibrary();

    FT_Face getFontFace(const char* path);

    /**
     * This inline function converts point size from float to internal integer representation used
     * by FreeType. Keep in mind that point size isn't a discrete measure and therefore a float.
     * @param pointSize size in typographic units
     * @return internal integer representation used by FreeType
     */
    static inline long convertPointSize(double pointSize);
};

/**
 * @brief The RasteredGlyph class is the base class for rasters glyph data from FreeType.
 *
 * This class is an abstract class to provide a common interface to the different format of glyph
 * data provided by FreeType. Depending on the parameters for font rendering FreeType uses different
 * approached for storing the data, e.g. if anti-aliasing is turned off, glyphs are rendered in
 * monochrome (handles by subclass MonochromeGlyph) using only one bit per pixel.
 */
class RasteredGlyph
{

protected:
    /**
     * @brief pitch is the length of one row of glyph data in bytes and signedness encoding
     * direction.
     *
     * The pitch is copied from the FreeType glyph data structure. It can be positive or negative
     * depending on the direction, in which the glyph is renders. See also FT_Bitmap in the FreeType
     * documentation. An always positive length value is stored in rowLength.
     */
    const int pitch;

    /**
     * @brief rowLength is the length of one row of glyph data in bytes.
     *
     * In contrast to pitch this value is always positive and can be used to compute the required
     * space of the glyph.
     */
    const unsigned int rowLength;

    /**
     * @brief width of the glyph in pixels.
     */
    const unsigned int width;

    /**
     * @brief height of the glyph in pixels.
     */
    const unsigned int height;

public:
    /**
     * @brief RasteredGlyph constructor
     *
     * The bitmap data structure has also a width and height (rows) field, however for sub-pixel
     * rendered bitmaps the respective value will be in sub-pixels, i.e. three times larger. This
     * should be handled by the constructor of the specialized sub-class.
     * @param bitmap the rendering result from FreeType
     * @param width the actual width in pixels
     * @param height the actual height in pixels
     */
    RasteredGlyph(FT_Bitmap* bitmap, uint width, uint height);

    virtual ~RasteredGlyph() = default;

    /**
     * @brief paint method will paint the glyph to the canvas given at position (x,y).
     *
     * The rendering slightly differs depending on the given font rendering options. Therefore this
     * is an abstract method, which has to be implemented by inheriting classes. For anti-aliased
     * and sub-pixel based rendering the glyph has to be painted using alpha blending, since the
     * FreeType data yields the coverage of a (sub-)pixel.
     * @param canvas where the glyph is to be painted on.
     * @param x leftmost coordinate where to start painting the glyph.
     * @param y upmost coordinate where to start painting the glyph.
     * @param pen color, in which the glyph will be painted.
     */
    virtual void paint(QImage* canvas, int x, int y, const QColor& pen) = 0;

    /**
     * @return the pixel height.
     */
    unsigned int getHeight() const;

    /**
     * @return the pixel width.
     */
    unsigned int getWidth() const;
};

/**
 * @brief The MonochromeGlyph class handles glyph data rendered without anti-aliasing.
 *
 * For every pixel there are only two possible states: either the glyph covers it, or not. Therefore
 * the data is stored in a bitmap, where one bit denotes one pixel.
 */
class MonochromeGlyph : public RasteredGlyph
{
protected:
    /**
     * @brief bitmap holds the pixel data of a rendered glyph, where every bit denotes a pixel.
     */
    unsigned char* bitmap;

public:
    /**
     * @brief MonochromeGlyph constructor.
     *
     * The width and height are passed from the FT_Bitmap input to the base constructor (@ref
     * RasteredGlyph::RasteredGlyph). Side note: the width field in FT_Bitmap for monochrome glyph
     * data gives the width in pixels (in contrast sub-pixel rendered glyph data).
     * @param bitmap the rendering result from FreeType
     */
    MonochromeGlyph(FT_Bitmap* bitmap);

    /**
     * @brief ~MonochromeGlyph frees private bitmap data copied from FreeType bitmap.
     */
    virtual ~MonochromeGlyph();

    /**
     * @copydoc RasteredGlyph::paint
     *
     * This will paint the monochrome glyph to the canvas. Since there is no anti-aliasing involved
     * no alpha blending is needed.
     */
    virtual void paint(QImage* canvas, int x, int y, const QColor& pen) override;

    /**
     * @brief pixelAt is a helper function to access the bit data of a single pixel.
     *
     * This is necessary since not only one byte holds the data of eight pixels but also they are
     * stored in most-significant bit order.
     * @param x horizontal pixel position in the glyph bitmap
     * @param y vertical pixel position in the glyph bitmap
     * @param pitch see @ref RasteredGlyph::pitch
     * @param buffer see @ref bitmap
     * @return 1 if the pixel is covered by the glyph and 0 otherwise
     */
    inline int pixelAt(uint x, uint y, int pitch, const unsigned char* buffer);
};

/**
 * @brief The ByteDataGlyph class is an abstract class for glyph data, where pixel data is stored in
 * bytes.
 *
 * This is the base class for anti-aliased and sub-pixel rendered glyph data.
 */
class ByteDataGlyph : public RasteredGlyph
{
protected:
    /**
     * @brief bytemap stores the glyph data copied from the FreeType bitmap.
     */
    QByteArray* bytemap;

public:
    /**
     * @brief ByteDataGlyph constructor.
     * @param bitmap see @ref RasteredGlyph::RasteredGlyph
     * @param bytesPerPixel number of bytes used to store one pixel. For gray scale glyphs it would
     *        be set to 1, for sub-pixel rendered glyphs it would be 3 (one for each sub-pixel).
     * @param width see @ref RasteredGlyph::RasteredGlyph
     * @param height see @ref RasteredGlyph::RasteredGlyph
     */
    ByteDataGlyph(FT_Bitmap* bitmap, uint bytesPerPixel, uint width, uint height);

    /**
     * @brief ~ByteDataGlyph frees private byte array with data copied from FreeType bitmap.
     */
    virtual ~ByteDataGlyph();

    /**
     * @copydoc RasteredGlyph::paint
     */
    virtual void paint(QImage* canvas, int x, int y, const QColor& pen) override = 0;
};

/**
 * @brief The GrayScaleGlyph class represents anti-aliased glyphs, rendered on pixel level.
 */
class GrayScaleGlyph : public ByteDataGlyph
{
public:
    /**
     * @brief GrayScaleGlyph constructor. Parameters for base classes are initialized.
     * @param bitmap see @ref RasteredGlyph::RasteredGlyph
     */
    GrayScaleGlyph(FT_Bitmap* bitmap);

    /**
     * @copydoc RasteredGlyph::paint
     *
     * Pen color will be alpha blended onto the background.
     */
    virtual void paint(QImage* canvas, int x, int y, const QColor& pen) override;
};

/**
 * @brief The AbstractSubPixelGlyph class is an abstract class for sub-pixel rendered glyphs.
 *
 * FreeType can render glyphs for utilizing sub-pixels of the displays panel to increase resolution.
 * FreeType assumed a pixel geometry of three sub-pixels (stripes). The rendering result from
 * FreeType is independent from the actual the sub-pixel color order. Usually LCD-Panels are
 * oriented in a way, that sub-pixels have the order Red, Green, Blue from left to right. If a
 * display is used in a turned position, the orientation and order of the sub-pixels change and
 * glyphs have to be rendered and displayed differently.
 *
 * This class implements the common per sub-pixel alpha blending in the paint method. Since
 * horizontal and vertical sub-pixel rendered glyphs are stored differently by FreeType the access
 * is encapsulated in the sub-classes SubPixelGlyph and VerticalSubPixelGlyph, which implement the
 * getValue method.
 */
class AbstractSubPixelGlyph : public ByteDataGlyph
{
protected:
    /**
     * @brief This indicates if the sub-pixel order is turned around, i.e. BGR instead of RGB.
     *
     * Setting this to true will swap the offset for accessing red and blue sub-pixel information.
     */
    bool reverse;

    /**
     * @brief getValue retrieve the sub-pixel value of a pixel in the rendered glyph image.
     * @param row in rendered glyph, y coordinate
     * @param column in rendered glyph, x coordinate
     * @param offset selector for sub-pixel is one out of 0, 1 or 2.
     *        Depending on the orientation 0 selects the left or top sub-pixel, 1 selects the middle
     *        sub-pixel and 2 selects the right or bottom sub-pixel.
     * @return the area of the sub-pixel covered by the glyph in a range from 0 to 255, where 0 is
     *         not covered and 255 is fully covered.
     */
    virtual unsigned char getValue(int row, int column, int offset) = 0;

public:
    AbstractSubPixelGlyph(
        FT_Bitmap* bitmap, uint bytesPerPixel, uint width, uint height, bool reversed);

    /**
     * @copydoc RasteredGlyph::paint
     *
     * Pen color will be alpha blended separately for every sub-pixel onto the background.
     */
    virtual void paint(QImage* canvas, int x, int y, const QColor& pen) override;
};

/**
 * @brief The SubPixelGlyph class represents sub-pixel rendered glyphs for horizontal sub-pixel
 *        orientation.
 */
class SubPixelGlyph : public AbstractSubPixelGlyph
{
protected:
    virtual inline unsigned char getValue(int row, int, int offset) override;

public:
    SubPixelGlyph(FT_Bitmap* bitmap, bool reversed);
};

/**
 * @brief The VerticalSubPixelGlyph class represents sub-pixel rendered glyphs for vertical
 *        sub-pixel orientation.
 */
class VerticalSubPixelGlyph : public AbstractSubPixelGlyph
{
protected:
    virtual inline unsigned char getValue(int row, int column, int offset) override;

public:
    VerticalSubPixelGlyph(FT_Bitmap* bitmap, bool reversed);
};

/**
 * @brief The GlyphData class holds metadata together with rendered glyph data.
 *
 * With the paint method it ca be used to directly put the rendered glyph on a QImage.
 */
class GlyphData
{
private:
    const float offsetX;
    const float offsetY;
    const float advanceX;
    const float advanceY;
    const float bearingLeft;
    const float bearingTop;

    RasteredGlyph* pixelData;

public:
    /**
     * @brief GlyphData constructor
     * @param glyphPos is a Harfbuzz position data structure from a Harfbuzz font shaping run.
     * @param glyphData is the data structure from FreeType containing the render results of a
     *        glyph.
     * @param reversedSubpixel This is only relevant for sub-pixel rendered glyphs. Set this to true
     *        for bgr and vbgr sub-pixel order. See also @ref AbstractSubPixelGlyph::reverse.
     */
    GlyphData(hb_glyph_position_t* glyphPos,
              FT_GlyphSlotRec* glyphData,
              bool reversedSubpixel = false);
    float getOffsetX() const;
    float getOffsetY() const;
    float getBearingLeft() const;
    float getBearingTop() const;
    float getAdvanceX() const;
    float getAdvanceY() const;
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    void paint(QImage* canvas, int x, int y, const QColor& pen);
};

/**
 * @brief The FontShaping class contains all data provided by a font shaping run using Harfbuzz.
 *
 * In order to estimate the space needed by the text the actual rendering, i.e. the rasterization
 * but not the painting to a surface, is conducted.
 */
class FontShaping
{
private:
    const char* path;

    unsigned int glyphCount;
    GlyphData** glyphs;
    float baseLineOffset;
    QRectF boundingBox;

public:
    /**
     * @brief FontShaping constructor conducts the shaping and the rendering steps.
     * @param freetypeLib a FreeTypeLibrary object
     * @param fontManagement a FontManagement object
     * @param text see @ref FreeTypeFontPreviewRenderer::renderText
     * @param font see @ref FreeTypeFontPreviewRenderer::renderText
     * @param pointSize see @ref FreeTypeFontPreviewRenderer::renderText
     * @param options see @ref FreeTypeFontPreviewRenderer::renderText
     */
    FontShaping(FreeTypeLibrary* freetypeLib,
                FontManagement* fontManagement,
                const char* text,
                const char* font,
                double pointSize,
                KXftConfig* options);

    ~FontShaping();

    unsigned int getGlyphCount() const;
    GlyphData** getGlyphs() const;
    unsigned int getBaseLineOffset() const;
    QRectF getBoundingBox() const;
};

/**
 * The FreeTypeFontPreviewRenderer class provides the possibility to render Text with FreeType
 * offside.
 */
class FreeTypeFontPreviewRenderer
{
private:
    FreeTypeLibrary* freeTypeLibrary;
    FontManagement* fontManagement;

public:
    /**
     * @brief FreeTypeFontPreviewRenderer
     *
     * The constructor initializes the library classes for Harfbuzz, FreeType and Fontconfig.
     */
    explicit FreeTypeFontPreviewRenderer();
    virtual ~FreeTypeFontPreviewRenderer();

    /**
     * @brief Render text independent from render settings of the running session.
     *
     * The given text will be rendered offside using the options provided as parameters. This is
     * intended to be presented to a user to give an impression, what rendering with the given
     * parameters would look without the need to change the actual setting for the session.
     * @param text string to render
     * @param font in which the text should be rendered
     * @param pointSize is the font size in typographic points
     * @param options config object which contains anti-aliasing, hinting and sub-pixel settings
     * @param background color
     * @param pen writing color
     * @return rendered text as QImage
     */
    QImage renderText(const char* text,
                      const char* font,
                      double pointSize,
                      KXftConfig* options,
                      QColor background,
                      QColor pen);
};

#endif // FREETYPE_RENDERER_H
