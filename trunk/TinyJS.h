/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
 *
 * Authored By Gordon Williams <gw@pur3.co.uk>
 *
 * Copyright (C) 2009 Pur3 Ltd
 *

 * 42TinyJS
 *
 * A fork of TinyJS with the goal to makes a more JavaScript/ECMA compliant engine
 *
 * Authored / Changed By Armin Diedering <armin@diedering.de>
 *
 * Copyright (C) 2010-2012 ardisoft
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TINYJS_H
#define TINYJS_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <stdint.h>
#include <cassert>
#include "config.h"

#ifdef NO_POOL_ALLOCATOR
	template<typename T, int num_objects=64>
	class fixed_size_object {};
#else
#	include "pool_allocator.h"
#endif

#ifdef _MSC_VER
#	if defined(_DEBUG) && defined(_DEBUG_NEW)
#		define _AFXDLL
#		include <afx.h>         // MFC-Kern- und -Standardkomponenten
#		define new DEBUG_NEW
#	endif
#	define DEPRECATED(_Text) __declspec(deprecated(_Text))
#elif defined(__GNUC__)
#	define DEPRECATED(_Text) __attribute__ ((deprecated))
#else
#	define DEPRECATED(_Text)
#endif

#ifndef ASSERT
#	define ASSERT(X) assert(X)
#endif

#undef TRACE
#ifndef TRACE
#define TRACE printf
#endif // TRACE

enum LEX_TYPES {
	LEX_EOF = 0,
#define LEX_RELATIONS_1_BEGIN LEX_EQUAL
	LEX_EQUAL = 256,
	LEX_TYPEEQUAL,
	LEX_NEQUAL,
	LEX_NTYPEEQUAL,
#define LEX_RELATIONS_1_END LEX_NTYPEEQUAL
	LEX_LEQUAL,
	LEX_GEQUAL,
#define LEX_SHIFTS_BEGIN LEX_LSHIFT
	LEX_LSHIFT,
	LEX_RSHIFT,
	LEX_RSHIFTU, // unsigned
#define LEX_SHIFTS_END LEX_RSHIFTU
	LEX_PLUSPLUS,
	LEX_MINUSMINUS,
	LEX_ANDAND,
	LEX_OROR,
	LEX_INT,

#define LEX_ASSIGNMENTS_BEGIN LEX_PLUSEQUAL
	LEX_PLUSEQUAL,
	LEX_MINUSEQUAL,
	LEX_ASTERISKEQUAL,
	LEX_SLASHEQUAL,
	LEX_PERCENTEQUAL,
	LEX_LSHIFTEQUAL,
	LEX_RSHIFTEQUAL,
	LEX_RSHIFTUEQUAL, // unsigned
	LEX_ANDEQUAL,
	LEX_OREQUAL,
	LEX_XOREQUAL,
#define LEX_ASSIGNMENTS_END LEX_XOREQUAL

#define LEX_TOKEN_STRING_BEGIN LEX_ID
	LEX_ID,
	LEX_STR,
	LEX_REGEXP,
	LEX_T_LABEL,
	LEX_T_DUMMY_LABEL,
	LEX_T_LOOP_LABEL,
#define LEX_TOKEN_STRING_END LEX_T_LOOP_LABEL

	LEX_FLOAT,

	// reserved words
	LEX_R_IF,
	LEX_R_ELSE,
#define LEX_TOKEN_LOOP_BEGIN LEX_R_DO
	LEX_R_DO,
	LEX_R_WHILE,
	LEX_R_FOR,
	LEX_T_FOR_IN,
	LEX_T_FOR_EACH_IN,
#define LEX_TOKEN_LOOP_END LEX_T_FOR_EACH_IN
	LEX_R_IN,
	LEX_R_BREAK,
	LEX_R_CONTINUE,
	LEX_R_RETURN,
	LEX_R_VAR,
	LEX_R_LET,
	LEX_R_WITH,
	LEX_R_TRUE,
	LEX_R_FALSE,
	LEX_R_NULL,
	LEX_R_NEW,
	LEX_R_TRY,
	LEX_R_CATCH,
	LEX_R_FINALLY,
	LEX_R_THROW,
	LEX_R_TYPEOF,
	LEX_R_VOID,
	LEX_R_DELETE,
	LEX_R_INSTANCEOF,
	LEX_R_SWITCH,
	LEX_R_CASE,
	LEX_R_DEFAULT,

	// special token
//	LEX_T_FILE,
#define LEX_TOKEN_FUNCTION_BEGIN LEX_R_FUNCTION_PLACEHOLDER
	LEX_R_FUNCTION_PLACEHOLDER,
	LEX_R_FUNCTION,
	LEX_T_FUNCTION_OPERATOR,
	LEX_T_FUNCTION_SHORT,
	LEX_T_FUNCTION_OPERATOR_SHORT,
	LEX_T_GET,
	LEX_T_SET,
#define LEX_TOKEN_FUNCTION_END LEX_T_SET

	LEX_T_SKIP,

	LEX_T_FORWARD,
	LEX_T_DESTRUCTURING_VAR,
	LEX_T_OBJECT_LITERAL,
};
#define LEX_TOKEN_DATA_STRING(tk) ((LEX_TOKEN_STRING_BEGIN<= tk && tk <= LEX_TOKEN_STRING_END))
#define LEX_TOKEN_DATA_FLOAT(tk) (tk==LEX_FLOAT)
#define LEX_TOKEN_DATA_FUNCTION(tk) (LEX_TOKEN_FUNCTION_BEGIN <= tk && tk <= LEX_TOKEN_FUNCTION_END)
#define LEX_TOKEN_DATA_OBJECT_LITERAL(tk) (tk==LEX_T_OBJECT_LITERAL)
#define LEX_TOKEN_DATA_DESTRUCTURING_VAR(tk) (tk==LEX_T_DESTRUCTURING_VAR)
#define LEX_TOKEN_DATA_FORWARDER(tk) (tk==LEX_T_FORWARD)

#define LEX_TOKEN_DATA_SIMPLE(tk) (!LEX_TOKEN_DATA_STRING(tk) && !LEX_TOKEN_DATA_FLOAT(tk) && !LEX_TOKEN_DATA_FUNCTION(tk) && !LEX_TOKEN_DATA_OBJECT_LITERAL(tk) && !LEX_TOKEN_DATA_DESTRUCTURING_VAR(tk) && !LEX_TOKEN_DATA_FORWARDER(tk))

enum SCRIPTVARLINK_FLAGS {
	SCRIPTVARLINK_OWNED				= 1<<0,
	SCRIPTVARLINK_WRITABLE			= 1<<1,
	SCRIPTVARLINK_DELETABLE			= 1<<2,
	SCRIPTVARLINK_ENUMERABLE		= 1<<3,
	SCRIPTVARLINK_HIDDEN				= 1<<4,
	SCRIPTVARLINK_DEFAULT			= SCRIPTVARLINK_WRITABLE | SCRIPTVARLINK_DELETABLE | SCRIPTVARLINK_ENUMERABLE,
	SCRIPTVARLINK_VARDEFAULT		= SCRIPTVARLINK_WRITABLE | SCRIPTVARLINK_ENUMERABLE,
	SCRIPTVARLINK_NATIVEDEFAULT	= SCRIPTVARLINK_WRITABLE,
};
enum RUNTIME_FLAGS {
	RUNTIME_BREAK				= 1<<0,
	RUNTIME_CONTINUE			= 1<<1,
	RUNTIME_CAN_THROW			= 1<<2,
	RUNTIME_THROW				= 1<<3,
	RUNTIME_THROW_MASK		= RUNTIME_CAN_THROW | RUNTIME_THROW,
};

enum ERROR_TYPES {
	Error = 0,
	EvalError,
	RangeError,
	ReferenceError,
	SyntaxError,
	TypeError
};
#define ERROR_MAX TypeError
#define ERROR_COUNT (ERROR_MAX+1)
extern const char *ERROR_NAME[];


#define TINYJS_RETURN_VAR					"return"
#define TINYJS_LOKALE_VAR					"__locale__"
#define TINYJS_ANONYMOUS_VAR				"__anonymous__"
#define TINYJS_ARGUMENTS_VAR				"arguments"
#define TINYJS___PROTO___VAR				"__proto__"
#define TINYJS_PROTOTYPE_CLASS			"prototype"
#define TINYJS_FUNCTION_CLOSURE_VAR		"__function_closure__"
#define TINYJS_SCOPE_PARENT_VAR			"__scope_parent__"
#define TINYJS_SCOPE_WITH_VAR				"__scope_with__"
#define TINYJS_ACCESSOR_GET_VAR			"__accessor_get__"
#define TINYJS_ACCESSOR_SET_VAR			"__accessor_set__"
#define TINYJS_TEMP_NAME			""
#define TINYJS_BLANK_DATA			""
#define TINYJS_NEGATIVE_INFINITY_DATA			"-1"
#define TINYJS_POSITIVE_INFINITY_DATA			"+1"

typedef std::vector<std::string> STRING_VECTOR_t;
typedef STRING_VECTOR_t::iterator STRING_VECTOR_it;

typedef std::set<std::string> STRING_SET_t;
typedef STRING_SET_t::iterator STRING_SET_it;

/// convert the given string into a quoted string suitable for javascript
std::string getJSString(const std::string &str);
/// convert the given int into a string
std::string int2string(int intData);
/// convert the given double into a string
std::string float2string(const double &floatData);

//////////////////////////////////////////////////////////////////////////
/// CScriptException
//////////////////////////////////////////////////////////////////////////

class CScriptException {
public:
	ERROR_TYPES errorType;
	std::string message;
	std::string fileName;
	int lineNumber;
	int column;
	CScriptException(const std::string &Message, const std::string &File, int Line=-1, int Column=-1) :
						errorType(Error), message(Message), fileName(File), lineNumber(Line), column(Column){}
	CScriptException(ERROR_TYPES ErrorType, const std::string &Message, const std::string &File, int Line=-1, int Column=-1) :
						errorType(ErrorType), message(Message), fileName(File), lineNumber(Line), column(Column){}
	CScriptException(const std::string &Message, const char *File="", int Line=-1, int Column=-1) :
						errorType(Error), message(Message), fileName(File), lineNumber(Line), column(Column){}
	CScriptException(ERROR_TYPES ErrorType, const std::string &Message, const char *File="", int Line=-1, int Column=-1) :
						errorType(ErrorType), message(Message), fileName(File), lineNumber(Line), column(Column){}
	std::string toString();
};

//////////////////////////////////////////////////////////////////////////
/// CSCRIPTLEX
//////////////////////////////////////////////////////////////////////////

class CScriptLex
{
public:
	CScriptLex(const char *Code, const std::string &File="", int Line=0, int Column=0);
	struct POS;
	int tk; ///< The type of the token that we have
	int last_tk; ///< The type of the last token that we have
	std::string tkStr; ///< Data contained in the token we have here

	void check(int expected_tk, int alternate_tk=-1); ///< Lexical check wotsit
	void match(int expected_tk, int alternate_tk=-1); ///< Lexical match wotsit
	void reset(const POS &toPos); ///< Reset this lex so we can start again

	std::string currentFile;
	struct POS {
		const char *tokenStart;
		int currentLine;
		const char *currentLineStart;
	} pos;
	int currentLine() { return pos.currentLine; }
	int currentColumn() { return pos.tokenStart-pos.currentLineStart; }
	bool lineBreakBeforeToken;
private:
	const char *data;
	const char *dataPos;
	char currCh, nextCh;

