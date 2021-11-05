#include "Font.h"

#include "Graphics/Text/Text.h"

namespace Wraith
{
    Font::Font(msdfgen::FreetypeHandle* freetypeHandle) { m_Freetype.handle = freetypeHandle; }

    bool Font::Init(std::filesystem::path fontPath)
    {
        m_Freetype.font_handle = msdfgen::loadFont(m_Freetype.handle, fontPath.generic_string().c_str());
        if (!m_Freetype.font_handle)
            return false;

        msdfgen::FontMetrics font_metrics;
        msdfgen::getFontMetrics(font_metrics, m_Freetype.font_handle);
        m_FontScale = (FONT_SIZE / font_metrics.emSize);
        m_LineHeight = font_metrics.lineHeight / font_metrics.emSize;

        m_HarfBuzz.buffer = hb_buffer_create();
        if (!m_HarfBuzz.buffer)
            return false;
        m_HarfBuzz.blob = hb_blob_create_from_file(fontPath.generic_string().c_str());
        if (!m_HarfBuzz.blob)
            return false;
        m_HarfBuzz.face = hb_face_create(m_HarfBuzz.blob, 0);
        if (!m_HarfBuzz.face)
            return false;
        m_HarfBuzz.font = hb_font_create(m_HarfBuzz.face);
        if (!m_HarfBuzz.font)
            return false;

        int x_scale;
        int y_scale;
        hb_font_get_scale(m_HarfBuzz.font, &x_scale, &y_scale);
        m_HarfBuzz.scale = Vec2f(x_scale, y_scale);

        return true;
    }

    void Font::Release()
    {
        if (m_Freetype.font_handle)
        {
            msdfgen::destroyFont(m_Freetype.font_handle);
            m_Freetype.font_handle = nullptr;
        }

        if (m_HarfBuzz.font)
        {
            hb_font_destroy(m_HarfBuzz.font);
            m_HarfBuzz.font = nullptr;
        }
        if (m_HarfBuzz.face)
        {
            hb_face_destroy(m_HarfBuzz.face);
            m_HarfBuzz.face = nullptr;
        }
        if (m_HarfBuzz.blob)
        {
            hb_blob_destroy(m_HarfBuzz.blob);
            m_HarfBuzz.blob = nullptr;
        }
        if (m_HarfBuzz.buffer)
        {
            hb_buffer_destroy(m_HarfBuzz.buffer);
            m_HarfBuzz.buffer = nullptr;
        }
    }

    std::optional<Font::ShapeData> Font::LoadShape(msdfgen::GlyphIndex glyph_index)
    {
        msdfgen::Shape shape;
        if (!msdfgen::loadGlyph(shape, m_Freetype.font_handle, glyph_index))
        {
            return std::nullopt;
        }
        if (shape.edgeCount() == 0)
        {
            return std::nullopt;
        }
        shape.normalize();

        const auto bounds = shape.getBounds();
        u32 w = std::ceil((bounds.r - bounds.l + SDF_RANGE * 2) * m_FontScale);
        u32 h = std::ceil((bounds.t - bounds.b + SDF_RANGE * 2) * m_FontScale);

        return ShapeData{
            .width = w,
            .height = h,
            .shape = shape,
            .offset = Vec2f(bounds.l, bounds.b) * m_FontScale,
        };
    }

    std::optional<Font::GlyphData> Font::GenerateGlyph(ShapeData& shape_data)
    {
        auto& [width, height, shape, offset] = shape_data;

        const auto bounds = shape.getBounds();

        msdfgen::edgeColoringSimple(shape, 3.0);
        msdfgen::Bitmap<float, 4> msdf(width, height);
        msdfgen::generateMTSDF(msdf, shape, { m_FontScale, { SDF_RANGE - bounds.l, SDF_RANGE - bounds.b } }, SDF_RANGE);

        GlyphData glyph_data;
        glyph_data.stride = shape_data.width * sizeof(float) * 4;
        glyph_data.bitmap = std::vector<u8>(reinterpret_cast<u8*>(msdf(0, 0)),
                                            reinterpret_cast<u8*>(msdf(0, 0)) + shape_data.height * glyph_data.stride);
        return glyph_data;
    }

    Font::DisplayData Font::ShapeText(std::string_view text, i32 direction)
    {
        m_FriBidi.logical.resize(text.size() * 2);
        m_FriBidi.visual.resize(text.size() * 2);

        FriBidiStrIndex len = fribidi_charset_to_unicode(FriBidiCharSet::FRIBIDI_CHAR_SET_UTF8,
                                                         text.data(),
                                                         static_cast<FriBidiStrIndex>(text.size()),
                                                         m_FriBidi.logical.data());
        FriBidiParType par_type = (direction == Direction::LTR) ? FRIBIDI_TYPE_LTR : FRIBIDI_TYPE_RTL;
        fribidi_log2vis(m_FriBidi.logical.data(), len, &par_type, m_FriBidi.visual.data(), nullptr, nullptr, nullptr);

        hb_buffer_clear_contents(m_HarfBuzz.buffer);
        hb_buffer_add_utf32(m_HarfBuzz.buffer, m_FriBidi.visual.data(), len, 0, len);

        hb_buffer_set_direction(m_HarfBuzz.buffer, HB_DIRECTION_LTR);
        hb_buffer_guess_segment_properties(m_HarfBuzz.buffer);

        hb_shape(m_HarfBuzz.font, m_HarfBuzz.buffer, nullptr, 0);

        u32 glyph_count;
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(m_HarfBuzz.buffer, &glyph_count);
        hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(m_HarfBuzz.buffer, &glyph_count);

        std::vector<ShapedGlyph> shapedGlyphs;
        shapedGlyphs.reserve(glyph_count);

        hb_position_t cursor_x = 0;
        hb_position_t cursor_y = 0;
        for (u32 i = 0; i < glyph_count; ++i)
        {
            const hb_codepoint_t glyph_index = glyph_info[i].codepoint;
            const hb_position_t x_offset = glyph_pos[i].x_offset;
            const hb_position_t y_offset = glyph_pos[i].y_offset;
            const hb_position_t x_advance = glyph_pos[i].x_advance;
            const hb_position_t y_advance = glyph_pos[i].y_advance;

            shapedGlyphs.push_back(ShapedGlyph{
                .glyph_index = msdfgen::GlyphIndex(glyph_index),
                .position = Vec2f(cursor_x + x_offset, cursor_y + y_offset) / m_HarfBuzz.scale,
            });

            cursor_x += x_advance;
            cursor_y += y_advance;
        }

        return { shapedGlyphs, Vec2f(cursor_x, cursor_y) / m_HarfBuzz.scale };
    }

}  // namespace Wraith