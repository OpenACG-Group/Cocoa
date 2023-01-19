/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include <list>

#include "include/core/SkPoint3.h"

#include "Gallium/bindings/glamor/EffectDSLParser.h"
#include "Gallium/bindings/glamor/CkPathWrap.h"
#include "Gallium/bindings/glamor/CkPathEffectWrap.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

struct Location
{
    int32_t line;
    int32_t column;

    bool operator==(const Location& loc) const {
        return (loc.line == line && loc.column == column);
    }

    bool operator!=(const Location& loc) const {
        return (loc.line != line || loc.column != column);
    }
};

struct Token
{
    enum TokenType
    {
        kInteger,
        kFloat,
        kIdentifier,    // identifiers
        kLPar,          // (
        kRPar,          // )
        kLBracket,      // [
        kRBracket,      // ]
        kComma,         // ,
        kReplacement,   // %something
        kNull,          // null replacement '_'
        kEOF
    };

    explicit operator bool() const {
        return (type != kEOF);
    }

    TokenType           type;
    Location            location;
    std::string         lexeme;
    double              double_value;
    int32_t             integer_value;
};

using TokenList = std::list<Token>;

std::string_view get_string_line_view(const std::string_view& source,
                                      int line)
{
    CHECK(line >= 1);

    int skip_count = line - 1;
    size_t start_offset = 0;
    while (skip_count > 0 && start_offset < source.length())
    {
        if (source[start_offset] == '\n')
            skip_count--;
        start_offset++;
    }
    CHECK(start_offset < source.length());

    std::string_view view(source);
    view.remove_prefix(start_offset);

    auto first_ln_pos = view.find_first_of('\n');
    if (first_ln_pos != std::string_view::npos)
        view.remove_suffix(view.length() - first_ln_pos);

    return view;
}

g_noreturn void report_syntax_error(const std::string& error,
                                    const std::string& source,
                                    const Location& loc)
{
    // Error report format:
    // Syntax error at <line>:<column>: <error>
    //     <source code (the line where error occurred)>
    //             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ (underline to emphasize)

    std::ostringstream oss;
    oss << fmt::format("Syntax error at {}:{}: {}\n", loc.line,
                       loc.column, error);

    std::string_view line_view(get_string_line_view(source, loc.line));
    oss << fmt::format("    {}\n", line_view);

    oss << std::string(loc.column + 4 - 1, ' ') << '^';
    oss << std::string(line_view.length() - loc.column, '~');

    g_throw(Error, oss.str());
}

bool is_identifier_char(char ch)
{
    return std::isalpha(ch) || std::isdigit(ch) || ch == '_';
}

class RefStringLexer
{
public:
    explicit RefStringLexer(const std::string& input)
            : input_(input) , pos_(-1), current_loc_{1, 0} {}
    ~RefStringLexer() = default;

    char PeekNext()
    {
        CHECK(pos_ < static_cast<ssize_t>(input_.length()));
        return input_[pos_ + 1];
    }

    char StepForward()
    {
        CHECK(pos_ < static_cast<ssize_t>(input_.length()));
        current_loc_.column++;
        return input_[++pos_];
    }

