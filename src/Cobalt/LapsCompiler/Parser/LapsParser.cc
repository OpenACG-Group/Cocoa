// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.

// "%code top" blocks.
#line 28 "LapsParser.yy"

    #include "Cobalt/LapsCompiler/Parser/LapsScanner.h"
    #include "Cobalt/LapsCompiler/Parser/LapsParser.hh"
    #include "Cobalt/LapsCompiler/Parser/LapsParserDriver.h"
    #include "Cobalt/LapsCompiler/Parser/location.hh"

    LAPS_COMPILER_BEGIN_NS
    static LapsParser::symbol_type yylex(LapsScanner& scanner, ParserDriver& drv)
    {
        return scanner.nextToken(drv);
    }
    LAPS_COMPILER_END_NS

#line 53 "LapsParser.cc"




#include "LapsParser.hh"




#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 4 "LapsParser.yy"
namespace  cocoa { namespace cobalt { namespace laps  {
#line 153 "LapsParser.cc"

  /// Build a parser object.
   LapsParser :: LapsParser  (LapsScanner& scanner_yyarg, ParserDriver& drv_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      scanner (scanner_yyarg),
      drv (drv_yyarg)
  {}

   LapsParser ::~ LapsParser  ()
  {}

   LapsParser ::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/



  // by_state.
   LapsParser ::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

   LapsParser ::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
   LapsParser ::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
   LapsParser ::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

   LapsParser ::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

   LapsParser ::symbol_kind_type
   LapsParser ::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

   LapsParser ::stack_symbol_type::stack_symbol_type ()
  {}

   LapsParser ::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_translation_unit: // translation_unit
      case symbol_kind::S_statement_list: // statement_list
      case symbol_kind::S_statement: // statement
      case symbol_kind::S_literal_value: // literal_value
      case symbol_kind::S_let_declaration: // let_declaration
      case symbol_kind::S_decl_attributes: // decl_attributes
      case symbol_kind::S_decl_attributes_spec_list: // decl_attributes_spec_list
      case symbol_kind::S_decl_attribute_spec: // decl_attribute_spec
      case symbol_kind::S_decl_attribute_literal_list: // decl_attribute_literal_list
        value.YY_MOVE_OR_COPY< ASTNodeBase::Ptr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_LITERAL_DEC_INT: // LITERAL_DEC_INT
      case symbol_kind::S_LITERAL_HEX_INT: // LITERAL_HEX_INT
      case symbol_kind::S_LITERAL_FLOAT: // LITERAL_FLOAT
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

   LapsParser ::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_translation_unit: // translation_unit
      case symbol_kind::S_statement_list: // statement_list
      case symbol_kind::S_statement: // statement
      case symbol_kind::S_literal_value: // literal_value
      case symbol_kind::S_let_declaration: // let_declaration
      case symbol_kind::S_decl_attributes: // decl_attributes
      case symbol_kind::S_decl_attributes_spec_list: // decl_attributes_spec_list
      case symbol_kind::S_decl_attribute_spec: // decl_attribute_spec
      case symbol_kind::S_decl_attribute_literal_list: // decl_attribute_literal_list
        value.move< ASTNodeBase::Ptr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_LITERAL_DEC_INT: // LITERAL_DEC_INT
      case symbol_kind::S_LITERAL_HEX_INT: // LITERAL_HEX_INT
      case symbol_kind::S_LITERAL_FLOAT: // LITERAL_FLOAT
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
   LapsParser ::stack_symbol_type&
   LapsParser ::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_translation_unit: // translation_unit
      case symbol_kind::S_statement_list: // statement_list
      case symbol_kind::S_statement: // statement
      case symbol_kind::S_literal_value: // literal_value
      case symbol_kind::S_let_declaration: // let_declaration
      case symbol_kind::S_decl_attributes: // decl_attributes
      case symbol_kind::S_decl_attributes_spec_list: // decl_attributes_spec_list
      case symbol_kind::S_decl_attribute_spec: // decl_attribute_spec
      case symbol_kind::S_decl_attribute_literal_list: // decl_attribute_literal_list
        value.copy< ASTNodeBase::Ptr > (that.value);
        break;

      case symbol_kind::S_LITERAL_DEC_INT: // LITERAL_DEC_INT
      case symbol_kind::S_LITERAL_HEX_INT: // LITERAL_HEX_INT
      case symbol_kind::S_LITERAL_FLOAT: // LITERAL_FLOAT
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

   LapsParser ::stack_symbol_type&
   LapsParser ::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_translation_unit: // translation_unit
      case symbol_kind::S_statement_list: // statement_list
      case symbol_kind::S_statement: // statement
      case symbol_kind::S_literal_value: // literal_value
      case symbol_kind::S_let_declaration: // let_declaration
      case symbol_kind::S_decl_attributes: // decl_attributes
      case symbol_kind::S_decl_attributes_spec_list: // decl_attributes_spec_list
      case symbol_kind::S_decl_attribute_spec: // decl_attribute_spec
      case symbol_kind::S_decl_attribute_literal_list: // decl_attribute_literal_list
        value.move< ASTNodeBase::Ptr > (that.value);
        break;

      case symbol_kind::S_LITERAL_DEC_INT: // LITERAL_DEC_INT
      case symbol_kind::S_LITERAL_HEX_INT: // LITERAL_HEX_INT
      case symbol_kind::S_LITERAL_FLOAT: // LITERAL_FLOAT
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
   LapsParser ::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
   LapsParser ::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
   LapsParser ::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
   LapsParser ::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
   LapsParser ::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
   LapsParser ::debug_stream () const
  {
    return *yycdebug_;
  }

  void
   LapsParser ::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


   LapsParser ::debug_level_type
   LapsParser ::debug_level () const
  {
    return yydebug_;
  }

  void
   LapsParser ::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

   LapsParser ::state_type
   LapsParser ::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
   LapsParser ::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
   LapsParser ::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
   LapsParser ::operator() ()
  {
    return parse ();
  }

  int
   LapsParser ::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (scanner, drv));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_translation_unit: // translation_unit
      case symbol_kind::S_statement_list: // statement_list
      case symbol_kind::S_statement: // statement
      case symbol_kind::S_literal_value: // literal_value
      case symbol_kind::S_let_declaration: // let_declaration
      case symbol_kind::S_decl_attributes: // decl_attributes
      case symbol_kind::S_decl_attributes_spec_list: // decl_attributes_spec_list
      case symbol_kind::S_decl_attribute_spec: // decl_attribute_spec
      case symbol_kind::S_decl_attribute_literal_list: // decl_attribute_literal_list
        yylhs.value.emplace< ASTNodeBase::Ptr > ();
        break;

