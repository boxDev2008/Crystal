#include "TextEditor.h"

static bool TokenizeCStyleString(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
	const char* p = in_begin;

	if (*p == '"')
	{
		p++;

		while (p < in_end)
		{
			// handle end of string
			if (*p == '"')
			{
				out_begin = in_begin;
				out_end = p + 1;
				return true;
			}

			// handle escape character for "
			if (*p == '\\' && p + 1 < in_end && p[1] == '"')
				p++;

			p++;
		}
	}

	return false;
}

static bool TokenizeCStyleCharacterLiteral(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
	const char* p = in_begin;

	if (*p == '\'')
	{
		p++;

		// handle escape characters
		if (p < in_end && *p == '\\')
			p++;

		if (p < in_end)
			p++;

		// handle end of character literal
		if (p < in_end && *p == '\'')
		{
			out_begin = in_begin;
			out_end = p + 1;
			return true;
		}
	}

	return false;
}

static bool TokenizeCStyleIdentifier(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
	const char* p = in_begin;

	if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_')
	{
		p++;

		while ((p < in_end) && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_'))
			p++;

		out_begin = in_begin;
		out_end = p;
		return true;
	}

	return false;
}

static bool TokenizeCStyleNumber(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
	const char* p = in_begin;

	const bool startsWithNumber = *p >= '0' && *p <= '9';

	if (*p != '+' && *p != '-' && !startsWithNumber)
		return false;

	p++;

	bool hasNumber = startsWithNumber;

	while (p < in_end && (*p >= '0' && *p <= '9'))
	{
		hasNumber = true;

		p++;
	}

	if (hasNumber == false)
		return false;

	bool isFloat = false;
	bool isHex = false;
	bool isBinary = false;

	if (p < in_end)
	{
		if (*p == '.')
		{
			isFloat = true;

			p++;

			while (p < in_end && (*p >= '0' && *p <= '9'))
				p++;
		}
		else if (*p == 'x' || *p == 'X')
		{
			// hex formatted integer of the type 0xef80

			isHex = true;

			p++;

			while (p < in_end && ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F')))
				p++;
		}
		else if (*p == 'b' || *p == 'B')
		{
			// binary formatted integer of the type 0b01011101

			isBinary = true;

			p++;

			while (p < in_end && (*p >= '0' && *p <= '1'))
				p++;
		}
	}

	if (isHex == false && isBinary == false)
	{
		// floating point exponent
		if (p < in_end && (*p == 'e' || *p == 'E'))
		{
			isFloat = true;

			p++;

			if (p < in_end && (*p == '+' || *p == '-'))
				p++;

			bool hasDigits = false;

			while (p < in_end && (*p >= '0' && *p <= '9'))
			{
				hasDigits = true;

				p++;
			}

			if (hasDigits == false)
				return false;
		}

		// single precision floating point type
		if (p < in_end && *p == 'f')
			p++;
	}

	if (isFloat == false)
	{
		// integer size type
		while (p < in_end && (*p == 'u' || *p == 'U' || *p == 'l' || *p == 'L'))
			p++;
	}

	out_begin = in_begin;
	out_end = p;
	return true;
}

static bool TokenizeCStylePunctuation(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
	(void)in_end;

	switch (*in_begin)
	{
	case '[':
	case ']':
	case '{':
	case '}':
	case '!':
	case '%':
	case '^':
	case '&':
	case '*':
	case '(':
	case ')':
	case '-':
	case '+':
	case '=':
	case '~':
	case '|':
	case '<':
	case '>':
	case '?':
	case ':':
	case '/':
	case ';':
	case ',':
	case '.':
		out_begin = in_begin;
		out_end = in_begin + 1;
		return true;
	}

	return false;
}

static bool TokenizeCStyleFunctionName(TextEditor::LanguageDefinition &definition, const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
    const char* p = in_begin;
    std::string identifier;

    if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_')
    {
        while (p < in_end && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_'))
        {
            identifier += *p;
            p++;
        }

        if (definition.mKeywords.find(identifier) != definition.mKeywords.end())
            return false;

        while (p < in_end && (*p == ' ' || *p == '\t'))
            p++;

        if (p < in_end && *p == '(')
        {
            out_begin = in_begin;
            out_end = p;
            return true;
        }
    }

    return false;
}

static bool TokenizeLuaStyleString(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
	const char* p = in_begin;

	bool is_single_quote = false;
	bool is_double_quotes = false;
	bool is_double_square_brackets = false;

	switch (*p)
	{
	case '\'':
		is_single_quote = true;
		break;
	case '"':
		is_double_quotes = true;
		break;
	case '[':
		p++;
		if (p < in_end && *(p) == '[')
			is_double_square_brackets = true;
		break;
	}

	if (is_single_quote || is_double_quotes || is_double_square_brackets)
	{
		p++;

		while (p < in_end)
		{
			// handle end of string
			if ((is_single_quote && *p == '\'') || (is_double_quotes && *p == '"') || (is_double_square_brackets && *p == ']' && p + 1 < in_end && *(p + 1) == ']'))
			{
				out_begin = in_begin;

				if (is_double_square_brackets)
					out_end = p + 2;
				else
					out_end = p + 1;

				return true;
			}

			// handle escape character for "
			if (*p == '\\' && p + 1 < in_end && (is_single_quote || is_double_quotes))
				p++;

			p++;
		}
	}

	return false;
}

static bool TokenizeLuaStyleIdentifier(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
	const char* p = in_begin;

	if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_')
	{
		p++;

		while ((p < in_end) && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_'))
			p++;

		out_begin = in_begin;
		out_end = p;
		return true;
	}

	return false;
}

static bool TokenizeLuaStyleNumber(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
	const char* p = in_begin;

	const bool startsWithNumber = *p >= '0' && *p <= '9';

	if (*p != '+' && *p != '-' && !startsWithNumber)
		return false;

	p++;

	bool hasNumber = startsWithNumber;

	while (p < in_end && (*p >= '0' && *p <= '9'))
	{
		hasNumber = true;

		p++;
	}

	if (hasNumber == false)
		return false;

	if (p < in_end)
	{
		if (*p == '.')
		{
			p++;

			while (p < in_end && (*p >= '0' && *p <= '9'))
				p++;
		}

		// floating point exponent
		if (p < in_end && (*p == 'e' || *p == 'E'))
		{
			p++;

			if (p < in_end && (*p == '+' || *p == '-'))
				p++;

			bool hasDigits = false;

			while (p < in_end && (*p >= '0' && *p <= '9'))
			{
				hasDigits = true;

				p++;
			}

			if (hasDigits == false)
				return false;
		}
	}

	out_begin = in_begin;
	out_end = p;
	return true;
}

static bool TokenizeLuaStylePunctuation(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
	(void)in_end;

	switch (*in_begin)
	{
	case '[':
	case ']':
	case '{':
	case '}':
	case '!':
	case '%':
	case '#':
	case '^':
	case '&':
	case '*':
	case '(':
	case ')':
	case '-':
	case '+':
	case '=':
	case '~':
	case '|':
	case '<':
	case '>':
	case '?':
	case ':':
	case '/':
	case ';':
	case ',':
	case '.':
		out_begin = in_begin;
		out_end = in_begin + 1;
		return true;
	}

	return false;
}

static bool TokenizeIniVariableName(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end)
{
    const char* p = in_begin;

    while (p < in_end && (*p == ' ' || *p == '\t'))
        p++;

    out_begin = p;

    while (p < in_end && *p != '=')
        p++;

    if (p < in_end && *p == '=')
    {
        const char* end = p;
        while (end > out_begin && (*(end - 1) == ' ' || *(end - 1) == '\t'))
            end--;

        out_end = end;

        if (out_begin < out_end)
            return true;
    }

    return false;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Cpp()
{
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited)
	{
		static const char* const cppKeywords[] = {
			"alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char16_t", "char32_t", "class",
			"compl", "concept", "const", "constexpr", "const_cast", "continue", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float",
			"for", "friend", "goto", "if", "import", "inline", "int", "long", "module", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
			"register", "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "synchronized", "template", "this", "thread_local",
			"throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
		};
		for (auto& k : cppKeywords)
			langDef.mKeywords.insert(k);

		/*static const char* const identifiers[] = {
			"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
			"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "printf", "sprintf", "snprintf", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper",
			"std", "string", "vector", "map", "unordered_map", "set", "unordered_set", "min", "max"
		};
		for (auto& k : identifiers)
		{
			Identifier id;
			id.mDeclaration = "Built-in function";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}*/

		langDef.mTokenize = [](const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex) -> bool
		{
			paletteIndex = PaletteIndex::Max;

			while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
				in_begin++;

			if (in_begin == in_end)
			{
				out_begin = in_end;
				out_end = in_end;
				paletteIndex = PaletteIndex::Default;
			}
			else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::String;
			else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::CharLiteral;
			else if (TokenizeCStyleFunctionName(langDef, in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::KnownIdentifier;
			else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Identifier;
			else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Number;
			else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Punctuation;

			return paletteIndex != PaletteIndex::Max;
		};

		langDef.mCommentStart = "/*";
		langDef.mCommentEnd = "*/";
		langDef.mSingleLineComment = "//";

		langDef.mCaseSensitive = true;

		langDef.mName = "C++";

		inited = true;
	}
	return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Hlsl()
{
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited)
	{
		static const char* const keywords[] = {
			"AppendStructuredBuffer", "asm", "asm_fragment", "BlendState", "bool", "break", "Buffer", "ByteAddressBuffer", "case", "cbuffer", "centroid", "class", "column_major", "compile", "compile_fragment",
			"CompileShader", "const", "continue", "ComputeShader", "ConsumeStructuredBuffer", "default", "DepthStencilState", "DepthStencilView", "discard", "do", "double", "DomainShader", "dword", "else",
			"export", "extern", "false", "float", "for", "fxgroup", "GeometryShader", "groupshared", "half", "Hullshader", "if", "in", "inline", "inout", "InputPatch", "int", "interface", "line", "lineadj",
			"linear", "LineStream", "matrix", "min16float", "min10float", "min16int", "min12int", "min16uint", "namespace", "nointerpolation", "noperspective", "NULL", "out", "OutputPatch", "packoffset",
			"pass", "pixelfragment", "PixelShader", "point", "PointStream", "precise", "RasterizerState", "RenderTargetView", "return", "register", "row_major", "RWBuffer", "RWByteAddressBuffer", "RWStructuredBuffer",
			"RWTexture1D", "RWTexture1DArray", "RWTexture2D", "RWTexture2DArray", "RWTexture3D", "sample", "sampler", "SamplerState", "SamplerComparisonState", "shared", "snorm", "stateblock", "stateblock_state",
			"static", "string", "struct", "switch", "StructuredBuffer", "tbuffer", "technique", "technique10", "technique11", "texture", "Texture1D", "Texture1DArray", "Texture2D", "Texture2DArray", "Texture2DMS",
			"Texture2DMSArray", "Texture3D", "TextureCube", "TextureCubeArray", "true", "typedef", "triangle", "triangleadj", "TriangleStream", "uint", "uniform", "unorm", "unsigned", "vector", "vertexfragment",
			"VertexShader", "void", "volatile", "while",
			"bool1","bool2","bool3","bool4","double1","double2","double3","double4", "float1", "float2", "float3", "float4", "int1", "int2", "int3", "int4", "in", "out", "inout",
			"uint1", "uint2", "uint3", "uint4", "dword1", "dword2", "dword3", "dword4", "half1", "half2", "half3", "half4",
			"float1x1","float2x1","float3x1","float4x1","float1x2","float2x2","float3x2","float4x2",
			"float1x3","float2x3","float3x3","float4x3","float1x4","float2x4","float3x4","float4x4",
			"half1x1","half2x1","half3x1","half4x1","half1x2","half2x2","half3x2","half4x2",
			"half1x3","half2x3","half3x3","half4x3","half1x4","half2x4","half3x4","half4x4",
		};
		for (auto& k : keywords)
			langDef.mKeywords.insert(k);

		static const char* const identifiers[] = {
			"abort", "abs", "acos", "all", "AllMemoryBarrier", "AllMemoryBarrierWithGroupSync", "any", "asdouble", "asfloat", "asin", "asint", "asint", "asuint",
			"asuint", "atan", "atan2", "ceil", "CheckAccessFullyMapped", "clamp", "clip", "cos", "cosh", "countbits", "cross", "D3DCOLORtoUBYTE4", "ddx",
			"ddx_coarse", "ddx_fine", "ddy", "ddy_coarse", "ddy_fine", "degrees", "determinant", "DeviceMemoryBarrier", "DeviceMemoryBarrierWithGroupSync",
			"distance", "dot", "dst", "errorf", "EvaluateAttributeAtCentroid", "EvaluateAttributeAtSample", "EvaluateAttributeSnapped", "exp", "exp2",
			"f16tof32", "f32tof16", "faceforward", "firstbithigh", "firstbitlow", "floor", "fma", "fmod", "frac", "frexp", "fwidth", "GetRenderTargetSampleCount",
			"GetRenderTargetSamplePosition", "GroupMemoryBarrier", "GroupMemoryBarrierWithGroupSync", "InterlockedAdd", "InterlockedAnd", "InterlockedCompareExchange",
			"InterlockedCompareStore", "InterlockedExchange", "InterlockedMax", "InterlockedMin", "InterlockedOr", "InterlockedXor", "isfinite", "isinf", "isnan",
			"ldexp", "length", "lerp", "lit", "log", "log10", "log2", "mad", "max", "min", "modf", "msad4", "mul", "noise", "normalize", "pow", "printf",
			"Process2DQuadTessFactorsAvg", "Process2DQuadTessFactorsMax", "Process2DQuadTessFactorsMin", "ProcessIsolineTessFactors", "ProcessQuadTessFactorsAvg",
			"ProcessQuadTessFactorsMax", "ProcessQuadTessFactorsMin", "ProcessTriTessFactorsAvg", "ProcessTriTessFactorsMax", "ProcessTriTessFactorsMin",
			"radians", "rcp", "reflect", "refract", "reversebits", "round", "rsqrt", "saturate", "sign", "sin", "sincos", "sinh", "smoothstep", "sqrt", "step",
			"tan", "tanh", "tex1D", "tex1D", "tex1Dbias", "tex1Dgrad", "tex1Dlod", "tex1Dproj", "tex2D", "tex2D", "tex2Dbias", "tex2Dgrad", "tex2Dlod", "tex2Dproj",
			"tex3D", "tex3D", "tex3Dbias", "tex3Dgrad", "tex3Dlod", "tex3Dproj", "texCUBE", "texCUBE", "texCUBEbias", "texCUBEgrad", "texCUBElod", "texCUBEproj", "transpose", "trunc"
		};
		for (auto& k : identifiers)
		{
			Identifier id;
			id.mDeclaration = "Built-in function";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}

		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([ \t]*#[ \t]*[a-zA-Z_]+)##", PaletteIndex::Preprocessor));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(L?\"(\\.|[^\"])*\")##", PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(\'\\?[^\']\')##", PaletteIndex::CharLiteral));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?[0-9]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[0-7]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([a-zA-Z_][a-zA-Z0-9_]*)##", PaletteIndex::Identifier));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([\[\]\{\}\!\%\^\&\*\(\)\-\+\=\~\|\<\>\?\/\;\,\.])##", PaletteIndex::Punctuation));

		langDef.mCommentStart = "/*";
		langDef.mCommentEnd = "*/";
		langDef.mSingleLineComment = "//";

		langDef.mCaseSensitive = true;

		langDef.mName = "HLSL";

		inited = true;
	}
	return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Glsl()
{
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited)
	{
		static const char* const keywords[] = {
			"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short",
			"signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
			"_Noreturn", "_Static_assert", "_Thread_local"
		};
		for (auto& k : keywords)
			langDef.mKeywords.insert(k);

		static const char* const identifiers[] = {
			"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
			"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper"
		};
		for (auto& k : identifiers)
		{
			Identifier id;
			id.mDeclaration = "Built-in function";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}

		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([ \t]*#[ \t]*[a-zA-Z_]+)##", PaletteIndex::Preprocessor));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(L?\"(\\.|[^\"])*\")##", PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(\'\\?[^\']\')##", PaletteIndex::CharLiteral));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?[0-9]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[0-7]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([a-zA-Z_][a-zA-Z0-9_]*)##", PaletteIndex::Identifier));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([\[\]\{\}\!\%\^\&\*\(\)\-\+\=\~\|\<\>\?\/\;\,\.])##", PaletteIndex::Punctuation));

		langDef.mCommentStart = "/*";
		langDef.mCommentEnd = "*/";
		langDef.mSingleLineComment = "//";

		langDef.mCaseSensitive = true;

		langDef.mName = "GLSL";

		inited = true;
	}
	return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Python()
{
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited)
	{
		static const char* const keywords[] = {
			"False", "await", "else", "import", "pass", "None", "break", "except", "in", "raise", "True", "class", "finally", "is", "return", "and", "continue", "for", "lambda", "try", "as", "def", "from", "nonlocal", "while", "assert", "del", "global", "not", "with", "async", "elif", "if", "or", "yield"
		};
		for (auto& k : keywords)
			langDef.mKeywords.insert(k);

		static const char* const identifiers[] = {
			"abs", "aiter", "all", "any", "anext", "ascii", "bin", "bool", "breakpoint", "bytearray", "bytes", "callable", "chr", "classmethod", "compile", "complex", "delattr", "dict", "dir", "divmod", "enumerate", "eval", "exec", "filter", "float", "format", "frozenset", "getattr", "globals", "hasattr", "hash", "help", "hex", "id", "input", "int", "isinstance", "issubclass", "iter", "len", "list", "locals", "map", "max", "memoryview", "min", "next", "object", "oct", "open", "ord", "pow", "print", "property", "range", "repr", "reversed", "round", "set", "setattr", "slice", "sorted", "staticmethod", "str", "sum", "super", "tuple", "type", "vars", "zip", "__import__"
		};
		for (auto& k : identifiers)
		{
			Identifier id;
			id.mDeclaration = "Built-in function";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}

		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##((b|u|f|r)?\"(\\.|[^\"])*\")##", PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##((b|u|f|r)?'(\\.|[^'])*')##", PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?[0-9]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[0-7]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([a-zA-Z_][a-zA-Z0-9_]*)##", PaletteIndex::Identifier));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([\[\]\{\}\!\%\^\&\*\(\)\-\+\=\~\|\<\>\?\/\;\,\.\:])##", PaletteIndex::Punctuation));

		langDef.mCommentStart = "\"\"\"";
		langDef.mCommentEnd = "\"\"\"";
		langDef.mSingleLineComment = "#";

		langDef.mCaseSensitive = true;

		langDef.mName = "Python";

		inited = true;
	}
	return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::C()
{
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited)
	{
		static const char* const keywords[] = {
			"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short",
			"signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
			"_Noreturn", "_Static_assert", "_Thread_local"
		};
		for (auto& k : keywords)
			langDef.mKeywords.insert(k);

		/*static const char* const identifiers[] = {
			"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
			"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper"
		};
		for (auto& k : identifiers)
		{
			Identifier id;
			id.mDeclaration = "Built-in function";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}*/

		langDef.mTokenize = [](const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex) -> bool
		{
			paletteIndex = PaletteIndex::Max;

			while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
				in_begin++;

			if (in_begin == in_end)
			{
				out_begin = in_end;
				out_end = in_end;
				paletteIndex = PaletteIndex::Default;
			}
			else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::String;
			else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::CharLiteral;
			else if (TokenizeCStyleFunctionName(langDef, in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::KnownIdentifier;
			else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Identifier;
			else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Number;
			else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Punctuation;

			return paletteIndex != PaletteIndex::Max;
		};

		langDef.mCommentStart = "/*";
		langDef.mCommentEnd = "*/";
		langDef.mSingleLineComment = "//";

		langDef.mCaseSensitive = true;

		langDef.mName = "C";

		inited = true;
	}
	return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Sql()
{
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited)
	{
		static const char* const keywords[] = {
			"ADD", "EXCEPT", "PERCENT", "ALL", "EXEC", "PLAN", "ALTER", "EXECUTE", "PRECISION", "AND", "EXISTS", "PRIMARY", "ANY", "EXIT", "PRINT", "AS", "FETCH", "PROC", "ASC", "FILE", "PROCEDURE",
			"AUTHORIZATION", "FILLFACTOR", "PUBLIC", "BACKUP", "FOR", "RAISERROR", "BEGIN", "FOREIGN", "READ", "BETWEEN", "FREETEXT", "READTEXT", "BREAK", "FREETEXTTABLE", "RECONFIGURE",
			"BROWSE", "FROM", "REFERENCES", "BULK", "FULL", "REPLICATION", "BY", "FUNCTION", "RESTORE", "CASCADE", "GOTO", "RESTRICT", "CASE", "GRANT", "RETURN", "CHECK", "GROUP", "REVOKE",
			"CHECKPOINT", "HAVING", "RIGHT", "CLOSE", "HOLDLOCK", "ROLLBACK", "CLUSTERED", "IDENTITY", "ROWCOUNT", "COALESCE", "IDENTITY_INSERT", "ROWGUIDCOL", "COLLATE", "IDENTITYCOL", "RULE",
			"COLUMN", "IF", "SAVE", "COMMIT", "IN", "SCHEMA", "COMPUTE", "INDEX", "SELECT", "CONSTRAINT", "INNER", "SESSION_USER", "CONTAINS", "INSERT", "SET", "CONTAINSTABLE", "INTERSECT", "SETUSER",
			"CONTINUE", "INTO", "SHUTDOWN", "CONVERT", "IS", "SOME", "CREATE", "JOIN", "STATISTICS", "CROSS", "KEY", "SYSTEM_USER", "CURRENT", "KILL", "TABLE", "CURRENT_DATE", "LEFT", "TEXTSIZE",
			"CURRENT_TIME", "LIKE", "THEN", "CURRENT_TIMESTAMP", "LINENO", "TO", "CURRENT_USER", "LOAD", "TOP", "CURSOR", "NATIONAL", "TRAN", "DATABASE", "NOCHECK", "TRANSACTION",
			"DBCC", "NONCLUSTERED", "TRIGGER", "DEALLOCATE", "NOT", "TRUNCATE", "DECLARE", "NULL", "TSEQUAL", "DEFAULT", "NULLIF", "UNION", "DELETE", "OF", "UNIQUE", "DENY", "OFF", "UPDATE",
			"DESC", "OFFSETS", "UPDATETEXT", "DISK", "ON", "USE", "DISTINCT", "OPEN", "USER", "DISTRIBUTED", "OPENDATASOURCE", "VALUES", "DOUBLE", "OPENQUERY", "VARYING","DROP", "OPENROWSET", "VIEW",
			"DUMMY", "OPENXML", "WAITFOR", "DUMP", "OPTION", "WHEN", "ELSE", "OR", "WHERE", "END", "ORDER", "WHILE", "ERRLVL", "OUTER", "WITH", "ESCAPE", "OVER", "WRITETEXT"
		};

		for (auto& k : keywords)
			langDef.mKeywords.insert(k);

		static const char* const identifiers[] = {
			"ABS", "ACOS", "ADD_MONTHS", "ASCII", "ASCIISTR", "ASIN", "ATAN", "ATAN2", "AVG", "BFILENAME", "BIN_TO_NUM", "BITAND", "CARDINALITY", "CASE", "CAST", "CEIL",
			"CHARTOROWID", "CHR", "COALESCE", "COMPOSE", "CONCAT", "CONVERT", "CORR", "COS", "COSH", "COUNT", "COVAR_POP", "COVAR_SAMP", "CUME_DIST", "CURRENT_DATE",
			"CURRENT_TIMESTAMP", "DBTIMEZONE", "DECODE", "DECOMPOSE", "DENSE_RANK", "DUMP", "EMPTY_BLOB", "EMPTY_CLOB", "EXP", "EXTRACT", "FIRST_VALUE", "FLOOR", "FROM_TZ", "GREATEST",
			"GROUP_ID", "HEXTORAW", "INITCAP", "INSTR", "INSTR2", "INSTR4", "INSTRB", "INSTRC", "LAG", "LAST_DAY", "LAST_VALUE", "LEAD", "LEAST", "LENGTH", "LENGTH2", "LENGTH4",
			"LENGTHB", "LENGTHC", "LISTAGG", "LN", "LNNVL", "LOCALTIMESTAMP", "LOG", "LOWER", "LPAD", "LTRIM", "MAX", "MEDIAN", "MIN", "MOD", "MONTHS_BETWEEN", "NANVL", "NCHR",
			"NEW_TIME", "NEXT_DAY", "NTH_VALUE", "NULLIF", "NUMTODSINTERVAL", "NUMTOYMINTERVAL", "NVL", "NVL2", "POWER", "RANK", "RAWTOHEX", "REGEXP_COUNT", "REGEXP_INSTR",
			"REGEXP_REPLACE", "REGEXP_SUBSTR", "REMAINDER", "REPLACE", "ROUND", "ROWNUM", "RPAD", "RTRIM", "SESSIONTIMEZONE", "SIGN", "SIN", "SINH",
			"SOUNDEX", "SQRT", "STDDEV", "SUBSTR", "SUM", "SYS_CONTEXT", "SYSDATE", "SYSTIMESTAMP", "TAN", "TANH", "TO_CHAR", "TO_CLOB", "TO_DATE", "TO_DSINTERVAL", "TO_LOB",
			"TO_MULTI_BYTE", "TO_NCLOB", "TO_NUMBER", "TO_SINGLE_BYTE", "TO_TIMESTAMP", "TO_TIMESTAMP_TZ", "TO_YMINTERVAL", "TRANSLATE", "TRIM", "TRUNC", "TZ_OFFSET", "UID", "UPPER",
			"USER", "USERENV", "VAR_POP", "VAR_SAMP", "VARIANCE", "VSIZE"
		};
		for (auto& k : identifiers)
		{
			Identifier id;
			//id.mDeclaration = "Built-in function";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}

		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(L?\"(\\.|[^\"])*\")##", PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(\'[^\']*\')##", PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?[0-9]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[0-7]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([a-zA-Z_][a-zA-Z0-9_]*)##", PaletteIndex::Identifier));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([\[\]\{\}\!\%\^\&\*\(\)\-\+\=\~\|\<\>\?\/\;\,\.])##", PaletteIndex::Punctuation));

		langDef.mCommentStart = "/*";
		langDef.mCommentEnd = "*/";
		langDef.mSingleLineComment = "--";

		langDef.mCaseSensitive = false;

		langDef.mName = "SQL";

		inited = true;
	}
	return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::AngelScript()
{
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited)
	{
		static const char* const keywords[] = {
			"and", "abstract", "auto", "bool", "break", "case", "cast", "class", "const", "continue", "default", "do", "double", "else", "enum", "false", "final", "float", "for",
			"from", "funcdef", "function", "get", "if", "import", "in", "inout", "int", "interface", "int8", "int16", "int32", "int64", "is", "mixin", "namespace", "not",
			"null", "or", "out", "override", "private", "protected", "return", "set", "shared", "super", "switch", "this ", "true", "typedef", "uint", "uint8", "uint16", "uint32",
			"uint64", "void", "while", "xor"
		};

		for (auto& k : keywords)
			langDef.mKeywords.insert(k);

		static const char* const identifiers[] = {
			"cos", "sin", "tab", "acos", "asin", "atan", "atan2", "cosh", "sinh", "tanh", "log", "log10", "pow", "sqrt", "abs", "ceil", "floor", "fraction", "closeTo", "fpFromIEEE", "fpToIEEE",
			"complex", "opEquals", "opAddAssign", "opSubAssign", "opMulAssign", "opDivAssign", "opAdd", "opSub", "opMul", "opDiv"
		};
		for (auto& k : identifiers)
		{
			Identifier id;
			//id.mDeclaration = "Built-in function";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}

		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(L?\"(\\.|[^\"])*\")##", PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(\'\\?[^\']\')##", PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?[0-9]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[0-7]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([a-zA-Z_][a-zA-Z0-9_]*)##", PaletteIndex::Identifier));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([\[\]\{\}\!\%\^\&\*\(\)\-\+\=\~\|\<\>\?\/\;\,\.])##", PaletteIndex::Punctuation));

		langDef.mCommentStart = "/*";
		langDef.mCommentEnd = "*/";
		langDef.mSingleLineComment = "//";

		langDef.mCaseSensitive = true;

		langDef.mName = "AngelScript";

		inited = true;
	}
	return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Lua()
{
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited)
	{
		static const char* const keywords[] = {
			"and", "break", "do", "else", "elseif", "end", "false", "for", "function", "goto", "if", "in", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until", "while"
		};

		for (auto& k : keywords)
			langDef.mKeywords.insert(k);

		static const char* const identifiers[] = {
			"assert", "collectgarbage", "dofile", "error", "getmetatable", "ipairs", "loadfile", "load", "loadstring", "next", "pairs", "pcall", "print", "rawequal", "rawlen", "rawget", "rawset",
			"select", "setmetatable", "tonumber", "tostring", "type", "xpcall", "_G", "_VERSION","arshift", "band", "bnot", "bor", "bxor", "btest", "extract", "lrotate", "lshift", "replace",
			"rrotate", "rshift", "create", "resume", "running", "status", "wrap", "yield", "isyieldable", "debug","getuservalue", "gethook", "getinfo", "getlocal", "getregistry", "getmetatable",
			"getupvalue", "upvaluejoin", "upvalueid", "setuservalue", "sethook", "setlocal", "setmetatable", "setupvalue", "traceback", "close", "flush", "input", "lines", "open", "output", "popen",
			"read", "tmpfile", "type", "write", "close", "flush", "lines", "read", "seek", "setvbuf", "write", "__gc", "__tostring", "abs", "acos", "asin", "atan", "ceil", "cos", "deg", "exp", "tointeger",
			"floor", "fmod", "ult", "log", "max", "min", "modf", "rad", "random", "randomseed", "sin", "sqrt", "string", "tan", "type", "atan2", "cosh", "sinh", "tanh",
			"pow", "frexp", "ldexp", "log10", "pi", "huge", "maxinteger", "mininteger", "loadlib", "searchpath", "seeall", "preload", "cpath", "path", "searchers", "loaded", "module", "require", "clock",
			"date", "difftime", "execute", "exit", "getenv", "remove", "rename", "setlocale", "time", "tmpname", "byte", "char", "dump", "find", "format", "gmatch", "gsub", "len", "lower", "match", "rep",
			"reverse", "sub", "upper", "pack", "packsize", "unpack", "concat", "maxn", "insert", "pack", "unpack", "remove", "move", "sort", "offset", "codepoint", "char", "len", "codes", "charpattern",
			"coroutine", "table", "io", "os", "string", "utf8", "bit32", "math", "debug", "package"
		};
		for (auto& k : identifiers)
		{
			Identifier id;
			//id.mDeclaration = "Built-in function";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}

		langDef.mTokenize = [](const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex) -> bool
		{
			paletteIndex = PaletteIndex::Max;

			while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
				in_begin++;

			if (in_begin == in_end)
			{
				out_begin = in_end;
				out_end = in_end;
				paletteIndex = PaletteIndex::Default;
			}
			else if (TokenizeLuaStyleString(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::String;
			else if (TokenizeLuaStyleIdentifier(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Identifier;
			else if (TokenizeLuaStyleNumber(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Number;
			else if (TokenizeLuaStylePunctuation(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Punctuation;

			return paletteIndex != PaletteIndex::Max;
		};

		langDef.mCommentStart = "--[[";
		langDef.mCommentEnd = "]]";
		langDef.mSingleLineComment = "--";

		langDef.mCaseSensitive = true;

		langDef.mName = "Lua";

		inited = true;
	}
	return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Cs()
{
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited)
	{
		static const char* const keywords[] = {
			"abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char", "checked", "class", "const", "continue", "decimal", "default", "delegate", "do", "double", "else", "enum", "event", "explicit", "extern", "false", "finally", "fixed", "float", "for", "foreach", "goto", "if", "implicit", "in", "in (generic modifier)", "int", "interface", "internal", "is", "lock", "long", "namespace", "new", "null", "object", "operator", "out", "out (generic modifier)", "override", "params", "private", "protected", "public", "readonly", "ref", "return", "sbyte", "sealed", "short", "sizeof", "stackalloc", "static", "string", "struct", "switch", "this", "throw", "true", "try", "typeof", "uint", "ulong", "unchecked", "unsafe", "ushort", "using", "using static", "void", "volatile", "while"
		};
		for (auto& k : keywords)
			langDef.mKeywords.insert(k);

		/*static const char* const identifiers[] = {
			"add", "alias", "ascending", "async", "await", "descending", "dynamic", "from", "get", "global", "group", "into", "join", "let", "orderby", "partial", "remove", "select", "set", "value", "var", "when", "where", "yield"
		};
		for (auto& k : identifiers)
		{
			Identifier id;
			id.mDeclaration = "Built-in function";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}*/
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(($|@)?\"(\\.|[^\"])*\")##", PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?[0-9]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[0-7]+[Uu]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([a-zA-Z_][a-zA-Z0-9_]*)##", PaletteIndex::Identifier));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([\[\]\{\}\!\%\^\&\*\(\)\-\+\=\~\|\<\>\?\/\;\,\.])##", PaletteIndex::Punctuation));

		langDef.mCommentStart = "/*";
		langDef.mCommentEnd = "*/";
		langDef.mSingleLineComment = "//";

		langDef.mCaseSensitive = true;

		langDef.mName = "C#";

		inited = true;
	}
	return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Json()
{
	static bool inited = false;
	static LanguageDefinition langDef;
	if (!inited)
	{
		langDef.mKeywords.clear();
		langDef.mIdentifiers.clear();

		
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(\"(\\.|[^\"])*\")##", PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?)##", PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([\[\]\{\}\!\%\^\&\*\(\)\-\+\=\~\|\<\>\?\/\;\,\.\:])##", PaletteIndex::Punctuation));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(false|true)##", PaletteIndex::Keyword));

		langDef.mCommentStart = "/*";
		langDef.mCommentEnd = "*/";
		langDef.mSingleLineComment = "//";

		langDef.mCaseSensitive = true;

		langDef.mName = "Json";

		inited = true;
	}
	return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Rust()
{
    static bool inited = false;
    static LanguageDefinition langDef;
    if (!inited)
    {
        static const char* const rustKeywords[] = {
            "as", "async", "await", "break", "const", "continue", "crate", "dyn", "else", "enum", "extern", "fn", "for", "if", "impl", "in", "let", "loop", "match", "mod", "move", "mut", "pub", "ref", "return", "self", "static", "struct", "trait", "type", "unsafe", "use", "where", "while", "yield"
        };
        for (auto& k : rustKeywords)
            langDef.mKeywords.insert(k);

        // Comment styles
        langDef.mCommentStart = "/*";
        langDef.mCommentEnd = "*/";
        langDef.mSingleLineComment = "//";

        // Tokenize function
        langDef.mTokenize = [](const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end, PaletteIndex & paletteIndex) -> bool
        {
            paletteIndex = PaletteIndex::Max;

            while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
                in_begin++;

            if (in_begin == in_end)
            {
                out_begin = in_end;
                out_end = in_end;
                paletteIndex = PaletteIndex::Default;
            }
            else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::String;
            else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::CharLiteral;
			else if (TokenizeCStyleFunctionName(langDef, in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::KnownIdentifier;
            else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::Identifier;
            else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::Number;
            else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::Punctuation;

            return paletteIndex != PaletteIndex::Max;
        };

        // Additional settings
        langDef.mCaseSensitive = true;
        langDef.mName = "Rust";

        inited = true;
    }
    return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::JavaScript()
{
    static bool inited = false;
    static LanguageDefinition langDef;
    if (!inited)
    {
        static const char* const jsKeywords[] = {
            "await", "break", "case", "catch", "class", "const", "continue", "debugger", "default", "delete", "do", "else", "enum", "export", "extends", "false", "finally", "for", "function", "if", "import", "in", "instanceof", 
            "new", "null", "return", "super", "switch", "this", "throw", "true", "try", "typeof", "var", "void", "while", "with", "yield"
        };

        for (auto& k : jsKeywords)
            langDef.mKeywords.insert(k);

        static const char* const identifiers[] = {
            "Array", "Boolean", "Date", "Error", "Function", "JSON", "Math", "Number", "Object", "Promise", "RegExp", "String", "Symbol", "TypeError", "ValueError"
        };
        for (auto& k : identifiers)
        {
            Identifier id;
            //id.mDeclaration = "Built-in function";
            langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
        }

        langDef.mTokenize = [](const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end, PaletteIndex & paletteIndex) -> bool
        {
            paletteIndex = PaletteIndex::Max;

            while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
                in_begin++;

            if (in_begin == in_end)
            {
                out_begin = in_end;
                out_end = in_end;
                paletteIndex = PaletteIndex::Default;
            }
            else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::String;
            else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::CharLiteral;
			else if (TokenizeCStyleFunctionName(langDef, in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::KnownIdentifier;
            else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::Identifier;
            else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::Number;
            else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::Punctuation;

            return paletteIndex != PaletteIndex::Max;
        };

        langDef.mCommentStart = "/*";
        langDef.mCommentEnd = "*/";
        langDef.mSingleLineComment = "//";

        langDef.mCaseSensitive = true;

        langDef.mName = "JavaScript";

        inited = true;
    }
    return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Ini()
{
    static bool inited = false;
    static LanguageDefinition langDef;
    if (!inited)
    {
        langDef.mCommentStart = "\n";
        langDef.mCommentEnd = "\n";
        langDef.mSingleLineComment = ";";

        langDef.mTokenize = [](const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end, PaletteIndex & paletteIndex) -> bool
        {
            paletteIndex = PaletteIndex::Max;

            while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
                in_begin++;

            if (in_begin == in_end)
            {
                out_begin = in_end;
                out_end = in_end;
                paletteIndex = PaletteIndex::Default;
            }
			else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::String;
			else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::CharLiteral;
			else if (TokenizeIniVariableName(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::KnownIdentifier;
			else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Identifier;
			else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::Punctuation;

            return paletteIndex != PaletteIndex::Max;
        };

        langDef.mCaseSensitive = false;
		langDef.mPreprocChar = 0;
        langDef.mName = "Ini";

        inited = true;
    }
    return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Html()
{
    static bool inited = false;
    static LanguageDefinition langDef;
    if (!inited)
    {
		langDef = Css();
        static const char* const keywords[] = {
            "a", "abbr", "address", "area", "article", "aside", "audio", "b", "base", "bdi", "bdo", "blockquote", "body", "br", "button",
            "canvas", "caption", "cite", "code", "col", "colgroup", "data", "datalist", "dd", "del", "details", "dfn", "dialog", "div",
            "dl", "dt", "em", "embed", "fieldset", "figcaption", "figure", "footer", "form", "h1", "h2", "h3", "h4", "h5", "h6", "head",
            "header", "hr", "html", "i", "iframe", "img", "input", "ins", "kbd", "label", "legend", "li", "link", "main", "map", "mark",
            "meta", "meter", "nav", "noscript", "object", "ol", "optgroup", "option", "output", "p", "picture", "pre", "progress", "q",
            "rp", "rt", "ruby", "s", "samp", "script", "section", "select", "small", "source", "span", "strong", "style", "sub", "summary",
            "sup", "table", "tbody", "td", "template", "textarea", "tfoot", "th", "thead", "time", "title", "tr", "track", "u", "ul",
            "var", "video", "wbr"
        };

        for (auto& k : keywords)
            langDef.mKeywords.insert(k);

        /*static const char* const identifiers[] = {
            "class", "id", "src", "href", "alt", "title", "style", "type", "name", "value", "placeholder", "checked", "disabled", "readonly",
            "selected", "required", "multiple", "maxlength", "minlength", "min", "max", "step", "pattern", "size", "rows", "cols", "wrap",
            "target", "rel", "media", "download", "accept", "autocomplete", "autofocus", "autoplay", "controls", "loop", "muted", "poster",
            "preload", "width", "height", "charset", "content", "http-equiv", "language", "defer", "async", "crossorigin", "integrity"
        };

        for (auto& k : identifiers)
        {
            Identifier id;
            //id.mDeclaration = "HTML attribute";
            langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
        }*/

        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(\"[^\"]*\")##", PaletteIndex::String));
        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(\'[^\']*\')##", PaletteIndex::String));
        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+))##", PaletteIndex::Number));
        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([a-zA-Z_][a-zA-Z0-9_\-]*)##", PaletteIndex::Identifier));
        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([<>\/\=\!])##", PaletteIndex::Punctuation));

        langDef.mCommentStart = "<!--";
        langDef.mCommentEnd = "-->";
        langDef.mSingleLineComment = "";

        langDef.mCaseSensitive = false;

        langDef.mName = "HTML";

        inited = true;
    }
    return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Css()
{
    static bool inited = false;
    static LanguageDefinition langDef;
    if (!inited)
    {
        static const char* const keywords[] = {
            "align-content", "align-items", "align-self", "all", "animation", "animation-delay", "animation-direction", 
            "animation-duration", "animation-fill-mode", "animation-iteration-count", "animation-name", "animation-play-state", 
            "animation-timing-function", "backface-visibility", "background", "background-attachment", "background-blend-mode", 
            "background-clip", "background-color", "background-image", "background-origin", "background-position", "background-repeat", 
            "background-size", "border", "border-bottom", "border-bottom-color", "border-bottom-left-radius", "border-bottom-right-radius", 
            "border-bottom-style", "border-bottom-width", "border-collapse", "border-color", "border-image", "border-image-outset", 
            "border-image-repeat", "border-image-slice", "border-image-source", "border-image-width", "border-left", "border-left-color", 
            "border-left-style", "border-left-width", "border-radius", "border-right", "border-right-color", "border-right-style", 
            "border-right-width", "border-spacing", "border-style", "border-top", "border-top-color", "border-top-left-radius", 
            "border-top-right-radius", "border-top-style", "border-top-width", "border-width", "bottom", "box-shadow", "box-sizing", 
            "caption-side", "clear", "clip", "color", "column-count", "column-fill", "column-gap", "column-rule", "column-rule-color", 
            "column-rule-style", "column-rule-width", "column-span", "column-width", "columns", "content", "counter-increment", 
            "counter-reset", "cursor", "direction", "display", "empty-cells", "filter", "flex", "flex-basis", "flex-direction", 
            "flex-flow", "flex-grow", "flex-shrink", "flex-wrap", "float", "font", "font-family", "font-feature-settings", "font-kerning", 
            "font-language-override", "font-size", "font-size-adjust", "font-stretch", "font-style", "font-synthesis", "font-variant", 
            "font-variant-alternates", "font-variant-caps", "font-variant-east-asian", "font-variant-ligatures", "font-variant-numeric", 
            "font-variant-position", "font-weight", "grid", "grid-area", "grid-auto-columns", "grid-auto-flow", "grid-auto-rows", 
            "grid-column", "grid-column-end", "grid-column-gap", "grid-column-start", "grid-gap", "grid-row", "grid-row-end", "grid-row-gap", 
            "grid-row-start", "grid-template", "grid-template-areas", "grid-template-columns", "grid-template-rows", "hanging-punctuation", 
            "height", "hyphens", "image-rendering", "isolation", "justify-content", "left", "letter-spacing", "line-break", "line-height", 
            "list-style", "list-style-image", "list-style-position", "list-style-type", "margin", "margin-bottom", "margin-left", 
            "margin-right", "margin-top", "max-height", "max-width", "min-height", "min-width", "mix-blend-mode", "object-fit", 
            "object-position", "opacity", "order", "orphans", "outline", "outline-color", "outline-offset", "outline-style", 
            "outline-width", "overflow", "overflow-wrap", "overflow-x", "overflow-y", "padding", "padding-bottom", "padding-left", 
            "padding-right", "padding-top", "page-break-after", "page-break-before", "page-break-inside", "perspective", "perspective-origin", 
            "pointer-events", "position", "quotes", "resize", "right", "scroll-behavior", "tab-size", "table-layout", "text-align", 
            "text-align-last", "text-combine-upright", "text-decoration", "text-decoration-color", "text-decoration-line", 
            "text-decoration-style", "text-indent", "text-justify", "text-orientation", "text-overflow", "text-shadow", "text-transform", 
            "text-underline-position", "top", "transform", "transform-origin", "transform-style", "transition", "transition-delay", 
            "transition-duration", "transition-property", "transition-timing-function", "unicode-bidi", "user-select", "vertical-align", 
            "visibility", "white-space", "widows", "width", "word-break", "word-spacing", "word-wrap", "writing-mode", "z-index"
        };

        for (auto& k : keywords)
            langDef.mKeywords.insert(k);

        static const char* const identifiers[] = {
            "absolute", "auto", "block", "bold", "both", "break-word", "center", "clip", "collapse", "content-box", "cover", "dashed", 
            "dotted", "double", "ease", "ease-in", "ease-in-out", "ease-out", "fixed", "flex", "hidden", "inherit", "initial", "inline", 
            "inline-block", "inline-flex", "italic", "justify", "large", "left", "lighter", "line-through", "medium", "none", "normal", 
            "nowrap", "pre", "pre-line", "pre-wrap", "relative", "repeat", "repeat-x", "repeat-y", "revert", "right", "scroll", "separate", 
            "small", "solid", "static", "sticky", "stretch", "thin", "thick", "underline", "unset", "uppercase", "visible"
        };

        for (auto& k : identifiers)
        {
            Identifier id;
            //id.mDeclaration = "CSS value";
            langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
        }

        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(\"[^\"]*\")##", PaletteIndex::String)); // Double-quoted strings
        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(\'[^\']*\')##", PaletteIndex::String)); // Single-quoted strings
        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##(\#[0-9a-fA-F]{3,6})##", PaletteIndex::Number)); // Hex colors
        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([+-]?([0-9]*[.])?[0-9]+(px|em|rem|%|vh|vw|vmin|vmax|ch|ex|cm|mm|in|pt|pc)?)##", PaletteIndex::Number)); // Numbers with units
        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([a-zA-Z_-][a-zA-Z0-9_-]*)##", PaletteIndex::Identifier)); // Identifiers
        langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>(R"##([{}\[\]:;,.\/])##", PaletteIndex::Punctuation)); // Punctuation

        langDef.mCommentStart = "/*";
        langDef.mCommentEnd = "*/";
        langDef.mSingleLineComment = "";

        langDef.mCaseSensitive = false;

        langDef.mName = "CSS";

        inited = true;
    }
    return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Gml()
{
    static bool inited = false;
    static LanguageDefinition langDef;
    if (!inited)
    {
        static const char* const keywords[] = {
            "begin", "end", "if", "else", "while", "for", "do", "switch", "case", "break", "continue", "return", "function",
            "var", "globalvar", "with", "repeat", "until", "enum", "and", "or", "not", "xor", "true", "false"
        };
        for (auto& k : keywords)
            langDef.mKeywords.insert(k);

        static const char* const identifiers[] = {
            "abs", "arccos", "arcsin", "arctan", "ceil", "cos", "exp", "floor", "log", "round", "sin", "sqrt", "tan",
            "draw_text", "draw_sprite", "instance_create", "instance_destroy", "keyboard_check", "mouse_check_button",
        };
        for (auto& k : identifiers)
        {
            Identifier id;
            id.mDeclaration = "Built-in function";
            langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
        }

        // Tokenizer function for GML
        langDef.mTokenize = [](const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex) -> bool
        {
            paletteIndex = PaletteIndex::Max;

            while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
                in_begin++;

            if (in_begin == in_end)
            {
                out_begin = in_end;
                out_end = in_end;
                paletteIndex = PaletteIndex::Default;
            }
            else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::String;
			else if (TokenizeCStyleFunctionName(langDef, in_begin, in_end, out_begin, out_end))
				paletteIndex = PaletteIndex::KnownIdentifier;
            else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::Identifier;
            else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::Number;
            else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
                paletteIndex = PaletteIndex::Punctuation;

            return paletteIndex != PaletteIndex::Max;
        };

        langDef.mCommentStart = "/*";
        langDef.mCommentEnd = "*/";
        langDef.mSingleLineComment = "//";

        langDef.mCaseSensitive = true;
        langDef.mName = "GML";

        inited = true;
    }
    return langDef;
}

const TextEditor::LanguageDefinition& TextEditor::LanguageDefinition::Text()
{
    static bool inited = false;
    static LanguageDefinition langDef;
    if (!inited)
    {
        langDef.mCommentStart = "\n";
        langDef.mCommentEnd = "\n";
        langDef.mSingleLineComment = "\n";
        // Tokenize function
        langDef.mTokenize = [](const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end, PaletteIndex & paletteIndex) -> bool
        {
            paletteIndex = PaletteIndex::Max;

            while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
                in_begin++;

            if (in_begin == in_end)
            {
                out_begin = in_end;
                out_end = in_end;
                paletteIndex = PaletteIndex::Default;
            }

            return paletteIndex != PaletteIndex::Max;
        };

        // Additional settings
        langDef.mCaseSensitive = false;
		langDef.mPreprocChar = 0;
        langDef.mName = "Text";

        inited = true;
    }
    return langDef;
}