    Token ScanNext()
    {
        static std::map<char, Token::TokenType> fast_types_map = {
                { '\0', Token::kEOF         },
                { '(',  Token::kLPar        },
                { ')',  Token::kRPar        },
                { '[',  Token::kLBracket    },
                { ']',  Token::kRBracket    },
                { ',',  Token::kComma       }
        };

        char peek = PeekNext();
        // Skip whitespaces
        while (peek == ' ' || peek == '\n' || peek == '\t')
        {
            if (peek == '\n')
            {
                current_loc_.line++;
                // treat `\n` as the 0th column in the next line
                current_loc_.column = 0;
            }
            StepForward();
            peek = PeekNext();
        }

        if (fast_types_map.count(peek) > 0)
        {
            StepForward();
            return Token{fast_types_map[peek], current_loc_};
        }

        // Recognize replacement
        Location start_location = current_loc_;

        if (peek == '%')
        {
            std::string lexeme;
            StepForward();
            peek = PeekNext();
            while (is_identifier_char(peek))
            {
                StepForward();
                lexeme.push_back(peek);
                peek = PeekNext();
            }

            return Token{Token::kReplacement, start_location, lexeme};
        }

        // Recognize numbers
        int32_t v_int = 0;
        int32_t float_scale = 1;
        bool is_float = false;
        int32_t neg_flag = 1;
        while (std::isdigit(peek) || peek == '-')
        {
            if (peek == '-')
            {
                if (neg_flag < 0 || v_int != 0)
                    report_syntax_error("Unrecognized character", input_, current_loc_);
                neg_flag = -1;
                StepForward();
                peek = PeekNext();
                continue;
            }

            if (is_float)
                float_scale *= 10;

            v_int = v_int * 10 + (peek - '0');

            StepForward();
            peek = PeekNext();
            if (peek == '.')
            {
                if (is_float)
                {
                    report_syntax_error("Invalid decimal number, unexpected '.'",
                                        input_, start_location);
                }

                is_float = true;
                StepForward();
                peek = PeekNext();
            }
        }

        if (is_float && float_scale == 1)
        {
            report_syntax_error("Invalid decimal number, expecting floating part",
                                input_, start_location);
        }

        if (start_location != current_loc_)
        {
            return Token{is_float ? Token::kFloat : Token::kInteger,
                         start_location,
                         "",
                         neg_flag * static_cast<double>(v_int) / float_scale,
                         neg_flag * v_int};
        }

        // Recognize identifier
        peek = PeekNext();
        if (!std::isalpha(peek) && peek != '_')
        {
            report_syntax_error("Unrecognized character",
                                input_, current_loc_);
        }

        std::string lexeme;
        while (is_identifier_char(peek))
        {
            StepForward();
            lexeme.push_back(peek);
            peek = PeekNext();
        }

        return Token{lexeme == "_" ? Token::kNull : Token::kIdentifier,
                     start_location, lexeme};
    }

    static TokenList Tokenize(const std::string& input)
    {
        RefStringLexer lexer(input);

        TokenList list;
        Token token;
        do
        {
            token = lexer.ScanNext();
            list.emplace_back(token);
        } while (token);

        return list;
    }

private:
    const std::string&      input_;
    ssize_t                 pos_;
    Location                current_loc_;
};

std::unordered_map<Token::TokenType, std::string_view> g_toktype_names_map = {
        { Token::kLPar,        "'('"   },
        { Token::kRPar,        "')'"   },
        { Token::kLBracket,    "'['"   },
        { Token::kRBracket,    "']'"   },
        { Token::kComma,       "','"   },
        { Token::kInteger,     "Integer"   },
        { Token::kFloat,       "Float" },
        { Token::kIdentifier,  "Identifier" },
        { Token::kReplacement, "Argument"   },
        { Token::kNull,        "Null" },
        { Token::kEOF,         "<EOF>" }
};

class Parser
{
public:
    explicit Parser(const std::string& str,
                    EffectStackOperand::KWArgsMap args_map,
                    EffectDSLParser::EffectorBuildersMap& builders_map)
            : source_(str)
            , token_list_(RefStringLexer::Tokenize(str))
            , current_itr_(token_list_.begin())
            , kwargs_map_(std::move(args_map))
            , builders_map_(builders_map)
    {
    }

    // effector := IDENTIFIER '(' expr ',' expr ... ')'
    // NOLINTNEXTLINE
    Effector ParseEffector()
    {
        std::string filter_name;

        AssertCurrentTokenIs(Token::kIdentifier);
        filter_name = current_itr_->lexeme;

        // Eats the identifier
        current_itr_++;

        AssertCurrentTokenIs(Token::kLPar);
        // Eats '('
        current_itr_++;

        int args_count = 0;
        while (current_itr_->type != Token::kRPar &&
               current_itr_->type != Token::kEOF)
        {
            args_count++;
            ParseExpr();
            if (current_itr_->type == Token::kRPar)
                break;

            AssertCurrentTokenIs(Token::kComma);
            // Eats ','
            current_itr_++;
        }
        AssertIsNotEOF();

        // Eats ')'
        current_itr_++;

        // Match a ImageFilter builder which will consume the operands
        // in the stack and push a new-created SkImageFilter object into it.
        if (builders_map_.count(filter_name) == 0)
            g_throw(Error, fmt::format("Invalid name for image filter: {}", filter_name));

        Effector effector = builders_map_[filter_name](operand_stack_, args_count);
        if (!effector)
            g_throw(Error, fmt::format("Failed to create a `{}` filter", filter_name));

        auto operand = std::make_unique<EffectStackOperand>();
        operand->type = EffectStackOperand::kEffector;
        operand->effector = effector;
        operand_stack_.push(std::move(operand));

        return effector;
    }