      case symbol_kind::S_LITERAL_DEC_INT: // LITERAL_DEC_INT
      case symbol_kind::S_LITERAL_HEX_INT: // LITERAL_HEX_INT
      case symbol_kind::S_LITERAL_FLOAT: // LITERAL_FLOAT
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
        yylhs.value.emplace< std::string > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // translation_unit: statement_list $end
#line 84 "LapsParser.yy"
                {
                    yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTTranslationUnitNode>(yystack_[1].value.as < ASTNodeBase::Ptr > ());
                    drv.SetTranslationUnit(yylhs.value.as < ASTNodeBase::Ptr > ()->Cast<ASTTranslationUnitNode>());
                }
#line 646 "LapsParser.cc"
    break;

  case 3: // statement_list: statement
#line 91 "LapsParser.yy"
                {
                    yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTStatementListNode>()
                         ->AppendChild(yystack_[0].value.as < ASTNodeBase::Ptr > ());
                }
#line 655 "LapsParser.cc"
    break;

  case 4: // statement_list: statement_list statement
#line 96 "LapsParser.yy"
                {
                    yylhs.value.as < ASTNodeBase::Ptr > () = yystack_[1].value.as < ASTNodeBase::Ptr > ()->Cast<ASTStatementListNode>()
                         ->AppendChild(yystack_[0].value.as < ASTNodeBase::Ptr > ());
                }
#line 664 "LapsParser.cc"
    break;

  case 5: // statement: let_declaration ";"
#line 102 "LapsParser.yy"
                                                    { yylhs.value.as < ASTNodeBase::Ptr > () = yystack_[1].value.as < ASTNodeBase::Ptr > (); }
#line 670 "LapsParser.cc"
    break;

  case 6: // literal_value: LITERAL_DEC_INT
#line 106 "LapsParser.yy"
                {
                    yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTLiteralValueNode>(yystack_[0].location,
                        ASTLiteralValueNode::LiteralType::kDecInteger, yystack_[0].value.as < std::string > ());
                    yylhs.location = yystack_[0].location;
                }
#line 680 "LapsParser.cc"
    break;

