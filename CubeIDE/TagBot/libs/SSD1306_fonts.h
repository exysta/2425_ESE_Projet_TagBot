/**
 * @file SSD1306_fonts.h
 * @brief Font definitions and image declarations for the SSD1306 OLED display driver.
 *
 * This header defines the structure and external declarations for fonts and images
 * used with the SSD1306 OLED display. Conditional compilation allows inclusion
 * of specific fonts and images.
 *
 */

#ifndef __SSD1306_FONTS_H__
#define __SSD1306_FONTS_H__

#include <stdint.h> // Include for standard integer types

/** @brief Include 6x8 pixel font if defined. */
#define SSD1306_INCLUDE_FONT_6x8

/** @brief Include 7x10 pixel font if defined. */
#define SSD1306_INCLUDE_FONT_7x10

/** @brief Include 11x18 pixel font if defined. */
#define SSD1306_INCLUDE_FONT_11x18

/** @brief Include 16x26 pixel font if defined. */
#define SSD1306_INCLUDE_FONT_16x26

/** @brief Include 16x24 pixel font if defined. */
#define SSD1306_INCLUDE_FONT_16x24

/** @brief Include images if defined. */
#define SSD1306_INCLUDE_IMG

#include "SSD1306_fonts.h" // Include the implementation for the font data

/**
 * @brief Font definition structure.
 *
 * This structure defines the properties of a font, including its width, height,
 * and a pointer to its pixel data array.
 */
typedef struct {
    const uint8_t FontWidth;    /*!< Width of the font characters in pixels. */
    uint8_t FontHeight;         /*!< Height of the font characters in pixels. */
    const uint16_t *data;       /*!< Pointer to the font data array. */
} FontDef;

/* External declarations for fonts, conditionally included based on macros. */

#ifdef SSD1306_INCLUDE_FONT_6x8
/** @brief Font definition for 6x8 pixel characters. */
extern FontDef Font_6x8;
#endif

#ifdef SSD1306_INCLUDE_FONT_7x10
/** @brief Font definition for 7x10 pixel characters. */
extern FontDef Font_7x10;
#endif

#ifdef SSD1306_INCLUDE_FONT_11x18
/** @brief Font definition for 11x18 pixel characters. */
extern FontDef Font_11x18;
#endif

#ifdef SSD1306_INCLUDE_FONT_16x26
/** @brief Font definition for 16x26 pixel characters. */
extern FontDef Font_16x26;
#endif

#ifdef SSD1306_INCLUDE_FONT_16x24
/** @brief Font definition for 16x24 pixel characters. */
extern FontDef Font_16x24;
#endif

/* External declarations for images, conditionally included based on macros. */

#ifdef SSD1306_INCLUDE_IMG
/** @brief Image data for a 115x64 pixel Nyan Cat graphic. */
extern const uint8_t Nyan_115x64px[];

/** @brief Image data for a 50x64 pixel Jerry Mouse graphic. */
extern const uint8_t Jerry_50x64px[];
#endif

#endif // __SSD1306_FONTS_H__