    // expr := REPLACEMENT | NULL | INTEGER | FLOAT | array | filter
    // NOLINTNEXTLINE
    void ParseExpr()
    {
        Token::TokenType type = current_itr_->type;

        EffectStackOperand::Ptr operand = std::make_unique<EffectStackOperand>();
        switch (type)
        {
        case Token::kReplacement:
            operand->type = EffectStackOperand::kKWArgs;
            operand->kwarg_pair.first = current_itr_->lexeme;
            operand->kwarg_pair.second = FindValueInKWArgs(current_itr_->lexeme);
            break;

        case Token::kNull:
            operand->type = EffectStackOperand::kNull;
            break;

        case Token::kInteger:
            operand->type = EffectStackOperand::kInt;
            operand->numeric.vi = current_itr_->integer_value;
            break;

        case Token::kFloat:
            operand->type = EffectStackOperand::kFloat;
            operand->numeric.vf = static_cast<SkScalar>(current_itr_->double_value);
            break;

        case Token::kLBracket:
            ParseArray();
            return;

        default:
            ParseEffector();
            return;
        }

        operand_stack_.push(std::move(operand));
        current_itr_++;
    }

    // array := '[' expr ',' expr ... ']'
    // NOLINTNEXTLINE
    void ParseArray()
    {
        AssertCurrentTokenIs(Token::kLBracket);
        // Eat '['
        current_itr_++;

        int elements_count = 0;
        while (current_itr_->type != Token::kEOF)
        {
            elements_count++;
            ParseExpr();
            if (current_itr_->type == Token::kRBracket)
                break;

            AssertCurrentTokenIs(Token::kComma);
            // Eats ','
            current_itr_++;
        }
        AssertIsNotEOF();
        // Eats ']'
        current_itr_++;

        // Reduce: consume `elements_count` operands and pack them into
        //         a single array.

        EffectStackOperand::Ptr operand = std::make_unique<EffectStackOperand>();
        operand->type = EffectStackOperand::kArray;
        for (int i = 0; i < elements_count; i++)
        {
            operand->array.emplace_back(std::move(operand_stack_.top()));
            operand_stack_.pop();
        }

        // Operands are pushed in the correct order into stack and popped
        // in a reverse order. Reverse them again to get the correct order.
        std::reverse(operand->array.begin(), operand->array.end());
        operand_stack_.push(std::move(operand));
    }

private:
    v8::Local<v8::Value> FindValueInKWArgs(const std::string& name)
    {
        if (kwargs_map_.count(name) == 0)
        {
            g_throw(Error,
                    fmt::format("Missing required keyword argument '{}' in kwargs dictionary", name));
        }
        return kwargs_map_[name];
    }

    void AssertIsNotEOF()
    {
        if (current_itr_->type == Token::kEOF)
            report_syntax_error("Unexpected EOF", source_, current_itr_->location);
    }

    void AssertCurrentTokenIs(Token::TokenType type)
    {
        if (current_itr_->type != type)
        {
            report_syntax_error(
                    fmt::format("Unexpected {}, expecting a(n) {}",
                                g_toktype_names_map[current_itr_->type],
                                g_toktype_names_map[type]),
                    source_, current_itr_->location);
        }
    }

    const std::string&          source_;
    TokenList                   token_list_;
    TokenList::const_iterator   current_itr_;
    EffectStackOperand::KWArgsMap          kwargs_map_;
    EffectDSLParser::EffectorBuildersMap&  builders_map_;
    EffectStack                            operand_stack_;
};

} // namespace anonymous

const char *EffectStackOperand::GetTypeName(Type type)
{
    static std::unordered_map<Type, const char*> name_map = {
            { kNull,     "Null"      },
            { kInt,      "Int"       },
            { kEffector, "Effector"  },
            { kArray,    "Array"     },
            { kKWArgs,   "KWArgs"    }
    };

    CHECK(name_map.count(type) > 0);
    return name_map[type];
}

void EffectStackOperand::AssertKWArgsJSType(bool check_result)
{
    if (!check_result)
    {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        auto type_name = binder::from_v8<std::string>(isolate,
                                                      kwarg_pair.second->TypeOf(isolate));
        g_throw(TypeError, fmt::format("Invalid type '{}' for keyword argument '{}'",
                                       type_name, kwarg_pair.first));
    }
}