  case 7: // literal_value: LITERAL_HEX_INT
#line 112 "LapsParser.yy"
                {
                    yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTLiteralValueNode>(yystack_[0].location,
                        ASTLiteralValueNode::LiteralType::kHexInteger, yystack_[0].value.as < std::string > ());
                    yylhs.location = yystack_[0].location;
                }
#line 690 "LapsParser.cc"
    break;

  case 8: // literal_value: LITERAL_FLOAT
#line 118 "LapsParser.yy"
                {
                    yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTLiteralValueNode>(yystack_[0].location,
                        ASTLiteralValueNode::LiteralType::kFloat, yystack_[0].value.as < std::string > ());
                    yylhs.location = yystack_[0].location;
                }
#line 700 "LapsParser.cc"
    break;

  case 9: // let_declaration: "let" IDENTIFIER ":" IDENTIFIER
#line 126 "LapsParser.yy"
                    {
                        auto t = ASTNodeBase::New<ASTTypeNameExprNode>(yystack_[0].location, yystack_[0].value.as < std::string > ());
                        yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTLetDeclarationStmtNode>(nullptr, t, yystack_[2].value.as < std::string > ());
                    }
#line 709 "LapsParser.cc"
    break;

  case 10: // let_declaration: decl_attributes "let" IDENTIFIER ":" IDENTIFIER
#line 131 "LapsParser.yy"
                    {
                        auto t = ASTNodeBase::New<ASTTypeNameExprNode>(yystack_[0].location, yystack_[0].value.as < std::string > ());
                        yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTLetDeclarationStmtNode>(yystack_[4].value.as < ASTNodeBase::Ptr > (), t, yystack_[2].value.as < std::string > ());
                    }
#line 718 "LapsParser.cc"
    break;

  case 11: // decl_attributes: "[" "[" decl_attributes_spec_list "]" "]"
#line 138 "LapsParser.yy"
                    { yylhs.value.as < ASTNodeBase::Ptr > () = yystack_[2].value.as < ASTNodeBase::Ptr > (); }
#line 724 "LapsParser.cc"
    break;

  case 12: // decl_attributes_spec_list: decl_attribute_spec
#line 142 "LapsParser.yy"
                            {
                                yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTDeclAttrSpecListNode>()
                                     ->AppendChild(yystack_[0].value.as < ASTNodeBase::Ptr > ());
                            }
#line 733 "LapsParser.cc"
    break;

  case 13: // decl_attributes_spec_list: decl_attributes_spec_list "," decl_attribute_spec
#line 147 "LapsParser.yy"
                            {
                                yylhs.value.as < ASTNodeBase::Ptr > () = yystack_[2].value.as < ASTNodeBase::Ptr > ()->Cast<ASTDeclAttrSpecListNode>()
                                     ->AppendChild(yystack_[0].value.as < ASTNodeBase::Ptr > ());
                            }
#line 742 "LapsParser.cc"
    break;

  case 14: // decl_attribute_spec: IDENTIFIER "(" decl_attribute_literal_list ")"
#line 154 "LapsParser.yy"
                        {
                            yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTDeclAttrSpec>(yystack_[3].value.as < std::string > (), yystack_[1].value.as < ASTNodeBase::Ptr > ());
                        }
#line 750 "LapsParser.cc"
    break;

  case 15: // decl_attribute_spec: IDENTIFIER
#line 158 "LapsParser.yy"
                        {
                            yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTDeclAttrSpec>(yystack_[0].value.as < std::string > ());
                        }
#line 758 "LapsParser.cc"
    break;

  case 16: // decl_attribute_literal_list: literal_value
#line 164 "LapsParser.yy"
                                {
                                    yylhs.value.as < ASTNodeBase::Ptr > () = ASTNodeBase::New<ASTDeclAttrSpecLiteralListNode>()
                                         ->AppendChild(yystack_[0].value.as < ASTNodeBase::Ptr > ());
                                }
#line 767 "LapsParser.cc"
    break;