	void getNextCh();
	void getNextToken(); ///< Get the text token from our text string
};

// ----------------------------------------------------------------------------------- CSCRIPTTOKEN
class CScriptToken;
typedef  std::vector<CScriptToken> TOKEN_VECT;
typedef  std::vector<CScriptToken>::iterator TOKEN_VECT_it;
typedef  std::vector<CScriptToken>::const_iterator TOKEN_VECT_cit;
class CScriptTokenData
{
protected:
	CScriptTokenData() : refs(0){}
	virtual ~CScriptTokenData() {}
private:
//	CScriptTokenData(const CScriptTokenData &noCopy);
//	CScriptTokenData &operator=(const CScriptTokenData &noCopy);
public:
	void ref() { refs++; }
	void unref() { if(--refs == 0) delete this; }
private:
	int refs;
};
class CScriptTokenDataString : public fixed_size_object<CScriptTokenDataString>, public CScriptTokenData {
public:
	CScriptTokenDataString(const std::string &String) : tokenStr(String) {}
	std::string tokenStr;
private:
};

class CScriptTokenDataFnc : public fixed_size_object<CScriptTokenDataFnc>, public CScriptTokenData {
public:
	std::string file;
	int line;
	std::string name;
	TOKEN_VECT arguments;
	TOKEN_VECT body;
	std::string getArgumentsString();
};

class CScriptTokenDataForwards : public fixed_size_object<CScriptTokenDataForwards>, public CScriptTokenData {
public:
	CScriptTokenDataForwards() {}
	STRING_SET_t vars;
	STRING_SET_t lets;
	STRING_SET_t vars_in_letscope;
	class compare_fnc_token_by_name {
	public:
		bool operator()(const CScriptToken& lhs, const CScriptToken& rhs) const;
	};
	typedef std::set<CScriptToken, compare_fnc_token_by_name> FNC_SET_t;
	typedef FNC_SET_t::iterator FNC_SET_it;
	FNC_SET_t functions;
	bool checkRedefinition(const std::string &Str, bool checkVars);
	void addVars( STRING_VECTOR_t &Vars );
	std::string addVarsInLetscope(STRING_VECTOR_t &Vars);
	std::string addLets(STRING_VECTOR_t &Lets);
private:
};

typedef std::pair<std::string, std::string> DESTRUCTURING_VAR_t;
typedef std::vector<DESTRUCTURING_VAR_t> DESTRUCTURING_VARS_t;
typedef DESTRUCTURING_VARS_t::iterator DESTRUCTURING_VARS_it;
class CScriptTokenDataDestructuringVar : public fixed_size_object<CScriptTokenDataDestructuringVar>, public CScriptTokenData {
public:
	DESTRUCTURING_VARS_t vars;
	void getVarNames(STRING_VECTOR_t Name);
	std::string getParsableString();
private:
};

class CScriptTokenDataObjectLiteral : public fixed_size_object<CScriptTokenDataObjectLiteral>, public CScriptTokenData {
public:
	enum {ARRAY, OBJECT} type;
	int flags;
	struct ELEMENT {
		std::string id;
		TOKEN_VECT value;
	};
	bool destructuring;
	bool structuring;
	std::vector<ELEMENT> elements;
	void setMode(bool Destructuring);
	std::string getParsableString();
private:
};

class CScriptTokenizer;
/*
	a Token needs 8 Byte
	2 Bytes for the Row-Position of the Token
	2 Bytes for the Token self
	and
	4 Bytes for special Datas in an union
			e.g. an int for interger-literals 
			or pointer for double-literals,
			for string-literals or for functions
*/
class CScriptToken : public fixed_size_object<CScriptToken>
{
public:
	CScriptToken() : line(0), column(0), token(0), intData(0) {}
	CScriptToken(CScriptLex *l, int Match=-1, int Alternate=-1);
	CScriptToken(uint16_t Tk, int IntData=0);
	CScriptToken(uint16_t Tk, const std::string &TkStr);
	CScriptToken(const  CScriptToken &Copy) : token(0) { *this = Copy; }
	CScriptToken &operator =(const CScriptToken &Copy);
	~CScriptToken() { clear(); }

	int &Int() { ASSERT(LEX_TOKEN_DATA_SIMPLE(token)); return intData; }
	std::string &String() { ASSERT(LEX_TOKEN_DATA_STRING(token)); return dynamic_cast<CScriptTokenDataString*>(tokenData)->tokenStr; }
	double &Float() { ASSERT(LEX_TOKEN_DATA_FLOAT(token)); return *floatData; }
	CScriptTokenDataFnc &Fnc() { ASSERT(LEX_TOKEN_DATA_FUNCTION(token)); return *dynamic_cast<CScriptTokenDataFnc*>(tokenData); }
	const CScriptTokenDataFnc &Fnc() const { ASSERT(LEX_TOKEN_DATA_FUNCTION(token)); return *dynamic_cast<CScriptTokenDataFnc*>(tokenData); }
	CScriptTokenDataObjectLiteral &Object() { ASSERT(LEX_TOKEN_DATA_OBJECT_LITERAL(token)); return *dynamic_cast<CScriptTokenDataObjectLiteral*>(tokenData); }
	CScriptTokenDataDestructuringVar &DestructuringVar() { ASSERT(LEX_TOKEN_DATA_DESTRUCTURING_VAR(token)); return *dynamic_cast<CScriptTokenDataDestructuringVar*>(tokenData); }
	CScriptTokenDataForwards &Forwarder() { ASSERT(LEX_TOKEN_DATA_FORWARDER(token)); return *dynamic_cast<CScriptTokenDataForwards*>(tokenData); }
	uint16_t			line;
	uint16_t			column;
	uint16_t			token;

	static std::string getParsableString(TOKEN_VECT &Tokens, const std::string &IndentString="", const std::string &Indent="");
	static std::string getParsableString(TOKEN_VECT_it Begin, TOKEN_VECT_it End, const std::string &IndentString="", const std::string &Indent="");
	static std::string getTokenStr( int token, bool *need_space=0 );
	static const char *isReservedWord(int Token);
	static int isReservedWord(const std::string &Str);
private:

	void clear();
	union {
		int										intData;
		double									*floatData;
		CScriptTokenData						*tokenData;
	};
};

// ----------------------------------------------------------------------------------- CSCRIPTTOKENIZER
/*
	the tokenizer converts the code in an Vector with Tokens
*/
class CScriptTokenizer
{
public:
	struct ScriptTokenPosition {
		ScriptTokenPosition(TOKEN_VECT *Tokens) : tokens(Tokens), pos(tokens->begin())/*, currentLine(0)*//*, currentColumn(0)*/ {}
		bool operator ==(const ScriptTokenPosition &eq) { return pos == eq.pos; }
		ScriptTokenPosition &operator =(const ScriptTokenPosition &copy) { 
			tokens=copy.tokens; pos=copy.pos; 
//			currentLine=copy.currentLine; 
			return *this;
		}
		TOKEN_VECT *tokens;
		TOKEN_VECT_it pos;
		int currentLine()		{ return pos->line; }
		int currentColumn()	{ return pos->column; }
	};
	CScriptTokenizer();
	CScriptTokenizer(CScriptLex &Lexer);
	CScriptTokenizer(const char *Code, const std::string &File="", int Line=0, int Column=0);
	void tokenizeCode(CScriptLex &Lexer);