#define NULLABLE_CHECK do { if (type == kNull) return {}; } while (false)

EffectStackOperand::Nullable<SkScalar> EffectStackOperand::ToFloatSafe()
{
    NULLABLE_CHECK;
    if (type == kInt)
        return numeric.vi;
    else if (type == kFloat)
        return numeric.vf;
    else if (type == kKWArgs)
    {
        AssertKWArgsJSType(kwarg_pair.second->IsNumber());
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        return binder::from_v8<SkScalar>(isolate, kwarg_pair.second);
    }

    g_throw(TypeError, "Operand cannot be converted to SkScalar");
}

EffectStackOperand::Nullable<int32_t> EffectStackOperand::ToIntegerSafe()
{
    NULLABLE_CHECK;
    if (type == kInt)
        return numeric.vi;
    else if (type == kKWArgs)
    {
        AssertKWArgsJSType(kwarg_pair.second->IsNumber());
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        return binder::from_v8<int32_t>(isolate, kwarg_pair.second);
    }

    g_throw(TypeError, "Operand cannot be converted to int32");
}

EffectStackOperand::Nullable<Effector> EffectStackOperand::ToEffectorSafe()
{
    NULLABLE_CHECK;
    if (type == kEffector)
        return effector;
    else if (type != kKWArgs)
        g_throw(TypeError, "Operand cannot be converted to filter");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK(isolate);

    auto *img_flt_wrapped = binder::Class<CkImageFilterWrap>::unwrap_object(isolate, kwarg_pair.second);
    if (img_flt_wrapped)
    {
        CHECK(img_flt_wrapped->getSkiaObject());
        return Effector(img_flt_wrapped->getSkiaObject());
    }

    auto *color_flt_wrapped = binder::Class<CkColorFilterWrap>::unwrap_object(isolate, kwarg_pair.second);
    if (color_flt_wrapped)
    {
        CHECK(color_flt_wrapped->getSkiaObject());
        return Effector(color_flt_wrapped->getSkiaObject());
    }

    auto *shader_wrapped = binder::Class<CkShaderWrap>::unwrap_object(isolate, kwarg_pair.second);
    if (shader_wrapped)
    {
        CHECK(shader_wrapped->getSkiaObject());
        return Effector(shader_wrapped->getSkiaObject());
    }

    auto *patheffect_wrapped = binder::Class<CkPathEffect>::unwrap_object(isolate, kwarg_pair.second);
    if (patheffect_wrapped)
    {
        CHECK(patheffect_wrapped->getSkiaObject());
        return Effector(patheffect_wrapped->getSkiaObject());
    }

    g_throw(TypeError, fmt::format("Keyword argument `{}` must be an instance"
                                   " one of `CkImageFilter`, `CkColorFilter`, "
                                   "`CkShader`, `CkPath` or `CkPathEffect`",
                                   kwarg_pair.first));
}

EffectStackOperand::Nullable<sk_sp<SkImage>> EffectStackOperand::ToImageSafe()
{
    NULLABLE_CHECK;
    if (type != kKWArgs)
        g_throw(Error, "Only kwarg operand can be converted to Image");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK(isolate);

    auto *wrapped = binder::Class<CkImageWrap>::unwrap_object(
            isolate, kwarg_pair.second);
    if (!wrapped)
    {
        g_throw(TypeError,
                fmt::format("Keyword argument `{}` is not an instance of CkImage",
                            kwarg_pair.first));
    }

    return wrapped->getImage();
}

EffectStackOperand::Nullable<SkPath*> EffectStackOperand::ToPathSafe()
{
    NULLABLE_CHECK;
    if (type != kKWArgs)
        g_throw(Error, "Only kwarg operand can be converted to Path");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK(isolate);

    auto *wrapped = binder::Class<CkPath>::unwrap_object(isolate, kwarg_pair.second);
    if (!wrapped)
    {
        g_throw(TypeError,
                fmt::format("Keyword argument `{}` is not an instance of CkPath",
                            kwarg_pair.first));
    }

    return &wrapped->GetPath();
}

EffectStackOperand::Nullable<SkMatrix*> EffectStackOperand::ToMatrixSafe()
{
    NULLABLE_CHECK;
    if (type != kKWArgs)
        g_throw(Error, "Only kwarg operand can be converted to Matrix");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK(isolate);

    auto *wrapped = binder::Class<CkMatrix>::unwrap_object(isolate, kwarg_pair.second);
    if (!wrapped)
    {
        g_throw(TypeError,
                fmt::format("Keyword argument `{}` is not an instance of CkMatrix",
                            kwarg_pair.first));
    }

    return &wrapped->GetMatrix();
}