  case 17: // decl_attribute_literal_list: decl_attribute_literal_list "," literal_value
#line 169 "LapsParser.yy"
                                {
                                    yylhs.value.as < ASTNodeBase::Ptr > () = yystack_[2].value.as < ASTNodeBase::Ptr > ()->Cast<ASTDeclAttrSpecLiteralListNode>()
                                         ->AppendChild(yystack_[0].value.as < ASTNodeBase::Ptr > ());
                                }
#line 776 "LapsParser.cc"
    break;


#line 780 "LapsParser.cc"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
   LapsParser ::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  const char *
   LapsParser ::symbol_name (symbol_kind_type yysymbol)
  {
    static const char *const yy_sname[] =
    {
    "end of file", "error", "invalid token", "[", "]", "(", ")", ",", ";",
  ":", "let", "LITERAL_DEC_INT", "LITERAL_HEX_INT", "LITERAL_FLOAT",
  "IDENTIFIER", "$accept", "translation_unit", "statement_list",
  "statement", "literal_value", "let_declaration", "decl_attributes",
  "decl_attributes_spec_list", "decl_attribute_spec",
  "decl_attribute_literal_list", YY_NULLPTR
    };
    return yy_sname[yysymbol];
  }



  //  LapsParser ::context.
   LapsParser ::context::context (const  LapsParser & yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
   LapsParser ::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }






  int
   LapsParser ::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
   LapsParser ::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char  LapsParser ::yypact_ninf_ = -11;

  const signed char  LapsParser ::yytable_ninf_ = -1;

  const signed char
   LapsParser ::yypact_[] =
  {
      -2,    -1,   -10,     5,     0,   -11,     3,    -3,     4,     8,
     -11,   -11,   -11,   -11,     6,    14,     2,   -11,     7,    13,
       1,    19,     4,   -11,    10,   -11,   -11,   -11,   -11,     9,
     -11,   -11,   -11,   -11,     1,   -11
  };

  const signed char
   LapsParser ::yydefact_[] =
  {
       0,     0,     0,     0,     0,     3,     0,     0,     0,     0,
       1,     2,     4,     5,     0,    15,     0,    12,     0,     0,
       0,     0,     0,     9,     0,     6,     7,     8,    16,     0,
      11,    13,    10,    14,     0,    17
  };

  const signed char
   LapsParser ::yypgoto_[] =
  {
     -11,   -11,   -11,    21,    -8,   -11,   -11,   -11,    11,   -11
  };

  const signed char
   LapsParser ::yydefgoto_[] =
  {
       0,     3,     4,     5,    28,     6,     7,    16,    17,    29
  };

  const signed char
   LapsParser ::yytable_[] =
  {
      11,     1,     8,     1,     9,    10,    21,    14,     2,    22,
       2,    13,    25,    26,    27,    33,    34,    18,    15,    20,
      19,    23,    24,    30,    32,    12,    35,     0,     0,     0,
       0,     0,     0,    31
  };

  const signed char
   LapsParser ::yycheck_[] =
  {
       0,     3,     3,     3,    14,     0,     4,    10,    10,     7,
      10,     8,    11,    12,    13,     6,     7,     9,    14,     5,
      14,    14,     9,     4,    14,     4,    34,    -1,    -1,    -1,
      -1,    -1,    -1,    22
  };

  const signed char
   LapsParser ::yystos_[] =
  {
       0,     3,    10,    16,    17,    18,    20,    21,     3,    14,
       0,     0,    18,     8,    10,    14,    22,    23,     9,    14,
       5,     4,     7,    14,     9,    11,    12,    13,    19,    24,
       4,    23,    14,     6,     7,    19
  };

  const signed char
   LapsParser ::yyr1_[] =
  {
       0,    15,    16,    17,    17,    18,    19,    19,    19,    20,
      20,    21,    22,    22,    23,    23,    24,    24
  };

  const signed char
   LapsParser ::yyr2_[] =
  {
       0,     2,     2,     1,     2,     2,     1,     1,     1,     4,
       5,     5,     1,     3,     4,     1,     1,     3
  };




#if YYDEBUG
  const unsigned char
   LapsParser ::yyrline_[] =
  {
       0,    83,    83,    90,    95,   102,   105,   111,   117,   125,
     130,   137,   141,   146,   153,   157,   163,   168
  };

  void
   LapsParser ::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
   LapsParser ::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


#line 4 "LapsParser.yy"
} } } //  cocoa::cobalt::laps 
#line 1217 "LapsParser.cc"

#line 175 "LapsParser.yy"