	CScriptToken &getToken() { return *(tokenScopeStack.back().pos); }
	void getNextToken();
	bool check(int ExpectedToken, int AlternateToken=-1);
	void match(int ExpectedToken, int AlternateToken=-1);
	void pushTokenScope(TOKEN_VECT &Tokens);
	ScriptTokenPosition &getPos() { return tokenScopeStack.back(); }
	void setPos(ScriptTokenPosition &TokenPos);
	ScriptTokenPosition &getPrevPos() { return prevPos; }
	void skip(int Tokens);
	int tk; // current Token
	std::string currentFile;
	int currentLine() { return getPos().currentLine();}
	int currentColumn() { return getPos().currentColumn();}
	const std::string &tkStr() { static std::string empty; return LEX_TOKEN_DATA_STRING(getToken().token)?getToken().String():empty; }
private:
	void tokenizeCatch(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeTry(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeSwitch(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeWith(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeWhile(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeDo(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeIf(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeFor(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void _tokenizeDeconstructionVarObject(DESTRUCTURING_VARS_t &Vars, STRING_VECTOR_t &VarNames);
	void _tokenizeDeconstructionVarArray(DESTRUCTURING_VARS_t &Vars, STRING_VECTOR_t &VarNames);
	void _tokenizeDestructionVar(DESTRUCTURING_VARS_t &Vars, const std::string &Path, STRING_VECTOR_t &VarNames);
	CScriptToken tokenizeDestructionVar(STRING_VECTOR_t &VarNames);
	void tokenizeFunction(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeLet(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeVar(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void _tokenizeLiteralObject(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags, int &State);
	void _tokenizeLiteralArray(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags, int &State);
	void tokenizeObjectLiteral(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags, int &State);

	void tokenizeLiteral(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags, int &State);
	void tokenizeMember(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags, int &State);
	void tokenizeFunctionCall(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags, int &State);
	void tokenizeSubExpression(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags, int &State);
	void tokenizeCondition(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags, int &State);
	void tokenizeAssignment(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeAssignment(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags, int &State);
	void tokenizeExpression(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeBlock(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);
	void tokenizeStatement(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks, STRING_VECTOR_t &Labels, STRING_VECTOR_t &LoopLabels, int Flags);

	int pushToken(TOKEN_VECT &Tokens, int Match=-1, int Alternate=-1);
	int pushToken(TOKEN_VECT &Tokens, const CScriptToken &Token);
	CScriptTokenDataForwards &pushForwarder(TOKEN_VECT &Tokens, std::vector<int> &BlockStart);
	void removeEmptyForwarder(TOKEN_VECT &Tokens, std::vector<int> &BlockStart, std::vector<int> &Marks);
	CScriptTokenDataForwards &__getForwarder(TOKEN_VECT &Tokens, int Pos, std::vector<int> &BlockStart, std::vector<int> &Marks);
	void throwTokenNotExpected();
	CScriptLex *l;
	TOKEN_VECT tokens;
	ScriptTokenPosition prevPos;
	std::vector<ScriptTokenPosition> tokenScopeStack;
};


////////////////////////////////////////////////////////////////////////// 
/// forward-declaration
//////////////////////////////////////////////////////////////////////////

class CScriptVar;
class CScriptVarPtr;
template<typename C> class CScriptVarPointer;
class CScriptVarLink;
class CScriptVarLinkPtr;
class CScriptVarLinkWorkPtr;

class CScriptVarPrimitive;
typedef CScriptVarPointer<CScriptVarPrimitive> CScriptVarPrimitivePtr;

class CScriptVarScopeFnc;
typedef CScriptVarPointer<CScriptVarScopeFnc> CFunctionsScopePtr;
typedef void (*JSCallback)(const CFunctionsScopePtr &var, void *userdata);

class CTinyJS;



//////////////////////////////////////////////////////////////////////////
/// CScriptVar
//////////////////////////////////////////////////////////////////////////
typedef	std::vector<class CScriptVarLinkPtr> SCRIPTVAR_CHILDS_t;
typedef	SCRIPTVAR_CHILDS_t::iterator SCRIPTVAR_CHILDS_it;
typedef	SCRIPTVAR_CHILDS_t::const_iterator SCRIPTVAR_CHILDS_cit;

class CScriptVar : public fixed_size_object<CScriptVar> {
protected:
	CScriptVar(CTinyJS *Context, const CScriptVarPtr &Prototype); ///< Create
	CScriptVar(const CScriptVar &Copy); ///< Copy protected -> use clone for public
private:
	CScriptVar & operator=(const CScriptVar &Copy); ///< private -> no assignment-Copy
public:
	virtual ~CScriptVar();
	virtual CScriptVarPtr clone()=0;

	/// Type
	virtual bool isObject();	///< is an Object
	virtual bool isArray();		///< is an Array
	virtual bool isError();		///< is an ErrorObject
	virtual bool isRegExp();	///< is a RegExpObject
	virtual bool isAccessor();	///< is an Accessor
	virtual bool isNull();		///< is Null
	virtual bool isUndefined();///< is Undefined
	virtual bool isNaN();		///< is NaN
	virtual bool isString();	///< is String
	virtual bool isInt();		///< is Integer
	virtual bool isBool();		///< is Bool
	virtual int isInfinity();	///< is Infinity ///< +1==POSITIVE_INFINITY, -1==NEGATIVE_INFINITY, 0==is not an InfinityVar
	virtual bool isDouble();	///< is Double

	virtual bool isRealNumber();	///< is isInt | isDouble
	virtual bool isNumber();	///< is isNaN | isInt | isDouble | isInfinity
	virtual bool isPrimitive();///< isNull | isUndefined | isNaN | isString | isInt | isDouble | isInfinity

	virtual bool isFunction();	///< is CScriptVarFunction / CScriptVarFunctionNativeCallback / CScriptVarFunctionNativeClass
	virtual bool isNative();	///< is CScriptVarFunctionNativeCallback / CScriptVarFunctionNativeClass

	bool isBasic() { return Childs.empty(); } ///< Is this *not* an array/object/etc


	/// Value
	int getInt(); ///< shortcut for this->getPrimitivVar()->getInt()
	int getInt(bool &execute); ///< shortcut for this->getPrimitivVar(execute)->getInt()
	bool getBool(); ///< shortcut for this->getPrimitivVar()->getBool()
	bool getBool(bool &execute); ///< shortcut for this->getPrimitivVar(execute)->getBool()
	double getDouble(); ///< shortcut for this->getPrimitivVar()->getDouble()
	double getDouble(bool &execute); ///< shortcut for this->getPrimitivVar(execute)->getDouble()
	std::string getString(); ///< shortcut for this->getPrimitivVarString()->getString()
	std::string getString(bool &execute); ///< shortcut for this->getPrimitivVarString(execute)->getString()

	virtual CScriptTokenDataFnc *getFunctionData(); ///< { return 0; }

//	virtual std::string getParsableString(const std::string &indentString, const std::string &indent, bool &hasRecursion); ///< get Data as a parsable javascript string
#define getParsableStringRecursionsCheck() do{		\
		if(uniqueID && uniqueID==temporaryID) { hasRecursion=true; return "recursion"; } \
		temporaryID = uniqueID; \
	} while(0)

	virtual std::string getParsableString(const std::string &indentString, const std::string &indent, uint32_t uniqueID, bool &hasRecursion); ///< get Data as a parsable javascript string
	virtual std::string getVarType()=0;
	CScriptVarPrimitivePtr getPrimitivStringVar(); ///< if the var an ObjectType gets the valueOf; if valueOf of an ObjectType gets toString / otherwise gets the Var itself 
	CScriptVarPrimitivePtr getPrimitivStringVar(bool &execute); ///< if the var an ObjectType gets the valueOf; if valueOf of an ObjectType gets toString / otherwise gets the Var itself 
	CScriptVarPrimitivePtr getPrimitivVar(); ///< if the var an ObjectType gets the valueOf; if valueOf of an ObjectType gets toString / otherwise gets the Var itself 
	CScriptVarPrimitivePtr getPrimitivVar(bool &execute); ///< if the var an ObjectType gets the valueOf; if valueOf of an ObjectType gets toString / otherwise gets the Var itself 
	virtual CScriptVarPtr getNumericVar(); ///< returns an Integer, a Double, an Infinity or a NaN
	virtual CScriptVarPtr toObject()=0;
	CScriptVarPtr valueOf(bool &execute);
	virtual CScriptVarPtr _valueOf(bool &execute);
	CScriptVarPtr toString(bool &execute, int radix=0);
	virtual CScriptVarPtr _toString(bool &execute, int radix=0);

	/// flags
	bool isExtensible()			{ return extensible; }
	void setExtensible(bool On)	{ extensible=On; }

	/// find 
	CScriptVarLinkPtr findChild(const std::string &childName); ///< Tries to find a child with the given name, may return 0
	CScriptVarLinkPtr findChildInPrototypeChain(const std::string &childName);
	CScriptVarLinkPtr findChildWithPrototypeChain(const std::string &childName);
	CScriptVarLinkPtr findChildByPath(const std::string &path); ///< Tries to find a child with the given path (separated by dots)
	CScriptVarLinkPtr findChildOrCreate(const std::string &childName/*, int varFlags=SCRIPTVAR_UNDEFINED*/); ///< Tries to find a child with the given name, or will create it with the given flags
	CScriptVarLinkPtr findChildOrCreateByPath(const std::string &path); ///< Tries to find a child with the given path (separated by dots)
	void keys(STRING_SET_t &Keys, bool OnlyEnumerable=true, uint32_t ID=0);
	/// add & remove
	CScriptVarLinkPtr addChild(const std::string &childName, const CScriptVarPtr &child, int linkFlags = SCRIPTVARLINK_DEFAULT);
	CScriptVarLinkPtr DEPRECATED("addChildNoDup is deprecated use addChildOrReplace instead!") addChildNoDup(const std::string &childName, const CScriptVarPtr &child, int linkFlags = SCRIPTVARLINK_DEFAULT);
	CScriptVarLinkPtr addChildOrReplace(const std::string &childName, const CScriptVarPtr &child, int linkFlags = SCRIPTVARLINK_DEFAULT); ///< add a child overwriting any with the same name
	bool removeLink(CScriptVarLinkPtr &link); ///< Remove a specific link (this is faster than finding via a child)
	void removeAllChildren();

	/// ARRAY
	CScriptVarPtr getArrayIndex(int idx); ///< The the value at an array index
	void setArrayIndex(int idx, const CScriptVarPtr &value); ///< Set the value at an array index
	int getArrayLength(); ///< If this is an array, return the number of items in it (else 0)
	
	//////////////////////////////////////////////////////////////////////////
	int getChildren() { return Childs.size(); } ///< Get the number of children
	CTinyJS *getContext() { return context; }
	CScriptVarPtr mathsOp(const CScriptVarPtr &b, int op); ///< do a maths op with another script variable

	void trace(const std::string &name = ""); ///< Dump out the contents of this using trace
	void trace(std::string &indentStr, uint32_t uniqueID, const std::string &name = ""); ///< Dump out the contents of this using trace
	std::string getFlagsAsString(); ///< For debugging - just dump a string version of the flags
//	void getJSON(std::ostringstream &destination, const std::string linePrefix=""); ///< Write out all the JS code needed to recreate this script variable to the stream (as JSON)

	SCRIPTVAR_CHILDS_t Childs;

	/// For memory management/garbage collection
private:
	CScriptVar *ref(); ///< Add reference to this variable
	void unref(); ///< Remove a reference, and delete this variable if required
public:
	int getRefs(); ///< Get the number of references to this script variable
	template<class T>
	operator T *(){ T *ret = dynamic_cast<T*>(this); ASSERT(ret!=0); return ret; }
	template<class T>
	T *get(){ T *ret = dynamic_cast<T*>(this); ASSERT(ret!=0); return ret; }

	template<typename T>	CScriptVarPtr newScriptVar(T t); // { return ::newScriptVar(context, t); }
	template<typename T>	const CScriptVarPtr &constScriptVar(T t); // { return ::newScriptVar(context, t); }
	void setTemporaryID(uint32_t ID) { temporaryID = ID; }
	virtual void setTemporaryID_recursive(uint32_t ID);
	uint32_t getTempraryID() { return temporaryID; }
protected:
	bool extensible;
	CTinyJS *context;
	int refs; ///< The number of references held to this - used for garbage collection
	CScriptVar *prev;
public:
	CScriptVar *next;
	uint32_t temporaryID;

	friend class CScriptVarPtr;
};

////////////////////////////////////////////////////////////////////////// 
/// CScriptVarPtr 
//////////////////////////////////////////////////////////////////////////

class CScriptVarPtr { 
public: 
	// construct
	CScriptVarPtr() : var(0) {} ///< 0-Pointer 
	CScriptVarPtr(CScriptVar *Var) : var(Var) { if(var) var->ref(); } // creates a new CScriptVar (from new);

	// copy
	CScriptVarPtr(const CScriptVarPtr &Copy) : var(Copy.var) { if(var) var->ref(); } 
	CScriptVarPtr& operator=(const CScriptVarPtr &Copy) { 
		if(var != Copy.var) { 
			if(var) var->unref(); 
			var = Copy.var; if(var) var->ref(); 
		} 
		return *this; 
	}
	// deconstruct 
	~CScriptVarPtr() { if(var) var->unref(); } 

	// if
	operator bool() const { return var!=0; } 

	bool operator ==(const CScriptVarPtr &Other) const { return var == Other.var; } 
	bool operator !=(const CScriptVarPtr &Other) const { return var != Other.var; } 

	// access
	CScriptVar * operator ->() const { return var; } 
	CScriptVar *getVar() const { return var; } 

	void clear() { if(var) var->unref(); var=0; }
protected: 
	CScriptVar *var; 
}; 

////////////////////////////////////////////////////////////////////////// 
/// CScriptVarPointer - template
//////////////////////////////////////////////////////////////////////////

template<typename C> 
class CScriptVarPointer : public CScriptVarPtr { 
public:
	CScriptVarPointer() {}
	CScriptVarPointer(const CScriptVarPtr &Copy) : CScriptVarPtr(Copy) { if(var) { var = dynamic_cast<C*>(var); } }
	C * operator ->() const { C *Var = dynamic_cast<C*>(var); ASSERT(var && Var); return Var; }
};

//////////////////////////////////////////////////////////////////////////
/// CScriptVarLink
//////////////////////////////////////////////////////////////////////////

class CScriptVarLink : public fixed_size_object<CScriptVarLink>
{
private: // prevent gloabal creating
	CScriptVarLink(const CScriptVarPtr &var, const std::string &name = TINYJS_TEMP_NAME, int flags = SCRIPTVARLINK_DEFAULT);
private: // prevent Copy
	CScriptVarLink(const CScriptVarLink &link); ///< Copy constructor
public:
	~CScriptVarLink();

	const std::string &getName() const { return name; }

	int getFlags() { return flags; }
	const CScriptVarPtr &getVarPtr() const { return var; }
	const CScriptVarPtr &setVarPtr(const CScriptVarPtr &Var) { return var = Var; } ///< simple Replace the Variable pointed to


	bool isOwned() const { return (flags & SCRIPTVARLINK_OWNED) != 0; }
	void setOwned(bool On) { On ? (flags |= SCRIPTVARLINK_OWNED) : (flags &= ~SCRIPTVARLINK_OWNED); }

	bool isOwner() const { return owner!=0; }

	bool isWritable() const { return (flags & SCRIPTVARLINK_WRITABLE) != 0; }
	void setWritable(bool On) { On ? (flags |= SCRIPTVARLINK_WRITABLE) : (flags &= ~SCRIPTVARLINK_DELETABLE); }
	bool isDeletable() const { return (flags & SCRIPTVARLINK_DELETABLE) != 0; }
	void setDeletable(bool On) { On ? (flags |= SCRIPTVARLINK_DELETABLE) : (flags &= ~SCRIPTVARLINK_DELETABLE); }
	bool isEnumerable() const { return (flags & SCRIPTVARLINK_ENUMERABLE) != 0; }
	void setEnumerable(bool On) { On ? (flags |= SCRIPTVARLINK_ENUMERABLE) : (flags &= ~SCRIPTVARLINK_ENUMERABLE); }
	bool isHidden() const { return (flags & SCRIPTVARLINK_HIDDEN) != 0; }
	void setHidden(bool On) { On ? (flags |= SCRIPTVARLINK_HIDDEN) : (flags &= ~SCRIPTVARLINK_HIDDEN); }

	CScriptVar *getOwner() { return owner; };
	void setOwner(CScriptVar *Owner) { owner = Owner; }

private:
	std::string name;
	CScriptVar *owner; // pointer to the owner CScriptVar
	uint32_t flags;
	CScriptVarPtr var;
#ifdef _DEBUG
	char dummy[24];
#endif
	CScriptVarLink *ref();
	void unref();
private:
	int refs;
	friend class CScriptVarLinkPtr;
};

//////////////////////////////////////////////////////////////////////////
/// CScriptVarLinkPtr
//////////////////////////////////////////////////////////////////////////

class CScriptVarLinkPtr { 
public: 
	// construct
	CScriptVarLinkPtr() : link(0) {} ///< 0-Pointer 
	CScriptVarLinkPtr(const CScriptVarPtr &var, const std::string &name = TINYJS_TEMP_NAME, int flags = SCRIPTVARLINK_DEFAULT) { link=(new CScriptVarLink(var, name, flags))->ref(); }
	CScriptVarLinkPtr(CScriptVarLink *Link) : link(Link) { if(link) link->ref(); } // creates a new CScriptVarLink (from new);

	// reconstruct
	CScriptVarLinkPtr &operator()(const CScriptVarPtr &var, const std::string &name = TINYJS_TEMP_NAME, int flags = SCRIPTVARLINK_DEFAULT);

	// deconstruct 
	~CScriptVarLinkPtr() { if(link) link->unref(); } 

	// copy
	CScriptVarLinkPtr(const CScriptVarLinkPtr &Copy) : link(Copy.link) { if(link) link->ref(); } 
	CScriptVarLinkPtr &operator=(const CScriptVarLinkPtr &Copy) { 
		if(link != Copy.link) { 
			if(link) link->unref(); 
			link = Copy.link; if(link) link->ref(); 
		} 
		return *this; 
	}

	// getter & setter
	CScriptVarLinkWorkPtr getter();
	CScriptVarLinkWorkPtr getter(bool &execute);
	CScriptVarLinkWorkPtr setter(const CScriptVarPtr &Var);
	CScriptVarLinkWorkPtr setter(bool &execute, const CScriptVarPtr &Var);

	// if
	operator bool() const { return link!=0; } 

	// for sorting in child-list
	bool operator <(const std::string &rhs) const;

	// access to CScriptVarLink
	CScriptVarLink *operator ->() const { return link; } 

	operator const CScriptVarPtr &() const { static CScriptVarPtr NullPtr; return link?link->getVarPtr():NullPtr; }

	void clear() { if(link) link->unref(); link=0; }
protected: 
	CScriptVarLink *link; 
}; 

//////////////////////////////////////////////////////////////////////////
/// CScriptVarLinkWorkPtr
//////////////////////////////////////////////////////////////////////////

class CScriptVarLinkWorkPtr : public CScriptVarLinkPtr {
public:
	// construct
	CScriptVarLinkWorkPtr() {}
	CScriptVarLinkWorkPtr(const CScriptVarPtr &var, const std::string &name = TINYJS_TEMP_NAME, int flags = SCRIPTVARLINK_DEFAULT) : CScriptVarLinkPtr(var, name, flags) {}
	CScriptVarLinkWorkPtr(CScriptVarLink *Link) : CScriptVarLinkPtr(Link) { if(link) referencedOwner = link->getOwner(); } // creates a new CScriptVarLink (from new);
	CScriptVarLinkWorkPtr(const CScriptVarLinkPtr &Copy) : CScriptVarLinkPtr(Copy) { if(link) referencedOwner = link->getOwner(); } 
	// reconstruct
	CScriptVarLinkWorkPtr &operator()(const CScriptVarPtr &var, const std::string &name = TINYJS_TEMP_NAME, int flags = SCRIPTVARLINK_DEFAULT) {CScriptVarLinkPtr::operator()(var, name, flags); referencedOwner.clear(); return *this; }

	// getter & setter
	CScriptVarLinkWorkPtr getter();
	CScriptVarLinkWorkPtr getter(bool &execute);
	CScriptVarLinkWorkPtr setter(const CScriptVarPtr &Var);
	CScriptVarLinkWorkPtr setter(bool &execute, const CScriptVarPtr &Var);


	void swap(CScriptVarLinkWorkPtr &Link) { 
		CScriptVarPtr _referencedOwner = referencedOwner; referencedOwner = Link.referencedOwner; Link.referencedOwner = _referencedOwner;
		CScriptVarLink *_link=link; link=Link.link; Link.link=_link; 
	}

	void clear() { CScriptVarLinkPtr::clear(); referencedOwner.clear(); }
	void setReferencedOwner(const CScriptVarPtr &Owner) { referencedOwner = Owner; }
	const CScriptVarPtr &getReferencedOwner() { return referencedOwner; }
private:
	CScriptVarPtr referencedOwner;
};


//////////////////////////////////////////////////////////////////////////
#define define_dummy_t(t1) struct t1##_t{}; extern t1##_t t1
#define declare_dummy_t(t1) t1##_t t1
#define define_newScriptVar_Fnc(t1, ...) CScriptVarPtr newScriptVar(__VA_ARGS__)
#define define_newScriptVar_NamedFnc(t1, ...) CScriptVarPtr newScriptVar##t1(__VA_ARGS__)
#define define_ScriptVarPtr_Type(t1) class CScriptVar##t1; typedef CScriptVarPointer<CScriptVar##t1> CScriptVar##t1##Ptr

#define define_DEPRECATED_newScriptVar_Fnc(t1, ...) CScriptVarPtr DEPRECATED("newScriptVar("#__VA_ARGS__") is deprecated use constScriptVar("#__VA_ARGS__") instead") newScriptVar(__VA_ARGS__)


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarObject
//////////////////////////////////////////////////////////////////////////

define_dummy_t(Object);
define_ScriptVarPtr_Type(Object);

class CScriptVarObject : public CScriptVar {
protected:
	CScriptVarObject(CTinyJS *Context);
	CScriptVarObject(CTinyJS *Context, const CScriptVarPtr &Prototype) : CScriptVar(Context, Prototype) {}
	CScriptVarObject(const CScriptVarObject &Copy) : CScriptVar(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarObject();
	virtual CScriptVarPtr clone();

	virtual bool isObject(); // { return true; }

	virtual std::string getParsableString(const std::string &indentString, const std::string &indent, uint32_t uniqueID, bool &hasRecursion);
	virtual std::string getVarType(); ///< always "object"
	virtual CScriptVarPtr toObject();

	virtual CScriptVarPtr _toString(bool &execute, int radix=0);
private:
	friend define_newScriptVar_Fnc(Object, CTinyJS *Context, Object_t);
	friend define_newScriptVar_Fnc(Object, CTinyJS *Context, Object_t, const CScriptVarPtr &);
};
inline define_newScriptVar_Fnc(Object, CTinyJS *Context, Object_t) { return new CScriptVarObject(Context); }
inline define_newScriptVar_Fnc(Object, CTinyJS *Context, Object_t, const CScriptVarPtr &Prototype) { return new CScriptVarObject(Context, Prototype); }


//////////////////////////////////////////////////////////////////////////
/// CScriptVarPrimitive
//////////////////////////////////////////////////////////////////////////

define_ScriptVarPtr_Type(Primitive);
class CScriptVarPrimitive : public CScriptVar {
protected:
	CScriptVarPrimitive(CTinyJS *Context, const CScriptVarPtr &Prototype) : CScriptVar(Context, Prototype), fakeObject(false) { setExtensible(false); }
	CScriptVarPrimitive(const CScriptVarPrimitive &Copy) : CScriptVar(Copy) { fakeObject=Copy.fakeObject; } ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarPrimitive();

	virtual bool isObject();		///< false by default / true by a faked Object
	virtual bool isPrimitive();	///< always true 

	int getInt();
	virtual int _getInt();
	bool getBool();
	virtual bool _getBool();
	double getDouble();
	virtual double _getDouble();
	std::string getString();
	virtual std::string _getString(); 
	

	virtual CScriptVarPtr toObject();
	virtual CScriptVarPrimitivePtr _toObject();
//	virtual CScriptVarPtr _valueOf(bool &execute);
	std::string getVarType(const char *typeStr); ///< { return fakeObject ? "object" : typeStr; }
protected:
	bool fakeObject;
};


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarError
////////////////////////////////////////////////////////////////////////// 

define_ScriptVarPtr_Type(Error);

class CScriptVarError : public CScriptVarObject {
protected:
	CScriptVarError(CTinyJS *Context, ERROR_TYPES type, const char *message, const char *file, int line, int column);// : CScriptVarObject(Context), value(Value) {}
	CScriptVarError(const CScriptVarError &Copy) : CScriptVarObject(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarError();
	virtual CScriptVarPtr clone();
	virtual bool isError(); // { return true; }

//	virtual std::string getParsableString(const std::string &indentString, const std::string &indent); ///< get Data as a parsable javascript string

	virtual CScriptVarPtr _toString(bool &execute, int radix=0);
private:
	friend define_newScriptVar_NamedFnc(Error, CTinyJS *Context, ERROR_TYPES type, const char *message, const char *file, int line, int column);
};
inline define_newScriptVar_NamedFnc(Error, CTinyJS *Context, ERROR_TYPES type, const char *message=0, const char *file=0, int line=-1, int column=-1) { return new CScriptVarError(Context, type, message, file, line, column); }

#if 0
////////////////////////////////////////////////////////////////////////// 
/// CScriptVarAccessor
//////////////////////////////////////////////////////////////////////////

define_dummy_t(Accessor);
define_ScriptVarPtr_Type(Accessor);

class CScriptVarAccessor : public CScriptVarObject {
protected:
	CScriptVarAccessor(CTinyJS *Context);
	CScriptVarAccessor(CTinyJS *Context, JSCallback getter, void *getterdata, JSCallback setter, void *setterdata);
	template<class C>	CScriptVarAccessor(CTinyJS *Context, C *class_ptr, void(C::*getterFnc)(const CFunctionsScopePtr &, void *), void *getterData, void(C::*setterFnc)(const CFunctionsScopePtr &, void *), void *setterData) : CScriptVarObject(Context) {
		if(getterFnc)
			addChild(TINYJS_ACCESSOR_GET_VAR, ::newScriptVar(Context, class_ptr, getterFnc, getterData), 0);
		if(setterFnc)
			addChild(TINYJS_ACCESSOR_SET_VAR, ::newScriptVar(Context, class_ptr, setterFnc, setterData), 0);
	}

	CScriptVarAccessor(const CScriptVarAccessor &Copy) : CScriptVarObject(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarAccessor();
	virtual CScriptVarPtr clone();
	virtual bool isAccessor(); // { return true; }
	virtual bool isPrimitive(); // { return false; } 

	virtual std::string getString(); // { return "[ Object ]"; };
	virtual std::string getParsableString(const std::string &indentString, const std::string &indent, uint32_t uniqueID, bool &hasRecursion);
	virtual std::string getVarType(); // { return "object"; }

	CScriptVarPtr getValue();

	friend define_newScriptVar_Fnc(Accessor, CTinyJS *Context, Accessor_t);
	friend define_newScriptVar_NamedFnc(Accessor, CTinyJS *Context, JSCallback getter, void *getterdata, JSCallback setter, void *setterdata);
	template<class C> friend define_newScriptVar_NamedFnc(Accessor, CTinyJS *Context, C *class_ptr, void(C::*getterFnc)(const CFunctionsScopePtr &, void *), void *getterData, void(C::*setterFnc)(const CFunctionsScopePtr &, void *), void *setterData);
};
inline define_newScriptVar_Fnc(Accessor, CTinyJS *Context, Accessor_t) { return new CScriptVarAccessor(Context); }
inline define_newScriptVar_NamedFnc(Accessor, CTinyJS *Context, JSCallback getter, void *getterdata, JSCallback setter, void *setterdata) { return new CScriptVarAccessor(Context, getter, getterdata, setter, setterdata); }
template<class C> define_newScriptVar_NamedFnc(Accessor, CTinyJS *Context, C *class_ptr, void(C::*getterFnc)(const CFunctionsScopePtr &, void *), void *getterData, void(C::*setterFnc)(const CFunctionsScopePtr &, void *), void *setterData)  { return new CScriptVarAccessor(Context, class_ptr, getterFnc, getterData, setterFnc, setterData); }

#endif
////////////////////////////////////////////////////////////////////////// 
/// CScriptVarArray
//////////////////////////////////////////////////////////////////////////

define_dummy_t(Array);
define_ScriptVarPtr_Type(Array);
class CScriptVarArray : public CScriptVarObject {
protected:
	CScriptVarArray(CTinyJS *Context);
	CScriptVarArray(const CScriptVarArray &Copy) : CScriptVarObject(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarArray();
	virtual CScriptVarPtr clone();
	virtual bool isArray(); // { return true; }

	virtual std::string getParsableString(const std::string &indentString, const std::string &indent, uint32_t uniqueID, bool &hasRecursion);

	virtual CScriptVarPtr _toString(bool &execute, int radix=0);

	friend define_newScriptVar_Fnc(Array, CTinyJS *Context, Array_t);
private:
	void native_Length(const CFunctionsScopePtr &c, void *data);
};
inline define_newScriptVar_Fnc(Array, CTinyJS *Context, Array_t) { return new CScriptVarArray(Context); } 


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarNull
//////////////////////////////////////////////////////////////////////////

define_dummy_t(Null);
define_ScriptVarPtr_Type(Null);
class CScriptVarNull : public CScriptVarPrimitive {
protected:
	CScriptVarNull(CTinyJS *Context);
	CScriptVarNull(const CScriptVarNull &Copy) : CScriptVarPrimitive(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarNull();
	virtual CScriptVarPtr clone();

	virtual bool isNull(); // { return true; }

	virtual std::string _getString(); // { return "null"; };

	virtual std::string getVarType(); // { return "null"; }
	virtual CScriptVarPtr getNumericVar(); ///< returns an Integer, a Double, an Infinity or a NaN

	friend define_DEPRECATED_newScriptVar_Fnc(Null, CTinyJS *Context, Null_t);
	friend define_newScriptVar_NamedFnc(Null, CTinyJS *Context);
};
inline define_DEPRECATED_newScriptVar_Fnc(Null, CTinyJS *Context, Null_t) { return new CScriptVarNull(Context); }
inline define_newScriptVar_NamedFnc(Null, CTinyJS *Context) { return new CScriptVarNull(Context); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarUndefined
//////////////////////////////////////////////////////////////////////////

define_dummy_t(Undefined);
define_ScriptVarPtr_Type(Undefined);
class CScriptVarUndefined : public CScriptVarPrimitive {
protected:
	CScriptVarUndefined(CTinyJS *Context);
	CScriptVarUndefined(const CScriptVarUndefined &Copy) : CScriptVarPrimitive(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarUndefined();
	virtual CScriptVarPtr clone();

	virtual bool isUndefined(); // { return true; }

	virtual std::string _getString(); // { return "undefined"; };
	virtual std::string getVarType(); // { return "undefined"; }
	friend define_DEPRECATED_newScriptVar_Fnc(Undefined, CTinyJS *, Undefined_t);
	friend define_newScriptVar_NamedFnc(Undefined, CTinyJS *Context);
};
inline define_DEPRECATED_newScriptVar_Fnc(Undefined, CTinyJS *Context, Undefined_t) { return new CScriptVarUndefined(Context); }
inline define_newScriptVar_NamedFnc(Undefined, CTinyJS *Context) { return new CScriptVarUndefined(Context); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarNaN
//////////////////////////////////////////////////////////////////////////

define_dummy_t(NaN);
define_ScriptVarPtr_Type(NaN);
class CScriptVarNaN : public CScriptVarPrimitive {
protected:
	CScriptVarNaN(CTinyJS *Context);
	CScriptVarNaN(const CScriptVarNaN &Copy) : CScriptVarPrimitive(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarNaN();
	virtual CScriptVarPtr clone();
	virtual bool isNaN();// { return true; }
	virtual bool isNumber(); // { return true; }
	virtual std::string _getString(); // { return "NaN"; };
	virtual std::string getVarType(); // { return "number"; }
	virtual CScriptVarPrimitivePtr _toObject();
	virtual CScriptVarPtr _valueOf(bool &execute);

	friend define_DEPRECATED_newScriptVar_Fnc(NaN, CTinyJS *, NaN_t);
	friend define_newScriptVar_NamedFnc(NaN, CTinyJS *Context);
};
inline define_DEPRECATED_newScriptVar_Fnc(NaN, CTinyJS *Context, NaN_t) { return new CScriptVarNaN(Context); }
inline define_newScriptVar_NamedFnc(NaN, CTinyJS *Context) { return new CScriptVarNaN(Context); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarString
//////////////////////////////////////////////////////////////////////////

define_ScriptVarPtr_Type(String);
class CScriptVarString : public CScriptVarPrimitive {
protected:
	CScriptVarString(CTinyJS *Context, const std::string &Data);
	CScriptVarString(const CScriptVarString &Copy) : CScriptVarPrimitive(Copy), data(Copy.data) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarString();
	virtual CScriptVarPtr clone();
	virtual bool isString(); // { return true; }

	virtual int _getInt(); // {return strtol(data.c_str(),0,0); }
	virtual bool _getBool(); // {return data.length()!=0;}
	virtual double _getDouble(); // {return strtod(data.c_str(),0);}
	virtual std::string _getString(); // { return data; }
	virtual std::string getParsableString(const std::string &indentString, const std::string &indent, uint32_t uniqueID, bool &hasRecursion); // { return getJSString(data); }
	virtual std::string getVarType(); // { return "string"; }
	virtual CScriptVarPrimitivePtr _toObject();
	virtual CScriptVarPtr _valueOf(bool &execute);
	virtual CScriptVarPtr getNumericVar(); ///< returns an Integer, a Double, an Infinity or a NaN
	int getChar(int Idx);
protected:
	std::string data;
private:
	void native_Length(const CFunctionsScopePtr &c, void *data);

	friend define_newScriptVar_Fnc(String, CTinyJS *Context, const std::string &);
	friend define_newScriptVar_Fnc(String, CTinyJS *Context, const char *);
	friend define_newScriptVar_Fnc(String, CTinyJS *Context, char *);
};
inline define_newScriptVar_Fnc(String, CTinyJS *Context, const std::string &Obj) { return new CScriptVarString(Context, Obj); }
inline define_newScriptVar_Fnc(String, CTinyJS *Context, const char *Obj) { return new CScriptVarString(Context, Obj); }
inline define_newScriptVar_Fnc(String, CTinyJS *Context, char *Obj) { return new CScriptVarString(Context, Obj); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarRegExp
//////////////////////////////////////////////////////////////////////////
#ifndef NO_REGEXP

define_ScriptVarPtr_Type(RegExp);
class CScriptVarRegExp : public CScriptVarObject {
protected:
	CScriptVarRegExp(CTinyJS *Context, const std::string &Source, const std::string &Flags);
	CScriptVarRegExp(const CScriptVarRegExp &Copy) : CScriptVarObject(Copy), regexp(Copy.regexp), flags(Copy.flags) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarRegExp();
	virtual CScriptVarPtr clone();
	virtual bool isRegExp(); // { return true; }
	virtual std::string getString(); // { return regexp; }
//	virtual std::string getParsableString(const std::string &indentString, const std::string &indent, uint32_t uniqueID, bool &hasRecursion); // { return getJSString(regexp); }
	virtual CScriptVarPtr _toString(bool &execute, int radix=0);

	CScriptVarPtr exec(const std::string &Input, bool Test=false);

	bool Global() { return flags.find('g')!=std::string::npos; }
	bool IgnoreCase() { return flags.find('i')!=std::string::npos; }
	bool Multiline() { return true; /* currently always true -- flags.find('m')!=std::string::npos;*/ }
	bool Sticky() { return flags.find('y')!=std::string::npos; }
	const std::string &Regexp() { return regexp; }
	unsigned int LastIndex();
	void LastIndex(unsigned int Idx);

	static const char *ErrorStr(int Error);
protected:
	std::string regexp;
	std::string flags;
private:
	void native_Global(const CFunctionsScopePtr &c, void *data);
	void native_IgnoreCase(const CFunctionsScopePtr &c, void *data);
	void native_Multiline(const CFunctionsScopePtr &c, void *data);
	void native_Sticky(const CFunctionsScopePtr &c, void *data);
	void native_Source(const CFunctionsScopePtr &c, void *data);

	friend define_newScriptVar_Fnc(RegExp, CTinyJS *Context, const std::string &, const std::string &);

};
inline define_newScriptVar_Fnc(RegExp, CTinyJS *Context, const std::string &Obj, const std::string &Flags) { return new CScriptVarRegExp(Context, Obj, Flags); }
#endif /* NO_REGEXP */


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarIntegerBase
//////////////////////////////////////////////////////////////////////////

class CScriptVarIntegerBase : public CScriptVarPrimitive {
protected:
	CScriptVarIntegerBase(CTinyJS *Context, const CScriptVarPtr &Prototype, int Data);
	CScriptVarIntegerBase(const CScriptVarIntegerBase &Copy) : CScriptVarPrimitive(Copy), data(Copy.data) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarIntegerBase();
	virtual bool isNumber(); // { return true; }
	virtual int _getInt(); // {return data; }
	virtual bool _getBool(); // {return data!=0;}
	virtual double _getDouble(); // {return data;}
	virtual std::string _getString(); // {return int2string(data);}
	virtual std::string getVarType(); // { return "number"; }
//	virtual CScriptVarPrimitivePtr _toObject();
//	virtual CScriptVarPtr _valueOf(bool &execute);
	virtual CScriptVarPtr getNumericVar(); ///< returns an Integer, a Double, an Infinity or a NaN
protected:
	int data;
};


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarInteger
//////////////////////////////////////////////////////////////////////////

define_dummy_t(Zero);
define_dummy_t(One);
define_ScriptVarPtr_Type(Integer);
class CScriptVarInteger : public CScriptVarIntegerBase {
protected:
	CScriptVarInteger(CTinyJS *Context, int Data);
	CScriptVarInteger(const CScriptVarInteger &Copy) : CScriptVarIntegerBase(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarInteger();
	virtual CScriptVarPtr clone();
	virtual bool isRealNumber(); // { return true; }
	virtual bool isInt(); // { return true; }

	virtual CScriptVarPrimitivePtr _toObject();
	virtual CScriptVarPtr _valueOf(bool &execute);
	virtual CScriptVarPtr _toString(bool &execute, int radix=0);
	friend define_newScriptVar_Fnc(Integer, CTinyJS *Context, int);
	friend define_newScriptVar_Fnc(Integer, CTinyJS *Context, char);
};
inline define_newScriptVar_Fnc(Integer, CTinyJS *Context, int Obj) { return new CScriptVarInteger(Context, Obj); }
inline define_newScriptVar_Fnc(Integer, CTinyJS *Context, char Obj) { return new CScriptVarInteger(Context, Obj); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarBool
//////////////////////////////////////////////////////////////////////////

define_ScriptVarPtr_Type(Bool);
class CScriptVarBool : public CScriptVarIntegerBase {
protected:
	CScriptVarBool(CTinyJS *Context, bool Data);
	CScriptVarBool(const CScriptVarBool &Copy) : CScriptVarIntegerBase(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarBool();
	virtual CScriptVarPtr clone();
	virtual bool isBool(); // { return true; }
	bool isNumber(); // { return false }
	virtual std::string _getString(); // {return data!=0?"true":"false";}
	virtual std::string getVarType(); // { return "boolean"; }
	virtual CScriptVarPtr getNumericVar(); ///< returns an Integer, a Double, an Infinity or a NaN

	virtual CScriptVarPrimitivePtr _toObject();
	virtual CScriptVarPtr _valueOf(bool &execute);

	friend define_DEPRECATED_newScriptVar_Fnc(Bool, CTinyJS *, bool);
	friend define_newScriptVar_NamedFnc(Bool, CTinyJS *Context, bool);
};
inline define_DEPRECATED_newScriptVar_Fnc(Bool, CTinyJS *Context, bool Obj) { return new CScriptVarBool(Context, Obj); }
inline define_newScriptVar_NamedFnc(Bool, CTinyJS *Context, bool Obj) { return new CScriptVarBool(Context, Obj); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarInfinity
//////////////////////////////////////////////////////////////////////////

class Infinity{public:Infinity(int Sig=1):sig(Sig){} int Sig(){return sig;} private:int sig; } ;
extern Infinity InfinityPositive;
extern Infinity InfinityNegative;
define_ScriptVarPtr_Type(Infinity);
class CScriptVarInfinity : public CScriptVarIntegerBase {
protected:
	CScriptVarInfinity(CTinyJS *Context, int Data);
	CScriptVarInfinity(const CScriptVarInfinity &Copy) : CScriptVarIntegerBase(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarInfinity();
	virtual CScriptVarPtr clone();
	virtual int isInfinity(); // { return data; }
	virtual std::string _getString(); // {return data<0?"-Infinity":"Infinity";}

	virtual CScriptVarPrimitivePtr _toObject();
	virtual CScriptVarPtr _valueOf(bool &execute);

	friend define_DEPRECATED_newScriptVar_Fnc(Infinity, CTinyJS *, Infinity);
	friend define_newScriptVar_NamedFnc(Infinity, CTinyJS *, Infinity);
};
inline define_DEPRECATED_newScriptVar_Fnc(Infinity, CTinyJS *Context, Infinity Obj) { return new CScriptVarInfinity(Context, Obj.Sig()); } 
inline define_newScriptVar_NamedFnc(Infinity, CTinyJS *Context, Infinity Obj) { return new CScriptVarInfinity(Context, Obj.Sig()); } 


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarDouble
//////////////////////////////////////////////////////////////////////////

define_ScriptVarPtr_Type(Double);
class CScriptVarDouble : public CScriptVarPrimitive {
protected:
	CScriptVarDouble(CTinyJS *Context, double Data);
	CScriptVarDouble(const CScriptVarDouble &Copy) : CScriptVarPrimitive(Copy), data(Copy.data) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarDouble();
	virtual CScriptVarPtr clone();
	virtual bool isDouble(); // { return true; }
	virtual bool isRealNumber(); // { return true; }
	virtual bool isNumber(); // { return true; }
	virtual int _getInt(); // {return (int)data; }
	virtual bool _getBool(); // {return data!=0.0;}
	virtual double _getDouble(); // {return data;}
	virtual std::string _getString(); // {return float2string(data);}
	virtual std::string getVarType(); // { return "number"; }
	virtual CScriptVarPrimitivePtr _toObject();
	virtual CScriptVarPtr _valueOf(bool &execute);
	virtual CScriptVarPtr getNumericVar(); ///< returns an Integer, a Double, an Infinity or a NaN
	virtual CScriptVarPtr _toString(bool &execute, int radix=0);
private:
	double data;
	friend define_newScriptVar_Fnc(Double, CTinyJS *Context, double);
};
inline define_newScriptVar_Fnc(Double, CTinyJS *Context, double Obj) { return new CScriptVarDouble(Context, Obj); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarFunction
//////////////////////////////////////////////////////////////////////////

define_ScriptVarPtr_Type(Function);
class CScriptVarFunction : public CScriptVarObject {
protected:
	CScriptVarFunction(CTinyJS *Context, CScriptTokenDataFnc *Data);
	CScriptVarFunction(const CScriptVarFunction &Copy) : CScriptVarObject(Copy), data(Copy.data) { data->ref(); } ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarFunction();
	virtual CScriptVarPtr clone();
	virtual bool isObject(); // { return true; }
	virtual bool isFunction(); // { return true; }
	virtual bool isPrimitive(); // { return false; } 

	virtual std::string getVarType(); // { return "function"; }
	virtual std::string getParsableString(const std::string &indentString, const std::string &indent, uint32_t uniqueID, bool &hasRecursion);
	virtual CScriptVarPtr _toString(bool &execute, int radix=0);
	virtual CScriptTokenDataFnc *getFunctionData();
	void setFunctionData(CScriptTokenDataFnc *Data);
private:
	CScriptTokenDataFnc *data;

	friend define_newScriptVar_Fnc(Function, CTinyJS *Context, CScriptTokenDataFnc *);
};
inline define_newScriptVar_Fnc(Function, CTinyJS *Context, CScriptTokenDataFnc *Obj) { return new CScriptVarFunction(Context, Obj); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarFunctionNative
//////////////////////////////////////////////////////////////////////////

define_ScriptVarPtr_Type(FunctionNative);
class CScriptVarFunctionNative : public CScriptVarFunction {
protected:
	CScriptVarFunctionNative(CTinyJS *Context, void *Userdata) : CScriptVarFunction(Context, new CScriptTokenDataFnc), jsUserData(Userdata) { }
	CScriptVarFunctionNative(const CScriptVarFunctionNative &Copy) : CScriptVarFunction(Copy), jsUserData(Copy.jsUserData) { } ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarFunctionNative();
	virtual CScriptVarPtr clone()=0;
	virtual bool isNative(); // { return true; }

	virtual std::string getString(); // {return "[ Function Native ]";}

	virtual void callFunction(const CFunctionsScopePtr &c)=0;// { jsCallback(c, jsCallbackUserData); }
protected:
	void *jsUserData; ///< user data passed as second argument to native functions
};


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarFunctionNativeCallback
//////////////////////////////////////////////////////////////////////////

define_ScriptVarPtr_Type(FunctionNativeCallback);
class CScriptVarFunctionNativeCallback : public CScriptVarFunctionNative {
protected:
	CScriptVarFunctionNativeCallback(CTinyJS *Context, JSCallback Callback, void *Userdata) : CScriptVarFunctionNative(Context, Userdata), jsCallback(Callback) { }
	CScriptVarFunctionNativeCallback(const CScriptVarFunctionNativeCallback &Copy) : CScriptVarFunctionNative(Copy), jsCallback(Copy.jsCallback) { } ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarFunctionNativeCallback();
	virtual CScriptVarPtr clone();
	virtual void callFunction(const CFunctionsScopePtr &c);
private:
	JSCallback jsCallback; ///< Callback for native functions
	friend define_newScriptVar_Fnc(FunctionNativeCallback, CTinyJS *Context, JSCallback Callback, void*);
};
inline define_newScriptVar_Fnc(FunctionNativeCallback, CTinyJS *Context, JSCallback Callback, void *Userdata) { return new CScriptVarFunctionNativeCallback(Context, Callback, Userdata); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarFunctionNativeClass
//////////////////////////////////////////////////////////////////////////

template<class native>
class CScriptVarFunctionNativeClass : public CScriptVarFunctionNative {
protected:
	CScriptVarFunctionNativeClass(CTinyJS *Context, native *ClassPtr, void (native::*ClassFnc)(const CFunctionsScopePtr &, void *), void *Userdata) : CScriptVarFunctionNative(Context, Userdata), classPtr(ClassPtr), classFnc(ClassFnc) { }
	CScriptVarFunctionNativeClass(const CScriptVarFunctionNativeClass &Copy) : CScriptVarFunctionNative(Copy), classPtr(Copy.classPtr), classFnc(Copy.classFnc) { } ///< Copy protected -> use clone for public
public:
	virtual CScriptVarPtr clone() { return new CScriptVarFunctionNativeClass(*this); }

	virtual void callFunction(const CFunctionsScopePtr &c) { (classPtr->*classFnc)(c, jsUserData); }
private:
	native *classPtr;
	void (native::*classFnc)(const CFunctionsScopePtr &c, void *userdata);
	template<typename native2>
	friend define_newScriptVar_Fnc(FunctionNativeCallback, CTinyJS*, native2 *, void (native2::*)(const CFunctionsScopePtr &, void *), void *);
};
template<typename native>
define_newScriptVar_Fnc(FunctionNativeCallback, CTinyJS *Context, native *ClassPtr, void (native::*ClassFnc)(const CFunctionsScopePtr &, void *), void *Userdata) { return new CScriptVarFunctionNativeClass<native>(Context, ClassPtr, ClassFnc, Userdata); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarAccessor
//////////////////////////////////////////////////////////////////////////

define_dummy_t(Accessor);
define_ScriptVarPtr_Type(Accessor);

class CScriptVarAccessor : public CScriptVarObject {
protected:
	CScriptVarAccessor(CTinyJS *Context);
	CScriptVarAccessor(CTinyJS *Context, JSCallback getter, void *getterdata, JSCallback setter, void *setterdata);
	template<class C>	CScriptVarAccessor(CTinyJS *Context, C *class_ptr, void(C::*getterFnc)(const CFunctionsScopePtr &, void *), void *getterData, void(C::*setterFnc)(const CFunctionsScopePtr &, void *), void *setterData) : CScriptVarObject(Context) {
		if(getterFnc)
			addChild(TINYJS_ACCESSOR_GET_VAR, ::newScriptVar(Context, class_ptr, getterFnc, getterData), 0);
		if(setterFnc)
			addChild(TINYJS_ACCESSOR_SET_VAR, ::newScriptVar(Context, class_ptr, setterFnc, setterData), 0);
	}

	CScriptVarAccessor(const CScriptVarAccessor &Copy) : CScriptVarObject(Copy) {} ///< Copy protected -> use clone for public
public:
	virtual ~CScriptVarAccessor();
	virtual CScriptVarPtr clone();
	virtual bool isAccessor(); // { return true; }
	virtual bool isPrimitive(); // { return false; } 

	virtual std::string getString(); // { return "[ Object ]"; };
	virtual std::string getParsableString(const std::string &indentString, const std::string &indent, uint32_t uniqueID, bool &hasRecursion);
	virtual std::string getVarType(); // { return "object"; }

	CScriptVarPtr getValue();

	friend define_newScriptVar_Fnc(Accessor, CTinyJS *Context, Accessor_t);
	friend define_newScriptVar_NamedFnc(Accessor, CTinyJS *Context, JSCallback getter, void *getterdata, JSCallback setter, void *setterdata);
	template<class C> friend define_newScriptVar_NamedFnc(Accessor, CTinyJS *Context, C *class_ptr, void(C::*getterFnc)(const CFunctionsScopePtr &, void *), void *getterData, void(C::*setterFnc)(const CFunctionsScopePtr &, void *), void *setterData);
};
inline define_newScriptVar_Fnc(Accessor, CTinyJS *Context, Accessor_t) { return new CScriptVarAccessor(Context); }
inline define_newScriptVar_NamedFnc(Accessor, CTinyJS *Context, JSCallback getter, void *getterdata, JSCallback setter, void *setterdata) { return new CScriptVarAccessor(Context, getter, getterdata, setter, setterdata); }
template<class C> define_newScriptVar_NamedFnc(Accessor, CTinyJS *Context, C *class_ptr, void(C::*getterFnc)(const CFunctionsScopePtr &, void *), void *getterData, void(C::*setterFnc)(const CFunctionsScopePtr &, void *), void *setterData)  { return new CScriptVarAccessor(Context, class_ptr, getterFnc, getterData, setterFnc, setterData); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarScope
//////////////////////////////////////////////////////////////////////////

define_dummy_t(Scope);
define_ScriptVarPtr_Type(Scope);
class CScriptVarScope : public CScriptVarObject {
protected: // only derived classes or friends can be created
	CScriptVarScope(CTinyJS *Context) // constructor for rootScope
		: CScriptVarObject(Context) {}
	virtual CScriptVarPtr clone();
	virtual bool isObject(); // { return false; }
public:
	virtual ~CScriptVarScope();
	virtual CScriptVarPtr scopeVar(); ///< to create var like: var a = ...
	virtual CScriptVarPtr scopeLet(); ///< to create var like: let a = ...
	virtual CScriptVarLinkWorkPtr findInScopes(const std::string &childName);
	virtual CScriptVarScopePtr getParent();
	friend define_newScriptVar_Fnc(Scope, CTinyJS *Context, Scope_t);
};
inline define_newScriptVar_Fnc(Scope, CTinyJS *Context, Scope_t) { return new CScriptVarScope(Context); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarScopeFnc
//////////////////////////////////////////////////////////////////////////

define_dummy_t(ScopeFnc);
define_ScriptVarPtr_Type(ScopeFnc);
class CScriptVarScopeFnc : public CScriptVarScope {
protected: // only derived classes or friends can be created
	CScriptVarScopeFnc(CTinyJS *Context, const CScriptVarScopePtr &Closure) // constructor for FncScope
		: CScriptVarScope(Context), closure(Closure ? addChild(TINYJS_FUNCTION_CLOSURE_VAR, Closure, 0) : CScriptVarLinkPtr()) {}
public:
	virtual ~CScriptVarScopeFnc();
	virtual CScriptVarLinkWorkPtr findInScopes(const std::string &childName);
	
	void setReturnVar(const CScriptVarPtr &var); ///< Set the result value. Use this when setting complex return data as it avoids a deepCopy()
	
	#define DEPRECATED_getParameter DEPRECATED("getParameter is deprecated use getArgument instead")
	DEPRECATED_getParameter CScriptVarPtr getParameter(const std::string &name); 
	DEPRECATED_getParameter CScriptVarPtr getParameter(int Idx); 
	CScriptVarPtr getArgument(const std::string &name); ///< If this is a function, get the parameter with the given name (for use by native functions)
	CScriptVarPtr getArgument(int Idx); ///< If this is a function, get the parameter with the given index (for use by native functions)
	DEPRECATED("getParameterLength is deprecated use getArgumentsLength instead") int getParameterLength(); ///< If this is a function, get the count of parameters
	int getArgumentsLength(); ///< If this is a function, get the count of parameters

	void throwError(ERROR_TYPES ErrorType, const std::string &message);

protected:
	CScriptVarLinkPtr closure;
	friend define_newScriptVar_Fnc(ScopeFnc, CTinyJS *Context, ScopeFnc_t, const CScriptVarScopePtr &Closure);
};
inline define_newScriptVar_Fnc(ScopeFnc, CTinyJS *Context, ScopeFnc_t, const CScriptVarScopePtr &Closure) { return new CScriptVarScopeFnc(Context, Closure); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarScopeLet
//////////////////////////////////////////////////////////////////////////

define_dummy_t(ScopeLet);
define_ScriptVarPtr_Type(ScopeLet);
class CScriptVarScopeLet : public CScriptVarScope {
protected: // only derived classes or friends can be created
	CScriptVarScopeLet(const CScriptVarScopePtr &Parent); // constructor for LetScope
//		: CScriptVarScope(Parent->getContext()), parent( context->getRoot() != Parent ? addChild(TINYJS_SCOPE_PARENT_VAR, Parent, 0) : 0) {}
public:
	virtual ~CScriptVarScopeLet();
	virtual CScriptVarLinkWorkPtr findInScopes(const std::string &childName);
	virtual CScriptVarPtr scopeVar(); ///< to create var like: var a = ...
	virtual CScriptVarScopePtr getParent();
	void setletExpressionInitMode(bool Mode) { letExpressionInitMode = Mode; }
protected:
	CScriptVarLinkPtr parent;
	bool letExpressionInitMode;
	friend define_newScriptVar_Fnc(ScopeLet, CTinyJS *Context, ScopeLet_t, const CScriptVarScopePtr &Parent);
};
inline define_newScriptVar_Fnc(ScopeLet, CTinyJS *, ScopeLet_t, const CScriptVarScopePtr &Parent) { return new CScriptVarScopeLet(Parent); }


////////////////////////////////////////////////////////////////////////// 
/// CScriptVarScopeWith
//////////////////////////////////////////////////////////////////////////

define_dummy_t(ScopeWith);
define_ScriptVarPtr_Type(ScopeWith);
class CScriptVarScopeWith : public CScriptVarScopeLet {
protected:
	CScriptVarScopeWith(const CScriptVarScopePtr &Parent, const CScriptVarPtr &With) 
		: CScriptVarScopeLet(Parent), with(addChild(TINYJS_SCOPE_WITH_VAR, With, 0)) {}

public:
	virtual ~CScriptVarScopeWith();
	virtual CScriptVarPtr scopeLet(); ///< to create var like: let a = ...
	virtual CScriptVarLinkWorkPtr findInScopes(const std::string &childName);
private:
	CScriptVarLinkPtr with;
	friend define_newScriptVar_Fnc(ScopeWith, CTinyJS *Context, ScopeWith_t, const CScriptVarScopePtr &Parent, const CScriptVarPtr &With);
};
inline define_newScriptVar_Fnc(ScopeWith, CTinyJS *, ScopeWith_t, const CScriptVarScopePtr &Parent, const CScriptVarPtr &With) { return new CScriptVarScopeWith(Parent, With); }

//////////////////////////////////////////////////////////////////////////
template<typename T>
inline CScriptVarPtr CScriptVar::newScriptVar(T t) { return ::newScriptVar(context, t); }
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/// CTinyJS
//////////////////////////////////////////////////////////////////////////

class CTinyJS {
public:
	CTinyJS();
	~CTinyJS();

	void execute(CScriptTokenizer &Tokenizer);
	void execute(const char *Code, const std::string &File="", int Line=0, int Column=0);
	void execute(const std::string &Code, const std::string &File="", int Line=0, int Column=0);
	/** Evaluate the given code and return a link to a javascript object,
	 * useful for (dangerous) JSON parsing. If nothing to return, will return
	 * 'undefined' variable type. CScriptVarLink is returned as this will
	 * automatically unref the result as it goes out of scope. If you want to
	 * keep it, you must use ref() and unref() */
	CScriptVarLinkPtr evaluateComplex(CScriptTokenizer &Tokenizer);
	/** Evaluate the given code and return a link to a javascript object,
	 * useful for (dangerous) JSON parsing. If nothing to return, will return
	 * 'undefined' variable type. CScriptVarLink is returned as this will
	 * automatically unref the result as it goes out of scope. If you want to
	 * keep it, you must use ref() and unref() */
	CScriptVarLinkPtr evaluateComplex(const char *code, const std::string &File="", int Line=0, int Column=0);
	/** Evaluate the given code and return a link to a javascript object,
	 * useful for (dangerous) JSON parsing. If nothing to return, will return
	 * 'undefined' variable type. CScriptVarLink is returned as this will
	 * automatically unref the result as it goes out of scope. If you want to
	 * keep it, you must use ref() and unref() */
	CScriptVarLinkPtr evaluateComplex(const std::string &code, const std::string &File="", int Line=0, int Column=0);
	/** Evaluate the given code and return a string. If nothing to return, will return
	 * 'undefined' */
	std::string evaluate(CScriptTokenizer &Tokenizer);
	/** Evaluate the given code and return a string. If nothing to return, will return
	 * 'undefined' */
	std::string evaluate(const char *code, const std::string &File="", int Line=0, int Column=0);
	/** Evaluate the given code and return a string. If nothing to return, will return
	 * 'undefined' */
	std::string evaluate(const std::string &code, const std::string &File="", int Line=0, int Column=0);

	/// add a native function to be called from TinyJS
	/** example:
		\code
			void scRandInt(const CFunctionsScopePtr &c, void *userdata) { ... }
			tinyJS->addNative("function randInt(min, max)", scRandInt, 0);
		\endcode

		or

		\code
			void scSubstring(const CFunctionsScopePtr &c, void *userdata) { ... }
			tinyJS->addNative("function String.substring(lo, hi)", scSubstring, 0);
		\endcode
		or

		\code
			class Class
			{
			public:
				void scSubstring(const CFunctionsScopePtr &c, void *userdata) { ... }
			};
			Class Instanz;
			tinyJS->addNative("function String.substring(lo, hi)", &Instanz, &Class::*scSubstring, 0);
		\endcode
	*/

	CScriptVarFunctionNativePtr addNative(const std::string &funcDesc, JSCallback ptr, void *userdata=0, int LinkFlags=SCRIPTVARLINK_NATIVEDEFAULT);
	template<class C>
	CScriptVarFunctionNativePtr addNative(const std::string &funcDesc, C *class_ptr, void(C::*class_fnc)(const CFunctionsScopePtr &, void *), void *userdata=0, int LinkFlags=SCRIPTVARLINK_NATIVEDEFAULT)
	{
		return addNative(funcDesc, ::newScriptVar<C>(this, class_ptr, class_fnc, userdata), LinkFlags);
	}

	/// Send all variables to stdout
	void trace();

	const CScriptVarScopePtr &getRoot() { return root; };   /// gets the root of symbol table
	//	CScriptVar *root;   /// root of symbol table

	/// newVars & constVars
	template<typename T>	CScriptVarPtr newScriptVar(T t) { return ::newScriptVar(this, t); }
	template<typename T1, typename T2>	CScriptVarPtr newScriptVar(T1 t1, T2 t2) { return ::newScriptVar(this, t1, t2); }
	const CScriptVarPtr &constScriptVar(Undefined_t)	{ return constUndefined; }
	const CScriptVarPtr &constScriptVar(Null_t)			{ return constNull; }
	const CScriptVarPtr &constScriptVar(NaN_t)			{ return constNaN; }
	const CScriptVarPtr &constScriptVar(Infinity t)		{ return t.Sig()<0 ? constInfinityNegative : constInfinityPositive; }
	const CScriptVarPtr &constScriptVar(bool Val)		{ return Val?constTrue:constFalse; }
	const CScriptVarPtr &constScriptVar(Zero_t)			{ return constZero; }
	const CScriptVarPtr &constScriptVar(One_t)			{ return constOne; }


private:
	static bool noexecute;
	CScriptTokenizer *t;       /// current tokenizer
	int runtimeFlags;
	std::string label;
	std::vector<CScriptVarScopePtr>scopes;
	CScriptVarScopePtr root;
	const CScriptVarScopePtr &scope() { return scopes.back(); }

	class CScopeControl { // helper-class to manage scopes
	private:
		CScopeControl(const CScopeControl& Copy); // no copy
		CScopeControl& operator =(const CScopeControl& Copy);
	public:
		CScopeControl(CTinyJS *Context) : context(Context), count(0) {} 
		~CScopeControl() { while(count--) {CScriptVarScopePtr parent = context->scopes.back()->getParent(); if(parent) context->scopes.back() = parent; else context->scopes.pop_back() ;} } 
		void addFncScope(const CScriptVarScopePtr &Scope) { context->scopes.push_back(Scope); count++; }
		CScriptVarScopeLetPtr addLetScope() {	count++; return context->scopes.back() = ::newScriptVar(context, ScopeLet, context->scopes.back()); }
		void addWithScope(const CScriptVarPtr &With) { context->scopes.back() = ::newScriptVar(context, ScopeWith, context->scopes.back(), With); count++; }  
	private:
		CTinyJS *context;
		int		count;
	};
	friend class CScopeControl;
public:
	CScriptVarPtr objectPrototype; /// Built in object class
	CScriptVarPtr objectPrototype_valueOf; /// Built in object class
	CScriptVarPtr objectPrototype_toString; /// Built in object class
	CScriptVarPtr arrayPrototype; /// Built in array class
	CScriptVarPtr stringPrototype; /// Built in string class
	CScriptVarPtr regexpPrototype; /// Built in string class
	CScriptVarPtr numberPrototype; /// Built in number class
	CScriptVarPtr booleanPrototype; /// Built in boolean class
	CScriptVarPtr functionPrototype; /// Built in function class
	const CScriptVarPtr &getErrorPrototype(ERROR_TYPES Type) { return errorPrototypes[Type]; }
private:
	CScriptVarPtr errorPrototypes[ERROR_COUNT]; /// Built in error class
	CScriptVarPtr constUndefined;
	CScriptVarPtr constNull;
	CScriptVarPtr constNaN;
	CScriptVarPtr constInfinityPositive;
	CScriptVarPtr constInfinityNegative;
	CScriptVarPtr constTrue;
	CScriptVarPtr constFalse;
	CScriptVarPtr constOne;
	CScriptVarPtr constZero;
	std::vector<CScriptVarPtr *> pseudo_refered;
	CScriptVarPtr exceptionVar; /// containing the exception var by (runtimeFlags&RUNTIME_THROW) == true; 

	void CheckRightHandVar(bool &execute, CScriptVarLinkWorkPtr &link)
	{
		if(execute && link && !link->isOwned() && !link->isOwner() && !link->getName().empty())
			throwError(execute, ReferenceError, link->getName() + " is not defined", t->getPrevPos());
	}

	void CheckRightHandVar(bool &execute, CScriptVarLinkWorkPtr &link, CScriptTokenizer::ScriptTokenPosition &Pos)
	{
		if(execute && link && !link->isOwned() && !link->isOwner() && !link->getName().empty())
			throwError(execute, ReferenceError, link->getName() + " is not defined", Pos);
	}

public:
	// function call
	CScriptVarPtr callFunction(const CScriptVarFunctionPtr &Function, std::vector<CScriptVarPtr> &Arguments, const CScriptVarPtr &This, CScriptVarPtr *newThis=0);
	CScriptVarPtr callFunction(bool &execute, const CScriptVarFunctionPtr &Function, std::vector<CScriptVarPtr> &Arguments, const CScriptVarPtr &This, CScriptVarPtr *newThis=0);
	const CScriptVarPtr &getExeptionVar() { return exceptionVar; }
	//
	// parsing - in order of precedence

	CScriptVarPtr mathsOp(bool &execute, const CScriptVarPtr &a, const CScriptVarPtr &b, int op);
private:

	void execute_var_init(bool hideLetScope, bool &execute);
	void execute_destructuring(CScriptTokenDataObjectLiteral &Objc, const CScriptVarPtr &Val, bool &execute);
	CScriptVarLinkWorkPtr execute_literals(bool &execute);
	CScriptVarLinkWorkPtr execute_member(CScriptVarLinkWorkPtr &parent, bool &execute);
	CScriptVarLinkWorkPtr execute_function_call(bool &execute);
	CScriptVarLinkWorkPtr execute_unary(bool &execute);
	CScriptVarLinkWorkPtr execute_term(bool &execute);
	CScriptVarLinkWorkPtr execute_expression(bool &execute);
	CScriptVarLinkWorkPtr execute_binary_shift(bool &execute);
	CScriptVarLinkWorkPtr execute_relation(bool &execute, int set=LEX_EQUAL, int set_n='<');
	CScriptVarLinkWorkPtr execute_binary_logic(bool &execute, int op='|', int op_n1='^', int op_n2='&');
	CScriptVarLinkWorkPtr execute_logic(bool &execute, int op=LEX_OROR, int op_n=LEX_ANDAND);
	CScriptVarLinkWorkPtr execute_condition(bool &execute);
	CScriptVarLinkPtr execute_assignment(CScriptVarLinkWorkPtr Lhs, bool &execute);
	CScriptVarLinkPtr execute_assignment(bool &execute);
	CScriptVarLinkPtr execute_base(bool &execute);
	void execute_block(bool &execute, bool createLetScope=true);
	CScriptVarLinkPtr execute_statement(bool &execute);
	// parsing utility functions
	CScriptVarLinkWorkPtr parseFunctionDefinition(const CScriptToken &FncToken);
	CScriptVarLinkWorkPtr parseFunctionsBodyFromString(const std::string &ArgumentList, const std::string &FncBody);
public:
	CScriptVarLinkPtr findInScopes(const std::string &childName); ///< Finds a child, looking recursively up the scopes
private:
	//////////////////////////////////////////////////////////////////////////
	/// addNative-helper
	CScriptVarFunctionNativePtr addNative(const std::string &funcDesc, CScriptVarFunctionNativePtr Var, int LinkFlags);

	//////////////////////////////////////////////////////////////////////////
	/// throws an Error & Exception
public:
	void throwError(bool &execute, ERROR_TYPES ErrorType, const std::string &message);
	void throwException(ERROR_TYPES ErrorType, const std::string &message);
	void throwError(bool &execute, ERROR_TYPES ErrorType, const std::string &message, CScriptTokenizer::ScriptTokenPosition &Pos);
	void throwException(ERROR_TYPES ErrorType, const std::string &message, CScriptTokenizer::ScriptTokenPosition &Pos);
private:
	//////////////////////////////////////////////////////////////////////////
	/// native Object-Constructors & prototype-functions

	void native_Object(const CFunctionsScopePtr &c, void *data);
	void native_Object_getPrototypeOf(const CFunctionsScopePtr &c, void *data);
	void native_Object_preventExtensions(const CFunctionsScopePtr &c, void *data);
	void native_Object_isExtensible(const CFunctionsScopePtr &c, void *data);
	void native_Object_prototype_hasOwnProperty(const CFunctionsScopePtr &c, void *data);
	void native_Object_prototype_valueOf(const CFunctionsScopePtr &c, void *data);
	void native_Object_prototype_toString(const CFunctionsScopePtr &c, void *data);

	void native_Array(const CFunctionsScopePtr &c, void *data);

	void native_String(const CFunctionsScopePtr &c, void *data);
	void native_String__constructor__(const CFunctionsScopePtr &c, void *data);

	void native_RegExp(const CFunctionsScopePtr &c, void *data);

	void native_Number(const CFunctionsScopePtr &c, void *data);
	void native_Number__constructor__(const CFunctionsScopePtr &c, void *data);

	void native_Boolean(const CFunctionsScopePtr &c, void *data);
	void native_Boolean__constructor__(const CFunctionsScopePtr &c, void *data);

	void native_Function(const CFunctionsScopePtr &c, void *data);
	void native_Function_prototype_call(const CFunctionsScopePtr &c, void *data);
	void native_Function_prototype_apply(const CFunctionsScopePtr &c, void *data);

	void native_Error(const CFunctionsScopePtr &c, void *data);
	void native_EvalError(const CFunctionsScopePtr &c, void *data);
	void native_RangeError(const CFunctionsScopePtr &c, void *data);
	void native_ReferenceError(const CFunctionsScopePtr &c, void *data);
	void native_SyntaxError(const CFunctionsScopePtr &c, void *data);
	void native_TypeError(const CFunctionsScopePtr &c, void *data);


	//////////////////////////////////////////////////////////////////////////
	/// global function

	void native_eval(const CFunctionsScopePtr &c, void *data);
	void native_isNAN(const CFunctionsScopePtr &c, void *data);
	void native_isFinite(const CFunctionsScopePtr &c, void *data);
	void native_parseInt(const CFunctionsScopePtr &c, void *data);
	void native_parseFloat(const CFunctionsScopePtr &c, void *data);



	void native_JSON_parse(const CFunctionsScopePtr &c, void *data);


	uint32_t uniqueID;
public:
	uint32_t getUniqueID() { return ++uniqueID; }
	CScriptVar *first;
	void setTemporaryID_recursive(uint32_t ID);
	void ClearUnreferedVars(const CScriptVarPtr &extra=CScriptVarPtr());
};

//////////////////////////////////////////////////////////////////////////
template<typename T>
inline const CScriptVarPtr &CScriptVar::constScriptVar(T t) { return context->constScriptVar(t); }
//////////////////////////////////////////////////////////////////////////

#endif


