#pragma once

#include <switch.h>
#include <string>
#include <vector>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ot.h>

#include "arabic_font_bin.h"

namespace tsl::ArabicHarfBuzz {

struct Glyph {
    hb_codepoint_t id;
    hb_position_t xAdvance;
    hb_position_t yAdvance;
    hb_position_t xOffset;
    hb_position_t yOffset;
};

struct Context {
    hb_blob_t* blob = nullptr;
    hb_face_t* face = nullptr;
    hb_font_t* font = nullptr;

    Context() {
        blob = hb_blob_create(
            reinterpret_cast<const char*>(arabic_font_bin),
            arabic_font_bin_size,
            HB_MEMORY_MODE_READONLY,
            nullptr,
            nullptr
        );

        face = hb_face_create(blob, 0);
        font = hb_font_create(face);

        hb_ot_font_set_funcs(font);

        const unsigned int upem = hb_face_get_upem(face);

        hb_font_set_scale(
            font,
            static_cast<int>(upem),
            static_cast<int>(upem)
        );
    }

    ~Context() {
        if (font)
            hb_font_destroy(font);

        if (face)
            hb_face_destroy(face);

        if (blob)
            hb_blob_destroy(blob);
    }
};

inline Context& getContext() {
    static Context context;
    return context;
}

inline bool isArabic(u32 cp) {
    return
        (cp >= 0x0600 && cp <= 0x06FF) ||
        (cp >= 0x0750 && cp <= 0x077F) ||
        (cp >= 0x08A0 && cp <= 0x08FF) ||
        (cp >= 0xFB50 && cp <= 0xFDFF) ||
        (cp >= 0xFE70 && cp <= 0xFEFF);
}

inline bool containsArabic(const char* text) {
    if (!text)
        return false;

    const u8* p = reinterpret_cast<const u8*>(text);

    while (*p) {
        u32 cp = 0;

        const ssize_t width = decode_utf8(&cp, p);

        if (width <= 0)
            break;

        if (isArabic(cp))
            return true;

        p += width;
    }

    return false;
}

inline bool shape(
    const char* text,
    std::vector<Glyph>& output
) {
    output.clear();

    if (!text || !containsArabic(text))
        return false;

    Context& ctx = getContext();

    if (!ctx.font)
        return false;

    hb_buffer_t* buffer = hb_buffer_create();

    if (!buffer)
        return false;

    hb_buffer_set_direction(
        buffer,
        HB_DIRECTION_RTL
    );

    hb_buffer_set_script(
        buffer,
        HB_SCRIPT_ARABIC
    );

    hb_buffer_set_language(
        buffer,
        hb_language_from_string("ar", -1)
    );

    hb_buffer_add_utf8(
        buffer,
        text,
        -1,
        0,
        -1
    );

    hb_shape(
        ctx.font,
        buffer,
        nullptr,
        0
    );

    unsigned int count = 0;

    hb_glyph_info_t* infos =
        hb_buffer_get_glyph_infos(
            buffer,
            &count
        );

    hb_glyph_position_t* positions =
        hb_buffer_get_glyph_positions(
            buffer,
            nullptr
        );

    if (!infos || !positions || count == 0) {
        hb_buffer_destroy(buffer);
        return false;
    }

    output.reserve(count);

    for (unsigned int i = 0; i < count; ++i) {
        output.push_back({
            infos[i].codepoint,
            positions[i].x_advance,
            positions[i].y_advance,
            positions[i].x_offset,
            positions[i].y_offset
        });
    }

    hb_buffer_destroy(buffer);

    return true;
}

} // namespace tsl::ArabicHarfBuzz
