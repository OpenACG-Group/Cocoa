#include <cstdint>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <optional>

#include <cairo.h>
#include <cairo-script-interpreter.h>

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <string_view>
#include <vector>

namespace {

class HeapMemory
{
public:
    explicit HeapMemory(emscripten::val& heap_mem)
    {
        if (!heap_mem["__wasm_heap_mem"].as<bool>())
            throw std::runtime_error("Memory is not allocated from WASM heap");
        
        uintptr_t ptr = heap_mem["__wasm_heap_ptr"].as<uintptr_t>();
        heap_ptr_ = reinterpret_cast<uint8_t*>(ptr);
        heap_length_ = heap_mem["length"].as<size_t>();
    }

    ~HeapMemory() = default;

    inline size_t GetLength() const {
        return heap_length_;
    }

    inline void *GetPtr() const {
        return heap_ptr_;
    }

    inline uint8_t *GetU8Ptr() const {
        return heap_ptr_;
    }

private:
    uint8_t *heap_ptr_;
    size_t   heap_length_;
};

void check_status_or_throw(cairo_status_t status)
{
    // TODO(sora): throw with error information
    if (status != CAIRO_STATUS_SUCCESS)
        throw std::runtime_error("Cairo status is not SUCCESS");
}

class Surface : public std::enable_shared_from_this<Surface>
{
public:
    static const cairo_user_data_key_t kUserdataKey;

    explicit Surface(cairo_surface_t *surface)
        : surface_(surface)
    {
        cairo_surface_set_user_data(surface, &kUserdataKey, this, nullptr);
    }

    ~Surface()
    {
        cairo_surface_set_user_data(surface_, &kUserdataKey, nullptr, nullptr);
        cairo_surface_destroy(surface_);
    }

    inline cairo_surface_t *GetHandle() const {
        return surface_;
    }

    inline void TakeOwnership() {
        surface_ = cairo_surface_reference(surface_);
    }

#define SURFACE_IMPL_ARG0(verb)     void verb() { cairo_surface_##verb(surface_); }

    SURFACE_IMPL_ARG0(flush)
    SURFACE_IMPL_ARG0(finish)
    SURFACE_IMPL_ARG0(mark_dirty)

private:
    cairo_surface_t *surface_;
};
const cairo_user_data_key_t Surface::kUserdataKey{};

class ImageSurface : public Surface {
public:
    using Surface::Surface;

    cairo_format_t get_format()
    {
        return cairo_image_surface_get_format(GetHandle());
    };

    int get_width()
    {
        return cairo_image_surface_get_width(GetHandle());
    }

    int get_height()
    {
        return cairo_image_surface_get_height(GetHandle());
    }
};

class RecordingSurface : public Surface {
public:
    using Surface::Surface;

    emscripten::val get_extents()
    {
        cairo_rectangle_t rect;
        if (!cairo_recording_surface_get_extents(GetHandle(), &rect))
            return emscripten::val::null();
        return emscripten::val::array<double>({ rect.x, rect.y, rect.width, rect.height });
    }

    emscripten::val ink_extents()
    {
        double x0, y0, width, height;
        cairo_recording_surface_ink_extents(GetHandle(), &x0, &y0, &width, &height);
        return emscripten::val::array<double>({ x0, y0, width, height });
    }
};

std::shared_ptr<Surface> unwrap_surface(cairo_surface_t *ptr)
{
    void *data = cairo_surface_get_user_data(ptr, &Surface::kUserdataKey);
    if (!data)
        return nullptr;
    return reinterpret_cast<Surface*>(data)->shared_from_this();
}

std::shared_ptr<Surface> wrap_surface(cairo_surface_t *ptr)
{
    cairo_surface_type_t type = cairo_surface_get_type(ptr);
    switch (type)
    {
    case CAIRO_SURFACE_TYPE_IMAGE:
        return std::make_shared<ImageSurface>(ptr);

    case CAIRO_SURFACE_TYPE_RECORDING:
        return std::make_shared<RecordingSurface>(ptr);

    default:
        throw std::runtime_error("Unsupported surface type");
    }
}

std::shared_ptr<Surface> unwrap_or_wrap_surface(cairo_surface_t *ptr, bool keep_ref)
{
    std::shared_ptr<Surface> unwrap = unwrap_surface(ptr);
    if (unwrap)
        return unwrap;
    if (keep_ref)
        ptr = cairo_surface_reference(ptr);
    return wrap_surface(ptr);
}

class Pattern : std::enable_shared_from_this<Pattern>
{
public:
    static cairo_user_data_key_t kUserdataKey;

    static std::shared_ptr<Pattern> Unwrap(cairo_pattern_t *pattern)
    {
        void *data = cairo_pattern_get_user_data(pattern, &kUserdataKey);
        if (!data)
            return nullptr;
        return reinterpret_cast<Pattern*>(data)->shared_from_this();
    }

    explicit Pattern(cairo_pattern_t *pattern)
        : pattern_(pattern)
    {
        cairo_pattern_set_user_data(pattern_, &kUserdataKey, this, nullptr);
    }

