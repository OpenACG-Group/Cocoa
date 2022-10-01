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

#include <utility>
#include <list>
#include <stack>
#include <unordered_map>
#include <vector>

#include "include/core/SkData.h"
#include "include/effects/SkImageFilters.h"
#include "fmt/format.h"

#include "Gallium/bindings/core/Exports.h"
#include "Gallium/bindings/glamor/Exports.h"
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
        while (std::isdigit(peek))
        {
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
                         static_cast<double>(v_int) / float_scale,
                         v_int};
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

static std::unordered_map<Token::TokenType,
                          std::string_view> g_toktype_names_map = {
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

using KWArgsMap = std::unordered_map<std::string, v8::Local<v8::Value>>;
using KWArgsPair = std::pair<std::string, v8::Local<v8::Value>>;

template<typename T>
using Nullable = std::optional<T>;

struct StackOperand
{
    enum Type
    {
        kNull,
        kInt,
        kFloat,
        kFilter,
        kArray,
        kKWArgs
    };

    static const char *GetTypeName(Type type)
    {
        static std::unordered_map<Type, const char*> name_map = {
            { kNull,    "Null"      },
            { kInt,     "Int"       },
            { kFilter,  "Filter"    },
            { kArray,   "Array"     },
            { kKWArgs,  "KWArgs"    }
        };

        CHECK(name_map.count(type) > 0);
        return name_map[type];
    }

    template<size_t N>
    void AssertTypes(const std::array<Type, N>& types) const
    {
        bool match = false;
        for (Type expected : types)
        {
            if (type == expected)
                match = true;
        }
        if (!match)
        {
            auto msg = fmt::format("Unexpected operand type {}",
                GetTypeName(type));
            g_throw(Error, msg);
        }
    }

    void AssertKWArgsJSType(bool check_result)
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

    // Cast functions:
    // Cast an operand to a required C++ type (primitive type or compound type).
    // These functions return a null value if the operand is null,
    // and throw a JSException if fail to convert.

#define NULLABLE_CHECK do { if (type == kNull) return {}; } while (false)

    Nullable<SkScalar> ToFloatSafe()
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

    Nullable<int32_t> ToIntegerSafe()
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

    Nullable<sk_sp<SkImageFilter>> ToFilterSafe()
    {
        NULLABLE_CHECK;
        if (type == kFilter)
            return filter;
        else if (type == kKWArgs)
        {
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            auto *wrapped = binder::Class<CkImageFilterWrap>::unwrap_object(
                    isolate, kwarg_pair.second);
            if (!wrapped)
            {
                g_throw(TypeError,
                        fmt::format("Keyword argument `{}` is not an instance "
                                    "of CkImageFilter", kwarg_pair.first));
            }

            CHECK(wrapped->getImageFilter());
            return wrapped->getImageFilter();
        }

        g_throw(TypeError, "Operand cannot be converted to filter");
    }

    template<typename T, typename Cast = std::function<Nullable<T>(
            const std::unique_ptr<StackOperand>&)>>
    Nullable<std::vector<T>> ToMonoTypeArraySafe(const Cast& value_cast)
    {
        NULLABLE_CHECK;
        AssertTypes<1>({kArray});

        std::vector<T> result;
        for (const auto& operand : array)
        {
            Nullable<T> maybe = value_cast(operand);
            if (!maybe)
                g_throw(Error, "Array members must not be null");
            result.emplace_back(std::move(*maybe));
        }
        return std::move(result);
    }

    Type type;

    // Only available when `type` is `kInt` or `kFloat`
    union {
        int32_t     vi;
        SkScalar    vf;
    } numeric;

    // Only available when `type` is `kFilter`
    sk_sp<SkImageFilter> filter;

    // Only available when `type` is `kArray`
    std::vector<std::unique_ptr<StackOperand>> array;

    // Only available when `type` is `kKWArgs`
    KWArgsPair kwarg_pair;
};
using OperandStack = std::stack<std::unique_ptr<StackOperand>>;

#define DEF_BUILDER(name) sk_sp<SkImageFilter> builder_##name (OperandStack& st, int argc)

#define THROW_IF_NULL(v, arg, flt) \
    do {                           \
        if (!(v)) {                \
            g_throw(Error, "Argument `" #arg "` for `" #flt "` cannot be null"); \
        }                          \
    } while (false)

//! FilterDecl: blur(Float sigma_x, Float sigma_y, Int tile_mode?, Filter input?)
DEF_BUILDER(blur)
{
    if (argc != 4)
        g_throw(Error, "Wrong number of arguments for `blur` filter");

    // Arguments in the stack should be taken in the reverse order
    Nullable<sk_sp<SkImageFilter>> input = st.top()->ToFilterSafe();
    st.pop();

    Nullable<int32_t> tile_mode_int = st.top()->ToIntegerSafe();
    st.pop();

    if (tile_mode_int)
    {
        if (*tile_mode_int < 0 || *tile_mode_int > static_cast<int>(SkTileMode::kLastTileMode))
            g_throw(RangeError, "Invalid enumeration value in argument `tile_mode` for `blur` filter");
    }

    Nullable<SkScalar> sigma_y = st.top()->ToFloatSafe();
    st.pop();
    THROW_IF_NULL(sigma_y, sigma_y, blur);

    Nullable<SkScalar> sigma_x = st.top()->ToFloatSafe();
    st.pop();
    THROW_IF_NULL(sigma_x, sigma_x, blur);

    return SkImageFilters::Blur(*sigma_x, *sigma_y,
                                tile_mode_int ? static_cast<SkTileMode>(*tile_mode_int)
                                              : SkTileMode::kClamp,
                                input ? *input : nullptr);
}

//! FilterDecl: compose(Filter outer, Filter inner)
DEF_BUILDER(compose)
{
    if (argc != 2)
        g_throw(Error, "Wrong number of arguments for `compose` filter");

    Nullable<sk_sp<SkImageFilter>> inner = st.top()->ToFilterSafe();
    st.pop();
    THROW_IF_NULL(inner, inner, compose);

    Nullable<sk_sp<SkImageFilter>> outer = st.top()->ToFilterSafe();
    st.pop();
    THROW_IF_NULL(outer, outer, compose);

    return SkImageFilters::Compose(*outer, *inner);
}

// TODO(sora): implement more filter builders.

using ImageFilterBuilder = std::function<sk_sp<SkImageFilter>(OperandStack&, int)>;
std::unordered_map<std::string_view, ImageFilterBuilder> g_image_filter_builders_map = {
    { "blur", builder_blur },
    { "compose", builder_compose }
};

class Parser
{
public:
    explicit Parser(const std::string& str, KWArgsMap args_map)
        : source_(str)
        , token_list_(RefStringLexer::Tokenize(str))
        , current_itr_(token_list_.begin())
        , kwargs_map_(std::move(args_map))
    {
    }

    // filter := IDENTIFIER '(' expr ',' expr ... ')'
    sk_sp<SkImageFilter> ParseFilter()
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
        if (g_image_filter_builders_map.count(filter_name) == 0)
            g_throw(Error, fmt::format("Invalid name for image filter: {}", filter_name));

        sk_sp<SkImageFilter> filter = g_image_filter_builders_map[filter_name](operand_stack_, args_count);
        if (!filter)
            g_throw(Error, fmt::format("Failed to create a `{}` filter", filter_name));

        auto operand = std::make_unique<StackOperand>();
        operand->type = StackOperand::kFilter;
        operand->filter = filter;
        operand_stack_.push(std::move(operand));

        return filter;
    }

    // expr := REPLACEMENT | NULL | INTEGER | FLOAT | array | filter
    void ParseExpr()
    {
        Token::TokenType type = current_itr_->type;

        std::unique_ptr<StackOperand> operand = std::make_unique<StackOperand>();
        switch (type)
        {
        case Token::kReplacement:
            operand->type = StackOperand::kKWArgs;
            operand->kwarg_pair.first = current_itr_->lexeme;
            operand->kwarg_pair.second = FindValueInKWArgs(current_itr_->lexeme);
            break;

        case Token::kNull:
            operand->type = StackOperand::kNull;
            break;

        case Token::kInteger:
            operand->type = StackOperand::kInt;
            operand->numeric.vi = current_itr_->integer_value;
            break;

        case Token::kFloat:
            operand->type = StackOperand::kFloat;
            operand->numeric.vf = static_cast<SkScalar>(current_itr_->double_value);
            break;

        case Token::kLBracket:
            ParseArray();
            return;

        default:
            ParseFilter();
            return;
        }

        operand_stack_.push(std::move(operand));
        current_itr_++;
    }

    // array := '[' expr ',' expr ... ']'
    void ParseArray()
    {
        AssertCurrentTokenIs(Token::kLBracket);
        // Eat '['
        current_itr_++;

        int elements_count = 0;
        while (current_itr_->type != Token::kLBracket &&
               current_itr_->type != Token::kEOF)
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

        std::unique_ptr<StackOperand> operand = std::make_unique<StackOperand>();
        operand->type = StackOperand::kArray;
        for (int i = 0; i < elements_count; i++)
        {
            operand->array.emplace_back(std::move(operand_stack_.top()));
            operand_stack_.pop();
        }

        // Operands are pushed in the correct order into stack and popped
        // in a reverse order. Reverse them again to get the correct order.
        std::reverse(operand->array.begin(), operand->array.end());
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
    KWArgsMap                   kwargs_map_;
    OperandStack                operand_stack_;
};

} // namespace anonymous

v8::Local<v8::Value> CkImageFilterWrap::MakeFromDescriptor(const std::string& descriptor,
                                                           v8::Local<v8::Value> params)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!params->IsObject())
        g_throw(TypeError, "`params` must be an object (dictionary) containing kwargs of descriptor");

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> kwargs_dict = v8::Local<v8::Object>::Cast(params);

    KWArgsMap kwargs_map;

    v8::Local<v8::Array> kwargs_names = CHECKED(kwargs_dict->GetOwnPropertyNames(context));
    for (int i = 0; i < kwargs_names->Length(); i++)
    {
        v8::Local<v8::Value> name = CHECKED(kwargs_names->Get(context, i));
        if (!name->IsString())
            g_throw(TypeError, "kwargs dictionary has a non-string named property");
        auto name_str = binder::from_v8<std::string>(isolate, name);
        kwargs_map[std::move(name_str)] = CHECKED(kwargs_dict->Get(context, name));
    }

    Parser parser(descriptor, std::move(kwargs_map));
    sk_sp<SkImageFilter> filter = parser.ParseFilter();
    CHECK(filter);

    return binder::Class<CkImageFilterWrap>::create_object(isolate, filter);
}

CkImageFilterWrap::CkImageFilterWrap(sk_sp<SkImageFilter> filter)
    : image_filter_(std::move(filter))
{
}

sk_sp<SkImageFilter> CkImageFilterWrap::getImageFilter() const
{
    return image_filter_;
}

v8::Local<v8::Value> CkImageFilterWrap::serialize()
{
    sk_sp<SkData> data = image_filter_->serialize();
    if (!data)
        g_throw(Error, "Failed to serialize the image filter");

    return Buffer::MakeFromExternal(data->writable_data(), data->size(),
                                    [data]() { CHECK(data->unique()); });
}

v8::Local<v8::Value> CkImageFilterWrap::Deserialize(v8::Local<v8::Value> buffer)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Buffer *wrapper = binder::Class<Buffer>::unwrap_object(isolate, buffer);
    if (!wrapper)
        g_throw(TypeError, "Argument `buffer` must be an instance of core:Buffer");

    auto filter = SkImageFilter::Deserialize(wrapper->addressU8(), wrapper->length());
    if (!filter)
        g_throw(Error, "Failed to deserialize the given buffer as an image filter");

    return binder::Class<CkImageFilterWrap>::create_object(isolate, filter);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
