/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 69
   ======================================================================== */

enum json_token_type
{
    Token_end_of_stream,
    Token_error,
    
    Token_open_brace,
    Token_open_bracket,
    Token_close_brace,
    Token_close_bracket,
    Token_comma,
    Token_colon,
    Token_semi_colon,
    Token_string_literal,
    Token_minus,
    Token_number,
    Token_true,
    Token_false,
    Token_null,
    
    Token_count,
};

struct json_token
{
    json_token_type Type;
    buffer Value;
};

struct json_object;
struct json_field
{
    buffer Label;
    buffer Value;
    json_object *SubObject;
    
    json_field *NextSibling;
};

struct json_object
{
    json_field *FirstField;
    json_object *NextSibling;
};

struct json_parser
{
    buffer Source;
    u64 At;
    b32 HadError;
};

static b32 IsJSONDigit(buffer Source, u64 At)
{
    b32 Result = false;
    if(IsInBounds(Source, At))
    {
        u8 Val = Source.Data[At];
        Result = ((Val >= '0') && (Val <= '9'));
    }
    
    return Result;
}

static b32 IsJSONWhitespace(buffer Source, u64 At)
{
    b32 Result = false;
    if(IsInBounds(Source, At))
    {
        u8 Val = Source.Data[At];
        Result = ((Val == ' ') || (Val == '\t') || (Val == '\n') || (Val == '\r'));
    }
    
    return Result;
}

static b32 IsParsing(json_parser *Parser)
{
    b32 Result = !Parser->HadError && IsInBounds(Parser->Source, Parser->At);
    return Result;
}

static void Error(json_parser *Parser, json_token Token, char const *Message)
{
    Parser->HadError = true;
    fprintf(stderr, "ERROR: \"%.*s\" - %s\n", (u32)Token.Value.Count, (char *)Token.Value.Data, Message);
}

static void ParseKeyword(buffer Source, u64 *At, buffer KeywordRemaining, json_token_type Type, json_token *Result)
{
    if((Source.Count - *At) >= KeywordRemaining.Count)
    {
        buffer Check = Source;
        Check.Data += *At;
        Check.Count = KeywordRemaining.Count;
        if(AreEqual(Check, KeywordRemaining))
        {
            Result->Type = Type;
            Result->Value.Count += KeywordRemaining.Count;
            *At += KeywordRemaining.Count;
        }
    }
}

static json_token GetJSONToken(json_parser *Parser)
{
    json_token Result = {};
    
    buffer Source = Parser->Source;
    u64 At = Parser->At;
    
    while(IsJSONWhitespace(Source, At))
    {
        ++At;
    }
    
    if(IsInBounds(Source, At))
    {
        Result.Type = Token_error;
        Result.Value.Count = 1;
        Result.Value.Data = Source.Data + At;
        u8 Val = Source.Data[At++];
        switch(Val)
        {
            case '{': {Result.Type = Token_open_brace;} break;
            case '[': {Result.Type = Token_open_bracket;} break;
            case '}': {Result.Type = Token_close_brace;} break;
            case ']': {Result.Type = Token_close_bracket;} break;
            case ',': {Result.Type = Token_comma;} break;
            case ':': {Result.Type = Token_colon;} break;
            case ';': {Result.Type = Token_semi_colon;} break;

            case 'f':
            {
                ParseKeyword(Source, &At, CONSTANT_STRING("alse"), Token_false, &Result);
            } break;
            
            case 'n':
            {
                ParseKeyword(Source, &At, CONSTANT_STRING("ull"), Token_null, &Result);
            } break;
            
            case 't':
            {
                ParseKeyword(Source, &At, CONSTANT_STRING("rue"), Token_true, &Result);
            } break;
            
            case '"':
            {
                Result.Type = Token_string_literal;
                
                u64 StringStart = At;
                
                while(IsInBounds(Source, At) && (Source.Data[At] != '"'))
                {
                    if(IsInBounds(Source, (At + 1)) &&
                       (Source.Data[At] == '\\') &&
                       (Source.Data[At + 1] == '"'))
                    {
                        // NOTE(casey): Skip escaped quotation marks
                        ++At;
                    }
                    
                    ++At;
                }
                
                Result.Value.Data = Source.Data + StringStart;
                Result.Value.Count = At - StringStart;
                if(IsInBounds(Source, At))
                {
                    ++At;
                }
            } break;

            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                u64 Start = At - 1;
                Result.Type = Token_number;

                // NOTE(casey): Move past a leading negative sign if one exists
                if((Val == '-') && IsInBounds(Source, At))
                {
                    Val = Source.Data[At++];
                }
                
                // NOTE(casey): If the leading digit wasn't 0, parse any digits before the decimal point
                if(Val != '0')
                {
                    while(IsJSONDigit(Source, At))
                    {
                        ++At;
                    }
                }
                
                // NOTE(casey): If there is a decimal point, parse any digits after the decimal point
                if(IsInBounds(Source, At) && (Source.Data[At] == '.'))
                {
                    ++At;
                    while(IsJSONDigit(Source, At))
                    {
                        ++At;
                    }
                }
                
                // NOTE(casey): If it's in scientific notation, parse any digits after the "e"
                if(IsInBounds(Source, At) && ((Source.Data[At] == 'e') || (Source.Data[At] == 'E')))
                {
                    ++At;
                    
                    if(IsInBounds(Source, At) && ((Source.Data[At] == '+') || (Source.Data[At] == '-')))
                    {
                        ++At;
                    }
                    
                    while(IsJSONDigit(Source, At))
                    {
                        ++At;
                    }
                }
                
                Result.Value.Count = At - Start;
            } break;
            
            default:
            {
            } break;
        }
    }
    
    Parser->At = At;
    
    return Result;
}