    ~Pattern()
    {
        cairo_pattern_set_user_data(pattern_, &kUserdataKey, nullptr, nullptr);
        cairo_pattern_destroy(pattern_);
    }

    inline cairo_pattern_t *GetHandle() const {
        return pattern_;
    }

#define PATTERN_IMPL_ARG1(verb, type1) \
    void verb(type1 v1) { cairo_pattern_##verb(pattern_, v1); }
#define PATTERN_IMPL_ARG4(verb, type1, type2, type3, type4) \
    void verb(type1 v1, type2 v2, type3 v3, type4 v4) { cairo_pattern_##verb(pattern_, v1, v2, v3, v4); }
#define PATTERN_IMPL_ARG5(verb, type1, type2, type3, type4, type5) \
    void verb(type1 v1, type2 v2, type3 v3, type4 v4, type5 v5) { cairo_pattern_##verb(pattern_, v1, v2, v3, v4, v5); }

    PATTERN_IMPL_ARG4(add_color_stop_rgb, double, double, double, double)
    PATTERN_IMPL_ARG5(add_color_stop_rgba, double, double, double, double, double)

    int get_color_stop_count()
    {
        int count;
        check_status_or_throw(cairo_pattern_get_color_stop_count(pattern_, &count));
        return count;
    }

    std::shared_ptr<Surface> get_surface()
    {
        cairo_surface_t *surface;
        check_status_or_throw(cairo_pattern_get_surface(pattern_, &surface));
        return unwrap_surface(surface);
    }

#define PATTERN_MESH_OP0(verb) \
    void mesh_##verb() { cairo_mesh_pattern_##verb(pattern_); }
#define PATTERN_MESH_OP2(verb, type1, type2) \
    void mesh_##verb(type1 v1, type2 v2) { cairo_mesh_pattern_##verb(pattern_, v1, v2); }
#define PATTERN_MESH_OP3(verb, type1, type2, type3) \
    void mesh_##verb(type1 v1, type2 v2, type3 v3) { cairo_mesh_pattern_##verb(pattern_, v1, v2, v3); }
#define PATTERN_MESH_OP4(verb, type1, type2, type3, type4) \
    void mesh_##verb(type1 v1, type2 v2, type3 v3, type4 v4) { cairo_mesh_pattern_##verb(pattern_, v1, v2, v3, v4); }
#define PATTERN_MESH_OP5(verb, type1, type2, type3, type4, type5) \
    void mesh_##verb(type1 v1, type2 v2, type3 v3, type4 v4, type5 v5) { cairo_mesh_pattern_##verb(pattern_, v1, v2, v3, v4, v5); }
#define PATTERN_MESH_OP6_SAME(verb, type) \
    void mesh_##verb(type v1, type v2, type v3, type v4, type v5, type v6) { cairo_mesh_pattern_##verb(pattern_, v1, v2, v3, v4, v5, v6); }

    PATTERN_MESH_OP0(begin_patch)
    PATTERN_MESH_OP0(end_patch)
    PATTERN_MESH_OP2(move_to, double, double)
    PATTERN_MESH_OP2(line_to, double, double)
    PATTERN_MESH_OP6_SAME(curve_to, double)
    PATTERN_MESH_OP3(set_control_point, unsigned int, double, double)
    PATTERN_MESH_OP4(set_corner_color_rgb, unsigned int, double, double, double)
    PATTERN_MESH_OP5(set_corner_color_rgba, unsigned int, double, double, double, double)

    int mesh_get_patch_count()
    {
        unsigned int count;
        check_status_or_throw(cairo_mesh_pattern_get_patch_count(pattern_, &count));
        return count;
    }

    // TODO(sora): mesh_get_path(...)

    PATTERN_IMPL_ARG1(set_extend, cairo_extend_t)
    PATTERN_IMPL_ARG1(set_filter, cairo_filter_t)

    cairo_extend_t get_extend()
    {
        return cairo_pattern_get_extend(pattern_);
    }

    cairo_filter_t get_filter()
    {
        return cairo_pattern_get_filter(pattern_);
    }

    // TODO(sora): set_matrix(...) get_matrix()

    cairo_pattern_type_t get_type()
    {
        return cairo_pattern_get_type(pattern_);
    }

private:
    cairo_pattern_t *pattern_;
};
cairo_user_data_key_t Pattern::kUserdataKey{};

class Cairo
{
public:
    static std::shared_ptr<Cairo> Make(const std::shared_ptr<Surface>& surface)
    {
        cairo_t *cairo = cairo_create(surface->GetHandle());
        if (!cairo)
            return nullptr;
        return std::make_shared<Cairo>(cairo);
    }

    explicit Cairo(cairo_t *ptr) : cairo_(ptr) {}
    ~Cairo() {
        cairo_destroy(cairo_);
    }

    std::shared_ptr<Surface> get_target()
    {
        return unwrap_surface(cairo_get_target(cairo_));
    }

#define CTX_IMPL_GET(verb, type) type verb() { return cairo_##verb(cairo_); }

#define CTX_IMPL_ARG0(verb) void verb() { cairo_##verb(cairo_); }
#define CTX_IMPL_ARG1(verb, type1) void verb(type1 v1) { cairo_##verb(cairo_, v1); }
#define CTX_IMPL_ARG2(verb, type1, type2) \
    void verb(type1 v1, type2 v2) { cairo_##verb(cairo_, v1, v2); }
#define CTX_IMPL_ARG3(verb, type1, type2, type3) \
    void verb(type1 v1, type2 v2, type3 v3) { cairo_##verb(cairo_, v1, v2, v3); }
#define CTX_IMPL_ARG4(verb, type1, type2, type3, type4) \
    void verb(type1 v1, type2 v2, type3 v3, type4 v4) { cairo_##verb(cairo_, v1, v2, v3, v4); }
#define CTX_IMPL_ARG5(verb, type1, type2, type3, type4, type5) \
    void verb(type1 v1, type2 v2, type3 v3, type4 v4, type5 v5) { cairo_##verb(cairo_, v1, v2, v3, v4, v5); }
#define CTX_IMPL_ARG6(verb, type1, type2, type3, type4, type5, type6) \
    void verb(type1 v1, type2 v2, type3 v3, type4 v4, type5 v5, type6 v6) { cairo_##verb(cairo_, v1, v2, v3, v4, v5, v6); }

#define CTX_IMPL_EXTENTS(what)                                      \
    emscripten::val what##_extents()                                \
    {                                                               \
        double x1, y1, x2, y2;                                      \
        cairo_##what##_extents(cairo_, &x1, &y1, &x2, &y2);         \
        return emscripten::val::array<double>({x1, y1, x2, y2});    \
    }

#define CTX_IMPL_INSIDENESS_TEST(what)          \
    bool in_##what(double x, double y)          \
    {                                           \
        return cairo_in_##what(cairo_, x, y);   \
    }


    CTX_IMPL_ARG0(save)
    CTX_IMPL_ARG0(restore)
    CTX_IMPL_ARG0(push_group)
    CTX_IMPL_ARG1(push_group_with_content, cairo_content_t)

    std::shared_ptr<Pattern> pop_group()
    {
        return std::make_shared<Pattern>(cairo_pop_group(cairo_));
    }

    CTX_IMPL_ARG0(pop_group_to_source)

    std::shared_ptr<Surface> get_group_target()
    {
        cairo_surface_t *target = cairo_get_group_target(cairo_);
        if (!target)
            return nullptr;
        return unwrap_or_wrap_surface(target, true);
    }

    CTX_IMPL_ARG3(set_source_rgb, double, double, double)
    CTX_IMPL_ARG4(set_source_rgba, double, double, double, double)

    void set_source(std::shared_ptr<Pattern> source)
    {
        cairo_set_source(cairo_, source->GetHandle());
    }

    std::shared_ptr<Pattern> get_source()
    {
        cairo_pattern_t *pattern = cairo_get_source(cairo_);
        auto sp = Pattern::Unwrap(pattern);
        if (!sp)
        {
            pattern = cairo_pattern_reference(pattern);
            sp = std::make_shared<Pattern>(pattern);
        }
        return sp;
    }

    void set_source_surface(std::shared_ptr<Surface> s, double x, double y)
    {
        cairo_set_source_surface(cairo_, s->GetHandle(), x, y);
    }

    CTX_IMPL_ARG1(set_antialias, cairo_antialias_t)
    CTX_IMPL_GET(get_antialias, cairo_antialias_t)
    
    void set_dash(emscripten::val dashes, double offset)
    {
        if (!dashes.isArray())
            throw std::runtime_error("Argument `dashes` must be an array of numbers");
        auto vdashes = emscripten::convertJSArrayToNumberVector<double>(dashes);
        cairo_set_dash(cairo_, vdashes.data(), vdashes.size(), offset);
    }

    CTX_IMPL_GET(get_dash_count, int)

    CTX_IMPL_ARG1(set_fill_rule, cairo_fill_rule_t);
    CTX_IMPL_GET(get_fill_rule, cairo_fill_rule_t)

    CTX_IMPL_ARG1(set_line_cap, cairo_line_cap_t)
    CTX_IMPL_GET(get_line_cap, cairo_line_cap_t)

    CTX_IMPL_ARG1(set_line_join, cairo_line_join_t)
    CTX_IMPL_GET(get_line_join, cairo_line_join_t)

    CTX_IMPL_ARG1(set_line_width, double)
    CTX_IMPL_GET(get_line_width, double)

    CTX_IMPL_ARG1(set_miter_limit, double)
    CTX_IMPL_GET(get_miter_limit, double)

    CTX_IMPL_ARG1(set_operator, cairo_operator_t)
    CTX_IMPL_GET(get_operator, cairo_operator_t)

    CTX_IMPL_ARG1(set_tolerance, double)
    CTX_IMPL_GET(get_tolerance, double)

    CTX_IMPL_ARG0(clip)
    CTX_IMPL_ARG0(clip_preserve)
    CTX_IMPL_ARG0(reset_clip)
    CTX_IMPL_EXTENTS(clip)
    CTX_IMPL_INSIDENESS_TEST(clip)

    CTX_IMPL_ARG0(fill)
    CTX_IMPL_ARG0(fill_preserve)
    CTX_IMPL_EXTENTS(fill)
    CTX_IMPL_INSIDENESS_TEST(fill)

    void mask(std::shared_ptr<Pattern> pattern)
    {
        cairo_mask(cairo_, pattern->GetHandle());
    }

    void mask_surface(std::shared_ptr<Surface> s, double x, double y)
    {
        cairo_mask_surface(cairo_, s->GetHandle(), x, y);
    }

    CTX_IMPL_ARG0(paint)
    CTX_IMPL_ARG1(paint_with_alpha, double)

    CTX_IMPL_ARG0(stroke)
    CTX_IMPL_ARG0(stroke_preserve)
    CTX_IMPL_EXTENTS(stroke)
    CTX_IMPL_INSIDENESS_TEST(stroke)

    CTX_IMPL_ARG0(copy_page)
    CTX_IMPL_ARG0(show_page)

    CTX_IMPL_ARG2(translate, double, double)
    CTX_IMPL_ARG2(scale, double, double)
    CTX_IMPL_ARG1(rotate, double)

    // TODO(sora): transform, set_matrix

    CTX_IMPL_ARG0(identity_matrix)

    CTX_IMPL_ARG0(new_path)
    CTX_IMPL_ARG2(move_to, double, double)
    CTX_IMPL_ARG0(new_sub_path)
    CTX_IMPL_ARG2(line_to, double, double)
    CTX_IMPL_ARG6(curve_to, double, double, double, double, double, double)
    CTX_IMPL_ARG5(arc, double, double, double, double, double)
    CTX_IMPL_ARG5(arc_negative, double, double, double, double, double)
    CTX_IMPL_ARG2(rel_move_to, double, double)
    CTX_IMPL_ARG2(rel_line_to, double, double)
    CTX_IMPL_ARG6(rel_curve_to, double, double, double, double, double, double)
    CTX_IMPL_ARG4(rectangle, double, double, double, double)
    CTX_IMPL_ARG0(close_path)
    CTX_IMPL_EXTENTS(path)

    void tag_begin(const std::string& name, const std::string& attr)
    {
        cairo_tag_begin(cairo_, name.c_str(), attr.c_str());
    }

    void tag_end(const std::string& name)
    {
        cairo_tag_end(cairo_, name.c_str());
    }

private:
    cairo_t *cairo_;
};


class ScriptInterpreter
{
public:
    explicit ScriptInterpreter(cairo_script_interpreter_t *csi)
        : csi_(csi)
        , create_surface_func_(emscripten::val::null())
    {
        cairo_script_interpreter_hooks_t hooks{
            .closure = this,
            .surface_create = hook_surface_create
        };
        cairo_script_interpreter_install_hooks(csi_, &hooks);
    }

    static cairo_surface_t *hook_surface_create(void *closure,
                                                cairo_content_t content,
                                                double width, double height, long uid)
    {
        auto *self = reinterpret_cast<ScriptInterpreter*>(closure);
        if (self->create_surface_func_.isNull() || self->create_surface_func_.isUndefined())
            return nullptr;
        emscripten::val ret = self->create_surface_func_(content, width, height, uid);
        auto ptr = ret.as<std::shared_ptr<Surface>>();
        ptr->TakeOwnership();
        return ptr->GetHandle();
    }

    ~ScriptInterpreter()
    {
        cairo_script_interpreter_destroy(csi_);
    }

    cairo_status_t feed_string(const std::string& source)
    {
        return cairo_script_interpreter_feed_string(csi_, source.c_str(), source.length());
    }

    cairo_status_t finish()
    {
        return cairo_script_interpreter_finish(csi_);
    }

    void install_hooks(emscripten::val hooks)
    {
        emscripten::val prop = hooks["surface_create"];
        if (prop.typeOf().as<std::string>() != "function")
            throw std::runtime_error("Property `surface_create` is not a function");
        create_surface_func_ = prop;
    }

private:
    cairo_script_interpreter_t *csi_;
    emscripten::val create_surface_func_;
};

} // namespace anonymous

EMSCRIPTEN_BINDINGS(Cairo)
{
    using namespace emscripten;

    enum_<cairo_status_t>("Status")
        .value("SUCCESS", CAIRO_STATUS_SUCCESS)
        .value("NO_MEMORY", CAIRO_STATUS_NO_MEMORY)
        .value("INVALID_RESTORE", CAIRO_STATUS_INVALID_RESTORE)
        .value("INVALID_POP_GROUP", CAIRO_STATUS_INVALID_POP_GROUP)
        .value("NO_CURRENT_POINT", CAIRO_STATUS_NO_CURRENT_POINT)
        .value("INVALID_MATRIX", CAIRO_STATUS_INVALID_MATRIX)
        .value("INVALID_STATUS", CAIRO_STATUS_INVALID_STATUS)
        .value("NULL_POINTER", CAIRO_STATUS_NULL_POINTER)
        .value("INVALID_STRING", CAIRO_STATUS_INVALID_STRING)
        .value("INVALID_PATH_DATA", CAIRO_STATUS_INVALID_PATH_DATA)
        .value("READ_ERROR", CAIRO_STATUS_READ_ERROR)
        .value("WRITE_ERROR", CAIRO_STATUS_WRITE_ERROR)
        .value("SURFACE_FINISHED", CAIRO_STATUS_SURFACE_FINISHED)
        .value("SURFACE_TYPE_MISMATCH", CAIRO_STATUS_SURFACE_TYPE_MISMATCH)
        .value("PATTERN_TYPE_MISMATCH", CAIRO_STATUS_PATTERN_TYPE_MISMATCH)
        .value("INVALID_CONTENT", CAIRO_STATUS_INVALID_CONTENT)
        .value("INVALID_FORMAT", CAIRO_STATUS_INVALID_FORMAT)
        .value("INVALID_VISUAL", CAIRO_STATUS_INVALID_VISUAL)
        .value("FILE_NOT_FOUND", CAIRO_STATUS_FILE_NOT_FOUND)
        .value("INVALID_DASH", CAIRO_STATUS_INVALID_DASH)
        .value("INVALID_DSC_COMMENT", CAIRO_STATUS_INVALID_DSC_COMMENT)
        .value("INVALID_INDEX", CAIRO_STATUS_INVALID_INDEX)
        .value("CLIP_NOT_REPRESENTABLE", CAIRO_STATUS_CLIP_NOT_REPRESENTABLE)
        .value("TEMP_FILE_ERROR", CAIRO_STATUS_TEMP_FILE_ERROR)
        .value("INVALID_STRIDE", CAIRO_STATUS_INVALID_STRIDE)
        .value("FONT_TYPE_MISMATCH", CAIRO_STATUS_FONT_TYPE_MISMATCH)
        .value("USER_FONT_IMMUTABLE", CAIRO_STATUS_USER_FONT_IMMUTABLE)
        .value("USER_FONT_ERROR", CAIRO_STATUS_USER_FONT_ERROR)
        .value("NEGATIVE_COUNT", CAIRO_STATUS_NEGATIVE_COUNT)
        .value("INVALID_CLUSTERS", CAIRO_STATUS_INVALID_CLUSTERS)
        .value("INVALID_SLANT", CAIRO_STATUS_INVALID_SLANT)
        .value("INVALID_WEIGHT", CAIRO_STATUS_INVALID_WEIGHT)
        .value("INVALID_SIZE", CAIRO_STATUS_INVALID_SIZE)
        .value("USER_FONT_NOT_IMPLEMENTED", CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED)
        .value("DEVICE_TYPE_MISMATCH", CAIRO_STATUS_DEVICE_TYPE_MISMATCH)
        .value("DEVICE_ERROR", CAIRO_STATUS_DEVICE_ERROR)
        .value("INVALID_MESH_CONSTRUCTION", CAIRO_STATUS_INVALID_MESH_CONSTRUCTION)
        .value("DEVICE_FINISHED", CAIRO_STATUS_DEVICE_FINISHED)
        .value("JBIG2_GLOBAL_MISSING", CAIRO_STATUS_JBIG2_GLOBAL_MISSING)
        .value("PNG_ERROR", CAIRO_STATUS_PNG_ERROR)
        .value("FREETYPE_ERROR", CAIRO_STATUS_FREETYPE_ERROR)
        .value("WIN32_GDI_ERROR", CAIRO_STATUS_WIN32_GDI_ERROR)
        .value("TAG_ERROR", CAIRO_STATUS_TAG_ERROR)
        .value("DWRITE_ERROR", CAIRO_STATUS_DWRITE_ERROR)
        .value("SVG_FONT_ERROR", CAIRO_STATUS_SVG_FONT_ERROR);

    enum_<cairo_format_t>("Format")
        .value("INVALID", CAIRO_FORMAT_INVALID)
        .value("ARGB32", CAIRO_FORMAT_ARGB32)
        .value("RGB24", CAIRO_FORMAT_RGB24)
        .value("A8", CAIRO_FORMAT_A8)
        .value("A1", CAIRO_FORMAT_A1)
        .value("RGB16_565", CAIRO_FORMAT_RGB16_565)
        .value("RGB30", CAIRO_FORMAT_RGB30)
        .value("RGB96F", CAIRO_FORMAT_RGB96F)
        .value("RGBA128F", CAIRO_FORMAT_RGBA128F);
    
    enum_<cairo_content_t>("Content")
        .value("ALPHA", CAIRO_CONTENT_ALPHA)
        .value("COLOR", CAIRO_CONTENT_COLOR)
        .value("COLOR_ALPHA", CAIRO_CONTENT_COLOR_ALPHA);
    
    enum_<cairo_antialias_t>("Antialias")
        .value("DEFAULT", CAIRO_ANTIALIAS_DEFAULT)
        .value("NONE", CAIRO_ANTIALIAS_NONE)
        .value("GRAY", CAIRO_ANTIALIAS_GRAY)
        .value("SUBPIXEL", CAIRO_ANTIALIAS_SUBPIXEL)
        .value("FAST", CAIRO_ANTIALIAS_FAST)
        .value("GOOD", CAIRO_ANTIALIAS_GOOD)
        .value("BEST", CAIRO_ANTIALIAS_BEST);

    enum_<cairo_fill_rule_t>("FillRule")
        .value("EVEN_ODD", CAIRO_FILL_RULE_EVEN_ODD)
        .value("WINDING", CAIRO_FILL_RULE_WINDING);

    enum_<cairo_line_cap_t>("LineCap")
        .value("BUTT", CAIRO_LINE_CAP_BUTT)
        .value("SQUARE", CAIRO_LINE_CAP_SQUARE)
        .value("ROUND", CAIRO_LINE_CAP_ROUND);

    enum_<cairo_line_join_t>("LineJoin")
        .value("BEVEL", CAIRO_LINE_JOIN_BEVEL)
        .value("MITER", CAIRO_LINE_JOIN_MITER)
        .value("ROUND", CAIRO_LINE_JOIN_ROUND);
    
    enum_<cairo_operator_t>("Operator")
        .value("CLEAR", CAIRO_OPERATOR_CLEAR)
        .value("SOURCE", CAIRO_OPERATOR_SOURCE)
        .value("OVER", CAIRO_OPERATOR_OVER)
        .value("IN", CAIRO_OPERATOR_IN)
        .value("OUT", CAIRO_OPERATOR_OUT)
        .value("ATOP", CAIRO_OPERATOR_ATOP)
        .value("DEST", CAIRO_OPERATOR_DEST)
        .value("DEST_OVER", CAIRO_OPERATOR_DEST_OVER)
        .value("DEST_IN", CAIRO_OPERATOR_DEST_IN)
        .value("DEST_OUT", CAIRO_OPERATOR_DEST_OUT)
        .value("DEST_ATOP", CAIRO_OPERATOR_DEST_ATOP)
        .value("XOR", CAIRO_OPERATOR_XOR)
        .value("ADD", CAIRO_OPERATOR_ADD)
        .value("SATURATE", CAIRO_OPERATOR_SATURATE)
        .value("MULTIPLY", CAIRO_OPERATOR_MULTIPLY)
        .value("SCREEN", CAIRO_OPERATOR_SCREEN)
        .value("OVERLAY", CAIRO_OPERATOR_OVERLAY)
        .value("DARKEN", CAIRO_OPERATOR_DARKEN)
        .value("LIGHTEN", CAIRO_OPERATOR_LIGHTEN)
        .value("COLOR_DODGE", CAIRO_OPERATOR_COLOR_DODGE)
        .value("COLOR_BURN", CAIRO_OPERATOR_COLOR_BURN)
        .value("HARD_LIGHT", CAIRO_OPERATOR_HARD_LIGHT)
        .value("SOFT_LIGHT", CAIRO_OPERATOR_SOFT_LIGHT)
        .value("DIFFERENCE", CAIRO_OPERATOR_DIFFERENCE)
        .value("EXCLUSION", CAIRO_OPERATOR_EXCLUSION)
        .value("HSL_HUE", CAIRO_OPERATOR_HSL_HUE)
        .value("HSL_SATURATION", CAIRO_OPERATOR_HSL_SATURATION)
        .value("HSL_COLOR", CAIRO_OPERATOR_HSL_COLOR)
        .value("HSL_LUMINOSITY", CAIRO_OPERATOR_HSL_LUMINOSITY);

    enum_<cairo_extend_t>("Extend")
        .value("NONE", CAIRO_EXTEND_NONE)
        .value("REPEAT", CAIRO_EXTEND_REPEAT)
        .value("REFLECT", CAIRO_EXTEND_REFLECT)
        .value("PAD", CAIRO_EXTEND_PAD);

    enum_<cairo_filter_t>("Filter")
        .value("FAST", CAIRO_FILTER_BEST)
        .value("GOOD", CAIRO_FILTER_GOOD)
        .value("BEST", CAIRO_FILTER_BEST)
        .value("NEAREST", CAIRO_FILTER_NEAREST)
        .value("BILINEAR", CAIRO_FILTER_BILINEAR)
        .value("GAUSSIAN", CAIRO_FILTER_GAUSSIAN);

    enum_<cairo_pattern_type_t>("PatternType")
        .value("SOLID", CAIRO_PATTERN_TYPE_SOLID)
        .value("SURFACE", CAIRO_PATTERN_TYPE_SURFACE)
        .value("LINEAR", CAIRO_PATTERN_TYPE_LINEAR)
        .value("RADIAL", CAIRO_PATTERN_TYPE_RADIAL)
        .value("MESH", CAIRO_PATTERN_TYPE_MESH)
        .value("RASTER_SOURCE", CAIRO_PATTERN_TYPE_RASTER_SOURCE);

    function("image_surface_create", optional_override(
        [](int width, int height, emscripten::val memory,
           cairo_format_t format, int stride) -> std::shared_ptr<ImageSurface>
        {
            HeapMemory heap_memory(memory);
            cairo_surface_t *surface = cairo_image_surface_create_for_data(
                heap_memory.GetU8Ptr(), format, width, height, stride);
            if (surface == nullptr)
                return nullptr;
            
            if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
                return nullptr;

            return std::make_shared<ImageSurface>(surface);
        }
    ));

    function("recording_surface_create", optional_override(
        [](cairo_content_t content, emscripten::val extents) -> std::shared_ptr<RecordingSurface>
        {
            std::optional<cairo_rectangle_t> rect;
            if (!extents.isNull())
            {
                if (!extents.isArray() || extents["length"].as<size_t>() != 4)
                    return nullptr;
                rect = cairo_rectangle_t{};
                rect->x = extents[0].as<double>();
                rect->y = extents[1].as<double>();
                rect->width = extents[2].as<double>();
                rect->height = extents[3].as<double>();
            }
            cairo_surface_t *surface = cairo_recording_surface_create(
                content, rect ? &(*rect) : nullptr);
            if (!surface)
                return nullptr;
            return std::make_shared<RecordingSurface>(surface);
        }
    ));

    function("cairo_create", &Cairo::Make);
    
    function("pattern_create_rgb", optional_override(
        [](double r, double g, double b) -> std::shared_ptr<Pattern> {
            cairo_pattern_t *pattern = cairo_pattern_create_rgb(r, g, b);
            if (!pattern || cairo_pattern_status(pattern) != CAIRO_STATUS_SUCCESS)
                return nullptr;
            return std::make_shared<Pattern>(pattern);
        }
    ));

    function("pattern_create_rgba", optional_override(
        [](double r, double g, double b, double a) -> std::shared_ptr<Pattern> {
            cairo_pattern_t *pattern = cairo_pattern_create_rgba(r, g, b, a);
            if (!pattern || cairo_pattern_status(pattern) != CAIRO_STATUS_SUCCESS)
                return nullptr;
            return std::make_shared<Pattern>(pattern);
        }
    ));

    function("pattern_create_for_surface", optional_override(
        [](std::shared_ptr<Surface> surface) -> std::shared_ptr<Pattern> {
            cairo_pattern_t *pattern = cairo_pattern_create_for_surface(surface->GetHandle());
            if (!pattern || cairo_pattern_status(pattern) != CAIRO_STATUS_SUCCESS)
                return nullptr;
            return std::make_shared<Pattern>(pattern);
        }
    ));

    function("pattern_create_linear", optional_override(
        [](double x0, double y0, double x1, double y1) -> std::shared_ptr<Pattern> {
            cairo_pattern_t *pattern = cairo_pattern_create_linear(x0, y0, x1, y1);
            if (!pattern || cairo_pattern_status(pattern) != CAIRO_STATUS_SUCCESS)
                return nullptr;
            return std::make_shared<Pattern>(pattern);
        }
    ));

    function("pattern_create_radial", optional_override(
        [](double cx0, double cy0, double radius0, double cx1, double cy1, double radius1) -> std::shared_ptr<Pattern> {
            cairo_pattern_t *pattern = cairo_pattern_create_radial(cx0, cy0, radius0, cx1, cy1, radius1);
            if (!pattern || cairo_pattern_status(pattern) != CAIRO_STATUS_SUCCESS)
                return nullptr;
            return std::make_shared<Pattern>(pattern);
        }
    ));

    function("pattern_create_mesh", optional_override(
        []() -> std::shared_ptr<Pattern> {
            cairo_pattern_t *pattern = cairo_pattern_create_mesh();
            if (!pattern || cairo_pattern_status(pattern) != CAIRO_STATUS_SUCCESS)
                return nullptr;
            return std::make_shared<Pattern>(pattern);
        }
    ));

    class_<Surface>("Surface")
        .smart_ptr<std::shared_ptr<Surface>>("std::shared_ptr<Surface>")
        .function("flush", &Surface::flush)
        .function("finish", &Surface::finish)
        .function("mark_dirty", &Surface::mark_dirty);
    
    class_<ImageSurface, base<Surface>>("ImageSurface")
        .smart_ptr<std::shared_ptr<ImageSurface>>("std::shared_ptr<ImageSurface>")
        .function("get_format", &ImageSurface::get_format)
        .function("get_width", &ImageSurface::get_width)
        .function("get_height", &ImageSurface::get_height);

    class_<RecordingSurface, base<Surface>>("RecordingSurface")
        .smart_ptr<std::shared_ptr<RecordingSurface>>("std::shared_ptr<RecordingSurface>")
        .function("ink_extents", &RecordingSurface::ink_extents)
        .function("get_extents", &RecordingSurface::get_extents);

    class_<Pattern>("Pattern")
        .smart_ptr<std::shared_ptr<Pattern>>("std::shared_ptr<Pattern>")
        .function("add_color_stop_rgb", &Pattern::add_color_stop_rgb)
        .function("add_color_stop_rgba", &Pattern::add_color_stop_rgba)
        .function("get_color_stop_count", &Pattern::get_color_stop_count)
        .function("get_surface", &Pattern::get_surface)
        .function("mesh_begin_patch", &Pattern::mesh_begin_patch)
        .function("mesh_end_patch", &Pattern::mesh_end_patch)
        .function("mesh_move_to", &Pattern::mesh_move_to)
        .function("mesh_line_to", &Pattern::mesh_line_to)
        .function("mesh_curve_to", &Pattern::mesh_curve_to)
        .function("mesh_set_control_point", &Pattern::mesh_set_control_point)
        .function("mesh_set_corner_color_rgb", &Pattern::mesh_set_corner_color_rgb)
        .function("mesh_set_corner_color_rgba", &Pattern::mesh_set_corner_color_rgba)
        .function("mesh_get_patch_count", &Pattern::mesh_get_patch_count)
        .function("set_extend", &Pattern::set_extend)
        .function("set_filter", &Pattern::set_filter)
        .function("get_extend", &Pattern::get_extend)
        .function("get_filter", &Pattern::get_filter)
        .function("get_type", &Pattern::get_type);

    class_<Cairo>("Cairo")
        .smart_ptr<std::shared_ptr<Cairo>>("std::shared_ptr<Cairo>")
        .function("get_target", &Cairo::get_target)
        .function("save", &Cairo::save)
        .function("restore", &Cairo::restore)
        .function("push_group", &Cairo::push_group)
        .function("push_group_with_content", &Cairo::push_group_with_content)
        .function("pop_group", &Cairo::pop_group)
        .function("pop_group_to_source", &Cairo::pop_group_to_source)
        .function("get_group_target", &Cairo::get_group_target)
        .function("set_source_rgb", &Cairo::set_source_rgb)
        .function("set_source_rgba", &Cairo::set_source_rgba)
        .function("set_source_surface", &Cairo::set_source_surface)
        .function("set_antialias", &Cairo::set_antialias)
        .function("get_antialias", &Cairo::get_antialias)
        .function("set_dash", &Cairo::set_dash)
        .function("get_dash_count", &Cairo::get_dash_count)
        .function("set_fill_rule", &Cairo::set_fill_rule)
        .function("get_fill_rule", &Cairo::get_fill_rule)
        .function("set_line_cap", &Cairo::set_line_cap)
        .function("get_line_cap", &Cairo::get_line_cap)
        .function("set_line_join", &Cairo::set_line_join)
        .function("get_line_join", &Cairo::get_line_join)
        .function("set_line_width", &Cairo::set_line_width)
        .function("get_line_width", &Cairo::get_line_width)
        .function("set_miter_limit", &Cairo::set_miter_limit)
        .function("get_miter_limit", &Cairo::get_miter_limit)
        .function("set_operator", &Cairo::set_operator)
        .function("get_operator", &Cairo::get_operator)
        .function("set_tolerance", &Cairo::set_tolerance)
        .function("get_tolerance", &Cairo::get_tolerance)
        .function("clip", &Cairo::clip)
        .function("clip_preserve", &Cairo::clip_preserve)
        .function("clip_extents", &Cairo::clip_extents)
        .function("in_clip", &Cairo::in_clip)
        .function("reset_clip", &Cairo::reset_clip)
        .function("fill", &Cairo::fill)
        .function("fill_preserve", &Cairo::fill_preserve)
        .function("fill_extents", &Cairo::fill_extents)
        .function("in_fill", &Cairo::in_fill)
        .function("mask_surface", &Cairo::mask_surface)
        .function("paint", &Cairo::paint)
        .function("paint_with_alpha", &Cairo::paint_with_alpha)
        .function("stroke", &Cairo::stroke)
        .function("stroke_preserve", &Cairo::stroke_preserve)
        .function("stroke_extents", &Cairo::stroke_extents)
        .function("in_stroke", &Cairo::in_stroke)
        .function("copy_page", &Cairo::copy_page)
        .function("show_page", &Cairo::show_page)
        .function("translate", &Cairo::translate)
        .function("scale", &Cairo::scale)
        .function("rotate", &Cairo::rotate)
        .function("identity_matrix", &Cairo::identity_matrix)
        .function("new_path", &Cairo::new_path)
        .function("move_to", &Cairo::move_to)
        .function("new_sub_path", &Cairo::new_sub_path)
        .function("line_to", &Cairo::line_to)
        .function("curve_to", &Cairo::curve_to)
        .function("arc", &Cairo::arc)
        .function("arc_negative", &Cairo::arc_negative)
        .function("rel_move_to", &Cairo::rel_move_to)
        .function("rel_line_to", &Cairo::rel_line_to)
        .function("rel_curve_to", &Cairo::rel_curve_to)
        .function("rectangle", &Cairo::rectangle)
        .function("close_path", &Cairo::close_path)
        .function("path_extents", &Cairo::path_extents)
        .function("tag_begin", &Cairo::tag_begin)
        .function("tag_end", &Cairo::tag_end)
        .function("set_source", &Cairo::set_source)
        .function("get_source", &Cairo::get_source)
        .function("mask", &Cairo::mask);

    function("script_interpreter_create", optional_override(
        []() -> std::shared_ptr<ScriptInterpreter>
        {
            return std::make_shared<ScriptInterpreter>(
                cairo_script_interpreter_create()
            );
        }
    ));

    class_<ScriptInterpreter>("ScriptInterpreter")
        .smart_ptr<std::shared_ptr<ScriptInterpreter>>("std::shared_ptr<ScriptInterpreter>")
        .function("feed_string", &ScriptInterpreter::feed_string)
        .function("finish", &ScriptInterpreter::finish)
        .function("install_hooks", &ScriptInterpreter::install_hooks);
}