EffectStackOperand::Nullable<SkRect> EffectStackOperand::ToRectSafe()
{
    NULLABLE_CHECK;

    using Op = const EffectStackOperand::Ptr&;
    auto xywh = ToMonoTypeArraySafe<SkScalar>([](Op op) {
        return op->ToFloatSafe();
    });

    if (!xywh) [[unlikely]]
        return {};

    if (xywh->size() != 4)
        g_throw(Error, "Rect type must be an array of 4 Float elements");

    return SkRect::MakeXYWH(xywh->at(0), xywh->at(1),
                            xywh->at(2), xywh->at(3));
}

EffectStackOperand::Nullable<SkColor> EffectStackOperand::ToColorSafe()
{
    NULLABLE_CHECK;

    using Op = const EffectStackOperand::Ptr&;
    auto color_4f = ToMonoTypeArraySafe<SkScalar>([](Op op) {
        return op->ToFloatSafe();
    });

    if (!color_4f) [[unlikely]]
        return {};

    if (color_4f->size() != 4)
        g_throw(Error, "Color type must be an array of 4 Float elements");

    return SkColor4f{(*color_4f)[0], (*color_4f)[1],
                     (*color_4f)[2], (*color_4f)[3]}.toSkColor();
}

EffectStackOperand::Nullable<SkPoint3> EffectStackOperand::ToVector3Safe()
{
    NULLABLE_CHECK;

    using Op = const EffectStackOperand::Ptr&;
    auto vec = ToMonoTypeArraySafe<SkScalar>([](Op op) {
        return op->ToFloatSafe();
    });

    if (!vec) [[unlikely]]
        return {};

    if (vec->size() != 3)
        g_throw(Error, "Vector3 type must be an array of 3 Float elements");

    return SkPoint3::Make((*vec)[0], (*vec)[1], (*vec)[2]);
}

EffectStackOperand::Nullable<SkPoint> EffectStackOperand::ToVector2Safe()
{
    NULLABLE_CHECK;

    using Op = const EffectStackOperand::Ptr&;
    auto vec = ToMonoTypeArraySafe<SkScalar>([](Op op) {
        return op->ToFloatSafe();
    });

    if (!vec) [[unlikely]]
        return {};

    if (vec->size() != 2)
        g_throw(Error, "Vector2 type must be an array of 2 Float elements");

    return SkPoint::Make((*vec)[0], (*vec)[1]);
}

EffectStackOperand::Nullable<SkIPoint> EffectStackOperand::ToIVector2Safe()
{
    NULLABLE_CHECK;

    using Op = const EffectStackOperand::Ptr&;
    auto vec = ToMonoTypeArraySafe<int32_t>([](Op op) {
        return op->ToIntegerSafe();
    });

    if (!vec) [[unlikely]]
        return {};

    if (vec->size() != 2)
        g_throw(Error, "IVector2 type must be an array of 2 Integer elements");

    return SkIPoint::Make((*vec)[0], (*vec)[1]);
}

Effector EffectDSLParser::Parse(v8::Isolate *isolate,
                                v8::Local<v8::String> dsl,
                                v8::Local<v8::Object> kwargs,
                                EffectorBuildersMap& builders_map)
{
    CHECK(isolate);

    if (!kwargs->IsObject())
        g_throw(TypeError, "`params` must be an object (dictionary) containing kwargs of descriptor");

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> kwargs_dict = v8::Local<v8::Object>::Cast(kwargs);

    EffectStackOperand::KWArgsMap kwargs_map;

    v8::Local<v8::Array> kwargs_names = CHECKED(kwargs_dict->GetOwnPropertyNames(context));
    for (int i = 0; i < kwargs_names->Length(); i++)
    {
        v8::Local<v8::Value> name = CHECKED(kwargs_names->Get(context, i));
        if (!name->IsString())
            g_throw(TypeError, "kwargs dictionary has a non-string named property");
        auto name_str = binder::from_v8<std::string>(isolate, name);
        kwargs_map[std::move(name_str)] = CHECKED(kwargs_dict->Get(context, name));
    }

    Parser parser(binder::from_v8<std::string>(isolate, dsl),
                  std::move(kwargs_map),
                  builders_map);

    return parser.ParseEffector();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
