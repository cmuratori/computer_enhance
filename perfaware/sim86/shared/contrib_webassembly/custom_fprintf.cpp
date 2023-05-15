static output_memory OutputMemory;

#define FILE output_memory
#define stdout &OutputMemory
#define stderr &OutputMemory

void ZeroCharBuffer(char Buffer[256]);
s32 Interpolate(char Buffer[256], s32 Number, s32 StartIndex);
s32 Interpolate(char Buffer[256], u32 Number, s32 StartIndex);
s32 InterpolateSigned(char Buffer[256], s32 Number, s32 StartIndex);
s32 InterpolateSigned(char Buffer[256], u32 Number, s32 StartIndex);
void PrintToOutput(char *LogLine, output_memory *OutputBuffer);

void
fprintf(FILE *OutputMemory, const char *Pattern, ...)
{
  char Buffer[256];
  ZeroCharBuffer(Buffer);

  s32 PatternIndex = 0;
  s32 BufferIndex = 0;

  __builtin_va_list args;
  __builtin_va_start(args, Pattern);

  while (Pattern[PatternIndex] != '\0')
  {
    union Printable_t {
      s32     i;
      u32     u;
      char   *s;
    } Printable;

    char Character = Pattern[PatternIndex++];
    if (Character != '%')
    {
      Buffer[BufferIndex++] = Character;
    }
    else 
    {
      Character = Pattern[PatternIndex++];
      if (Character == 'd')
      {
        Printable.i = __builtin_va_arg(args, s32);
        BufferIndex = Interpolate(Buffer, Printable.i, BufferIndex);
      }
      else if (Character == 'u')
      {
        Printable.u = __builtin_va_arg(args, u32);
        BufferIndex = Interpolate(Buffer, Printable.u, BufferIndex);
      }
      else if (Character == 's')
      {
        char *String = __builtin_va_arg(args, char *);
        while (*String != '\0')
        {
          Buffer[BufferIndex++] = *String++;
        }
      }
      else if (Character == '+')
      {
        PatternIndex++;
        Printable.i = __builtin_va_arg(args, s32);
        BufferIndex = InterpolateSigned(Buffer, Printable.i, BufferIndex);
      }
    }
  }

  __builtin_va_end(args);

  PrintToOutput((char *)Buffer, OutputMemory);
}

void
ZeroCharBuffer(char Buffer[256])
{
  for (s32 Index = 0; Index < 256; ++Index)
  {
    Buffer[Index] = '\0';
  }
}

inline s32 
NumberOfDigits(s64 Number)
{
  s32 NumDigits = 0;
  if (Number < 0)
  {
    Number *= -1;
  }

  while (Number)
  {
    s32 Digit = (s32)(Number % 10);
    Number = Number / 10;
    NumDigits++;
  }

  return NumDigits;
}

inline void
PutDigitsInBuffer(char Buffer[256], s32 Number, s32 NumDigits, s32 StartIndex = 0)
{
  for (s32 DigitIndex = StartIndex + NumDigits - 1; 
      DigitIndex >= StartIndex;
      --DigitIndex)
  {
    s32 Digit = Number % 10;
    Number = Number / 10;
    Buffer[DigitIndex] = Digit + 48;
  }
}

s32
Interpolate(char Buffer[256], s32 Number, s32 StartIndex = 0)
{
  s32 NumDigits = NumberOfDigits((s64)Number);
  if (Number < 0)
  {
    Number *= -1;
    Buffer[StartIndex++] = '-';
  }

  PutDigitsInBuffer(Buffer, Number, NumDigits, StartIndex);

  return StartIndex + NumDigits;
}

s32
Interpolate(char Buffer[256], u32 Number, s32 StartIndex = 0)
{
  s32 NumDigits = NumberOfDigits((s64)Number);

  PutDigitsInBuffer(Buffer, Number, NumDigits, StartIndex);

  return StartIndex + NumDigits;
}

s32
InterpolateSigned(char Buffer[256], s32 Number, s32 StartIndex = 0)
{
  if (Number < 0)
  {
    Number *= -1;
    Buffer[StartIndex++] = '-';
  } 
  else 
  {
    Buffer[StartIndex++] = '+';
  }

  s32 NumDigits = NumberOfDigits((s64)Number);

  PutDigitsInBuffer(Buffer, Number, NumDigits, StartIndex);

  return StartIndex;
}

s32
InterpolateSigned(char Buffer[256], u32 Number, s32 StartIndex = 0)
{
  Buffer[StartIndex++] = '+';
  s32 NumDigits = NumberOfDigits((s64)Number);

  PutDigitsInBuffer(Buffer, Number, NumDigits, StartIndex);

  return StartIndex;
}

s32
Concat(char Buffer[256], char *Arg1, char *Arg2, s32 StartIndex = 0)
{
  while (*Arg1 && StartIndex < 256)
  {
    Buffer[StartIndex++] = *Arg1++;
  }

  while (*Arg2 && StartIndex < 256)
  {
    Buffer[StartIndex++] = *Arg2++;
  }

  return StartIndex;
}

void
PrintToOutput(char *LogLine, output_memory *OutputBuffer)
{
  if (OutputBuffer->Used + 256 == OutputBuffer->Max)
  {
    OutputBuffer->Used = 0;
  }

  u32 LogLineLength = 0;
  u8 *BuffCursor = OutputBuffer->Base + OutputBuffer->Used;
  // Log lines beyond this will get truncated
  while (LogLineLength < 256)
  {
    if (*LogLine)
    {
      *BuffCursor++ = *LogLine++;
    } 
    else 
    {
      *BuffCursor++ = '\n';
      break;
    }
    LogLineLength++;
  }

  OutputBuffer->Used += LogLineLength;
}

void 
memcpy(u8 *Destination, u8 *Source, u32 SourceSize)
{
	__builtin_memcpy(Destination, Source, SourceSize);
}
