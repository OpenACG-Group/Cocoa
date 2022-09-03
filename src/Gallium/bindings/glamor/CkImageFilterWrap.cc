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

#include "include/effects/SkImageFilters.h"
#include "fmt/format.h"

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
        kNumber,        // floats and integers
        kIdentifier,    // identifiers
        kLPar,          // (
        kRPar,          // )
        kLBracket,      // [
        kRBracket,      // ]
        kComma,         // ,
        kReplacement,   // %something
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

g_noreturn void report_syntax_error(const std::string& error,
                                    const std::string& source,
                                    const Location& loc)
{
    // TODO: Report a more detailed information
    g_throw(Error, error);
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
                { '\0', Token::kEOF      },
                { '(',  Token::kLPar     },
                { ')',  Token::kRPar     },
                { '[',  Token::kLBracket },
                { ']',  Token::kRBracket },
                { ',',  Token::kComma    }
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
            while (std::isalpha(peek))
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
            return Token{Token::kNumber,
                         start_location,
                         "",
                         static_cast<double>(v_int) / float_scale,
                         v_int};
        }

        // Recognize identifier
        peek = PeekNext();
        if (!std::isalpha(peek))
        {
            report_syntax_error("Unrecognized character",
                                input_, current_loc_);
        }

        std::string lexeme;
        while (std::isalpha(peek))
        {
            StepForward();
            lexeme.push_back(peek);
            peek = PeekNext();
        }

        return Token{Token::kIdentifier, start_location, lexeme};
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

class Parser
{
public:
    class MaybeError
    {
    public:
        MaybeError()
            : error_(nullptr), location_{} {}
        MaybeError(const char *error, const Location& location)
            : error_(error), location_(location) {}

        static MaybeError MakeSuccess() {
            return MaybeError();
        }

        static MaybeError Make(const char *error, const Location& location) {
            return MaybeError(error, location);
        }

        g_nodiscard bool HasError() const {
            return error_;
        }

        explicit operator bool() const {
            return HasError();
        }

    private:
        const char *error_;
        Location    location_;
    };

    explicit Parser(const std::string& input)
        : source_(input), token_list_(RefStringLexer::Tokenize(input))
        , current_token_itr_(token_list_.begin()) {}
    ~Parser() = default;

    // filter := IDENT '(' param_list* ')'
    MaybeError ParseFilter()
    {
        Token& token = GetToken();
        if (token.type != Token::kIdentifier)
            return {"Unexpected token, expecting an identifier here", token.location};
        std::string filter_name = std::move(token.lexeme);

        token = GetToken();
        if (token.type != Token::kLPar)
            return {"Unexpected token, expecting a '(' here", token.location};

        if (auto error = ParseParamList())
            return error;

        token = GetToken();
        if (token.type != Token::kRPar)
        {
            return {"Unexpected token, expecting a ')' here", token.location};
        }

        fmt::print("Filter: {}\n", token.lexeme);

        return MaybeError::MakeSuccess();
    }

    // param_list := param | param_list ',' param
    MaybeError ParseParamList()
    {
        Save();
        MaybeError error = ParseParam();
        while (!error)
        {
            // Accept a `param` non-terminator successfully, and discard
            // the save recording.
            Discard();

            Save();
            error = ParseParam();
            if (error)
            {
                Restore();
                return MaybeError::MakeSuccess();
            }
        }
        Restore();
    }

    MaybeError ParseParam()
    {
    }

private:
    void Save()
    {
        iterator_stack_.push(current_token_itr_);
    }

    void Restore()
    {
        current_token_itr_ = iterator_stack_.top();
        iterator_stack_.pop();
    }

    void Discard()
    {
        iterator_stack_.pop();
    }

    Token& GetToken()
    {
        CHECK(current_token_itr_ != token_list_.end());
        return *current_token_itr_++;
    }

    const std::string&                  source_;
    TokenList                           token_list_;
    TokenList::iterator                 current_token_itr_;
    std::stack<TokenList::iterator>     iterator_stack_;
};

void parse_descriptor(const std::string& descriptor)
{
    TokenList token_list = RefStringLexer::Tokenize(descriptor);
}

} // namespace anonymous

v8::Local<v8::Value> CkImageFilterWrap::MakeFromDescriptor(const std::string& descriptor,
                                                           v8::Local<v8::Value> params)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!params->IsArray())
        g_throw(TypeError, "`params` must be an array contains parameters of descriptor");

    parse_descriptor(descriptor);

    return v8::Undefined(isolate);
}

CkImageFilterWrap::CkImageFilterWrap(sk_sp<SkImageFilter> filter)
    : image_filter_(std::move(filter))
{
}

sk_sp<SkImageFilter> CkImageFilterWrap::getImageFilter() const
{
    return image_filter_;
}

GALLIUM_BINDINGS_GLAMOR_NS_END