static json_object *ParseJSONList(json_parser *Parser, json_token StartingToken, json_token_type EndType, b32 HasLabels);
static json_field *ParseJSONField(json_parser *Parser, buffer Label, json_token Value)
{
    b32 Valid = true;
    
    json_object *SubObject = 0;
    if(Value.Type == Token_open_bracket)
    {
        SubObject = ParseJSONList(Parser, Value, Token_close_bracket, false);
    }
    else if(Value.Type == Token_open_brace)
    {
        SubObject = ParseJSONList(Parser, Value, Token_close_brace, true);
    }
    else if((Value.Type == Token_string_literal) ||
            (Value.Type == Token_true) ||
            (Value.Type == Token_false) ||
            (Value.Type == Token_null) ||
            (Value.Type == Token_number))
    {
        // NOTE(casey): Nothing to do here, since there is no additional data
    }
    else
    {
        Valid = false;
    }
    
    json_field *Result = 0;
    
    if(Valid)
    {
        Result = (json_field *)malloc(sizeof(json_field));
        Result->Label = Label;
        Result->Value = Value.Value;
        Result->SubObject = SubObject;
        Result->NextSibling = 0;
    }
    
    return Result;
}

static json_object *ParseJSONList(json_parser *Parser, json_token StartingToken, json_token_type EndType, b32 HasLabels)
{
    json_field *FirstField = {};
    json_field *LastField = {};
    
    while(IsParsing(Parser))
    {
        buffer Label = {};
        json_token Value = GetJSONToken(Parser);
        if(HasLabels)
        {
            if(Value.Type == Token_string_literal)
            {
                Label = Value.Value;
                
                json_token Colon = GetJSONToken(Parser);
                if(Colon.Type == Token_colon)
                {
                    Value = GetJSONToken(Parser);
                }
                else
                {
                    Error(Parser, Colon, "Expected colon after field name");
                }
            }
            else if(Value.Type != EndType)
            {
                Error(Parser, Value, "Unexpected token in JSON");
            }
        }
        
        json_field *Field = ParseJSONField(Parser, Label, Value);
        if(Field)
        {
            LastField = (LastField ? LastField->NextSibling : FirstField) = Field;
        }
        else if(Value.Type == EndType)
        {
            break;
        }
        else
        {
            Error(Parser, Value, "Unexpected token in JSON");
        }
        
        json_token Comma = GetJSONToken(Parser);
        if(Comma.Type == EndType)
        {
            break;
        }
        else if(Comma.Type != Token_comma)
        {
            Error(Parser, Comma, "Unexpected token in JSON");
        }
    }
    
    json_object *Result = (json_object *)malloc(sizeof(json_object));
    if(Result)
    {
        Result->FirstField = FirstField;
        Result->NextSibling = 0;
    }
    else
    {
        Error(Parser, StartingToken, "Unable to allocate storage for JSON object");
    }
    
    return Result;
}

static json_field *ParseJSON(buffer InputJSON)
{
    json_parser Parser = {};
    Parser.Source = InputJSON;
    
    json_field *Result = ParseJSONField(&Parser, {}, GetJSONToken(&Parser));
    return Result;
}

static void FreeJSON(json_field *Field)
{
    while(Field)
    {
        json_field *FreeField = Field;
        Field = Field->NextSibling;
    
        json_object *Object = FreeField->SubObject;
        while(Object)
        {
            json_object *FreeObject = Object;
            Object = Object->NextSibling;
            
            FreeJSON(FreeObject->FirstField);
            free(FreeObject);
        }        
        free(FreeField);
    }
}

static json_field *LookupField(json_object *Object, buffer FieldName)
{
    json_field *Result = 0;
    
    if(Object)
    {
        for(json_field *Search = Object->FirstField; Search; Search = Search->NextSibling)
        {
            if(AreEqual(Search->Label, FieldName))
            {
                Result = Search;
                break;
            }
        }
    }
    
    return Result;
}

static f64 ConvertJSONSign(buffer Source, u64 *AtResult)
{
    u64 At = *AtResult;

    f64 Result = 1.0;
    if(IsInBounds(Source, At) && (Source.Data[At] == '-'))
    {
        Result = -1.0;
        ++At;
    }
    
    *AtResult = At;

    return Result;
}

static f64 ConvertJSONNumber(buffer Source, u64 *AtResult)
{
    u64 At = *AtResult;
    
    f64 Result = 0.0;
    while(IsInBounds(Source, At))
    {
        u8 Char = Source.Data[At] - (u8)'0';
        if(Char < 10)
        {
            Result = 10.0*Result + (f64)Char;
            ++At;
        }
        else
        {
            break;
        }
    }
    
    *AtResult = At;
    
    return Result;
}

static f64 ConvertFieldToF64(json_object *Object, buffer FieldName)
{
    f64 Result = 0.0;
    
    json_field *Field = LookupField(Object, FieldName);
    if(Field)
    {
        buffer Source = Field->Value;
        u64 At = 0;
        
        f64 Sign = ConvertJSONSign(Source, &At);
        f64 Number = ConvertJSONNumber(Source, &At);
        
        if(IsInBounds(Source, At) && (Source.Data[At] == '.'))
        {
            ++At;
            f64 C = 1.0 / 10.0;
            while(IsInBounds(Source, At))
            {
                u8 Char = Source.Data[At] - (u8)'0';
                if(Char < 10)
                {
                    Number = Number + C*(f64)Char;
                    C *= 1.0 / 10.0;
                    ++At;
                }
                else
                {
                    break;
                }
            }
        }
        
        if(IsInBounds(Source, At) && ((Source.Data[At] == 'e') || (Source.Data[At] == 'E')))
        {
            ++At;
            if(IsInBounds(Source, At) && (Source.Data[At] == '+'))
            {
                ++At;
            }

            f64 ExponentSign = ConvertJSONSign(Source, &At);
            f64 Exponent = ExponentSign*ConvertJSONNumber(Source, &At);
            Number *= pow(10.0, Exponent);
        }
        
        Result = Sign*Number;
    }
    
    return Result;
}

static u64 ParseHaversinePairs(buffer InputJSON, u64 MaxPairCount, haversine_pair *Pairs)
{
    u64 PairCount = 0;
    
    json_field *JSON = ParseJSON(InputJSON);
    json_field *PairsField = LookupField(JSON->SubObject, CONSTANT_STRING("pairs"));
    if(PairsField && PairsField->SubObject)
    {
        for(json_field *Element = PairsField->SubObject->FirstField;
            Element && (PairCount < MaxPairCount);
            Element = Element->NextSibling)
        {
            haversine_pair *Pair = Pairs + PairCount++;
            
            Pair->X0 = ConvertFieldToF64(Element->SubObject, CONSTANT_STRING("x0"));
            Pair->Y0 = ConvertFieldToF64(Element->SubObject, CONSTANT_STRING("y0"));
            Pair->X1 = ConvertFieldToF64(Element->SubObject, CONSTANT_STRING("x1"));
            Pair->Y1 = ConvertFieldToF64(Element->SubObject, CONSTANT_STRING("y1"));
        }
    }
    
    FreeJSON(JSON);
    
    return PairCount;
}
