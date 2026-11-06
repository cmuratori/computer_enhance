/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 164
   ======================================================================== */

enum test_mode : u32
{
    TestMode_Uninitialized,
    TestMode_Testing,
    TestMode_Completed,
    TestMode_Error,
};

enum repetition_value_type
{
    RepValue_TestCount,
    
    RepValue_CPUTimer,
    RepValue_MemPageFaults,
    RepValue_ByteCount,
    
    StatValue_Seconds,
    StatValue_GBPerSecond,
    StatValue_KBPerPageFault,
    
    RepValue_Count,
};

struct repetition_value
{
    u64 E[RepValue_Count];
    
    // NOTE(casey): These values are computed from the E[] array and the CPUTimerFreq
    f64 PerCount[RepValue_Count];
};

struct repetition_test_results
{
    repetition_value Total;
    repetition_value Min;
    repetition_value Max;
};

struct repetition_tester
{
    u64 TargetProcessedByteCount;
    u64 CPUTimerFreq;
    u64 TryForTime;
    u64 TestsStartedAt;
    
    test_mode Mode;
    b32 PrintNewMinimums;
    u32 OpenBlockCount;
    u32 CloseBlockCount;
    
    repetition_value AccumulatedOnThisTest;
    repetition_test_results Results;
};

struct repetition_series_label
{
    char Chars[64];
};

struct repetition_test_series
{
    buffer Memory;

    u32 MaxRowCount;
    u32 ColumnCount;
    
    u32 RowIndex;
    u32 ColumnIndex;
    
    repetition_test_results *TestResults; // NOTE(casey): [RowCount][ColumnCount]
    repetition_series_label *RowLabels; // NOTE(casey): [RowCount]
    repetition_series_label *ColumnLabels; // NOTE(casey): [ColumnCount]
    
    repetition_series_label RowLabelLabel;
};

static f64 SecondsFromCPUTime(f64 CPUTime, u64 CPUTimerFreq)
{
    f64 Result = 0.0;
    if(CPUTimerFreq)
    {
        Result = (CPUTime / (f64)CPUTimerFreq);
    }
    
    return Result;
}
 
static void ComputeDerivedValues(repetition_value *Value, u64 CPUTimerFreq)
{
    u64 TestCount = Value->E[RepValue_TestCount];
    f64 Divisor = TestCount ? (f64)TestCount : 1;
    
    f64 *PerCount = Value->PerCount;
    for(u32 EIndex = 0; EIndex < ArrayCount(Value->PerCount); ++EIndex)
    {
        PerCount[EIndex] = (f64)Value->E[EIndex] / Divisor;
    }
    
    if(CPUTimerFreq)
    {
        f64 Seconds = SecondsFromCPUTime(PerCount[RepValue_CPUTimer], CPUTimerFreq);
        PerCount[StatValue_Seconds] = Seconds;
        
        if(PerCount[RepValue_ByteCount] > 0)
        {
            f64 Gigabyte = (1024.0f * 1024.0f * 1024.0f);
            PerCount[StatValue_GBPerSecond] = PerCount[RepValue_ByteCount] / (Gigabyte * Seconds);
        }
    }
    
    if(PerCount[RepValue_MemPageFaults] > 0)
    {
        PerCount[StatValue_KBPerPageFault] = PerCount[RepValue_ByteCount] / (PerCount[RepValue_MemPageFaults] * 1024.0);
    }
}

static void PrintValue(char const *Label, repetition_value Value)
{
    printf("%s: %.0f", Label, Value.PerCount[RepValue_CPUTimer]);
    printf(" (%fms)", 1000.0f*Value.PerCount[StatValue_Seconds]);
    if(Value.PerCount[RepValue_ByteCount] > 0)
    {
        printf(" %fgb/s", Value.PerCount[StatValue_GBPerSecond]);
    }
    
    if(Value.PerCount[StatValue_KBPerPageFault])
    {
        printf(" PF: %0.4f (%0.4fk/fault)", Value.PerCount[RepValue_MemPageFaults], Value.PerCount[StatValue_KBPerPageFault]);
    }
}

static void PrintResults(repetition_test_results Results)
{
    PrintValue("Min", Results.Min);
    printf("\n");
    PrintValue("Max", Results.Max);
    printf("\n");
    PrintValue("Avg", Results.Total);
    printf("\n");
}

static void Error(repetition_tester *Tester, char const *Message)
{
    Tester->Mode = TestMode_Error;
    fprintf(stderr, "ERROR: %s\n", Message);
}

static void NewTestWave(repetition_tester *Tester, u64 TargetProcessedByteCount, u64 CPUTimerFreq, u32 SecondsToTry = 10)
{
    if(Tester->Mode == TestMode_Uninitialized)
    {
        Tester->Mode = TestMode_Testing;
        Tester->TargetProcessedByteCount = TargetProcessedByteCount;
        Tester->CPUTimerFreq = CPUTimerFreq;
        Tester->PrintNewMinimums = true;
        Tester->Results.Min.E[RepValue_CPUTimer] = (u64)-1;
    }
    else if(Tester->Mode == TestMode_Completed)
    {
        Tester->Mode = TestMode_Testing;
        
        if(Tester->TargetProcessedByteCount != TargetProcessedByteCount)
        {
            Error(Tester, "TargetProcessedByteCount changed");
        }
        
        if(Tester->CPUTimerFreq != CPUTimerFreq)
        {
            Error(Tester, "CPU frequency changed");
        }
    }

    Tester->TryForTime = SecondsToTry*CPUTimerFreq;
    Tester->TestsStartedAt = ReadCPUTimer();
}

static void BeginTime(repetition_tester *Tester)
{
    ++Tester->OpenBlockCount;
    
    repetition_value *Accum = &Tester->AccumulatedOnThisTest;
    Accum->E[RepValue_MemPageFaults] -= ReadOSPageFaultCount();
    Accum->E[RepValue_CPUTimer] -= ReadCPUTimer();
}

static void EndTime(repetition_tester *Tester)
{
    repetition_value *Accum = &Tester->AccumulatedOnThisTest;
    Accum->E[RepValue_CPUTimer] += ReadCPUTimer();
    Accum->E[RepValue_MemPageFaults] += ReadOSPageFaultCount();

    ++Tester->CloseBlockCount;
}

static void CountBytes(repetition_tester *Tester, u64 ByteCount)
{
    repetition_value *Accum = &Tester->AccumulatedOnThisTest;
    Accum->E[RepValue_ByteCount] += ByteCount;
}

static b32 IsTesting(repetition_tester *Tester)
{
    if(Tester->Mode == TestMode_Testing)
    {
        repetition_value Accum = Tester->AccumulatedOnThisTest;
        u64 CurrentTime = ReadCPUTimer();
        
        if(Tester->OpenBlockCount) // NOTE(casey): We don't count tests that had no timing blocks - we assume they took some other path
        {
            if(Tester->OpenBlockCount != Tester->CloseBlockCount)
            {
                Error(Tester, "Unbalanced BeginTime/EndTime");
            }
            
            if(Accum.E[RepValue_ByteCount] != Tester->TargetProcessedByteCount)
            {
                Error(Tester, "Processed byte count mismatch");
            }
    
            if(Tester->Mode == TestMode_Testing)
            {
                repetition_test_results *Results = &Tester->Results;
                
                Accum.E[RepValue_TestCount] = 1;
                for(u32 EIndex = 0; EIndex < ArrayCount(Accum.E); ++EIndex)
                {
                    Results->Total.E[EIndex] += Accum.E[EIndex];
                }
                
                if(Results->Max.E[RepValue_CPUTimer] < Accum.E[RepValue_CPUTimer])
                {
                    Results->Max = Accum;
                }
                
                if(Results->Min.E[RepValue_CPUTimer] > Accum.E[RepValue_CPUTimer])
                {
                    Results->Min = Accum;
                    
                    // NOTE(casey): Whenever we get a new minimum time, we reset the clock to the full trial time
                    Tester->TestsStartedAt = CurrentTime;
                    
                    if(Tester->PrintNewMinimums)
                    {
                        ComputeDerivedValues(&Results->Min, Tester->CPUTimerFreq);
                        PrintValue("Min", Results->Min);
                        printf("                                   \r");
                    }
                }
                
                Tester->OpenBlockCount = 0;
                Tester->CloseBlockCount = 0;
                Tester->AccumulatedOnThisTest = {};
            }
        }
        
        if((CurrentTime - Tester->TestsStartedAt) > Tester->TryForTime)
        {
            Tester->Mode = TestMode_Completed;

            ComputeDerivedValues(&Tester->Results.Total, Tester->CPUTimerFreq);
            ComputeDerivedValues(&Tester->Results.Min, Tester->CPUTimerFreq);
            ComputeDerivedValues(&Tester->Results.Max, Tester->CPUTimerFreq);
            
            printf("                                                          \r");
            PrintResults(Tester->Results);
        }
    }
    
    b32 Result = (Tester->Mode == TestMode_Testing);
    return Result;
}

static repetition_test_series AllocateTestSeries(u32 ColumnCount, u32 MaxRowCount)
{
    repetition_test_series Series = {};
    
    u64 TestResultsSize = (ColumnCount*MaxRowCount)*sizeof(repetition_test_results);
    u64 RowLabelsSize = (MaxRowCount)*sizeof(repetition_series_label);
    u64 ColumnLabelsSize = (ColumnCount)*sizeof(repetition_series_label);
    
    Series.Memory = AllocateBuffer(TestResultsSize + RowLabelsSize + ColumnLabelsSize);
    if(IsValid(Series.Memory))
    {
        Series.MaxRowCount = MaxRowCount;
        Series.ColumnCount = ColumnCount;
        
        Series.TestResults = (repetition_test_results *)Series.Memory.Data;
        Series.RowLabels = (repetition_series_label *)((u8 *)Series.TestResults + TestResultsSize);
        Series.ColumnLabels = (repetition_series_label *)((u8 *)Series.RowLabels + RowLabelsSize);
    }
    
    return Series;
}

static b32 IsValid(repetition_test_series Series)
{
    b32 Result = IsValid(Series.Memory);
    return Result;
}

static void FreeTestSeries(repetition_test_series *Series)
{
    if(Series)
    {
        FreeBuffer(&Series->Memory);
        *Series = {};
    }
}

static b32 IsInBounds(repetition_test_series *Series)
{
    b32 Result = ((Series->ColumnIndex < Series->ColumnCount) &&
                  (Series->RowIndex < Series->MaxRowCount));
    return Result;
}

static void SetRowLabelLabel(repetition_test_series *Series, char const *Format, ...)
{
    repetition_series_label *Label = &Series->RowLabelLabel;
    va_list Args;
    va_start(Args, Format);
    vsnprintf(Label->Chars, sizeof(Label->Chars), Format, Args);
    va_end(Args);
}

static void SetRowLabel(repetition_test_series *Series, char const *Format, ...)
{
    if(IsInBounds(Series))
    {
        repetition_series_label *Label = Series->RowLabels + Series->RowIndex;
        va_list Args;
        va_start(Args, Format);
        vsnprintf(Label->Chars, sizeof(Label->Chars), Format, Args);
        va_end(Args);
    }
}

static void SetColumnLabel(repetition_test_series *Series, char const *Format, ...)
{
    if(IsInBounds(Series))
    {
        repetition_series_label *Label = Series->ColumnLabels + Series->ColumnIndex;
        va_list Args;
        va_start(Args, Format);
        vsnprintf(Label->Chars, sizeof(Label->Chars), Format, Args);
        va_end(Args);
    }
}

static void NewTestWave(repetition_test_series *Series,
                        repetition_tester *Tester, u64 TargetProcessedByteCount, u64 CPUTimerFreq, u32 SecondsToTry = 10)
{
    if(IsInBounds(Series))
    {
        printf("\n--- %s %s ---\n",
               Series->ColumnLabels[Series->ColumnIndex].Chars,
               Series->RowLabels[Series->RowIndex].Chars);
    }
    
    NewTestWave(Tester, TargetProcessedByteCount, CPUTimerFreq, SecondsToTry);
}

static repetition_test_results *GetTestResults(repetition_test_series *Series, u32 ColumnIndex, u32 RowIndex)
{
    repetition_test_results *Result = 0;
    if((ColumnIndex < Series->ColumnCount) && (RowIndex < Series->MaxRowCount))
    {
        Result = Series->TestResults + (RowIndex*Series->ColumnCount + ColumnIndex);
    }
    
    return Result;
}

static b32 IsTesting(repetition_test_series *Series, repetition_tester *Tester)
{
    b32 Result = IsTesting(Tester);
    
    if(!Result)
    {
        if(IsInBounds(Series))
        {
            *GetTestResults(Series, Series->ColumnIndex, Series->RowIndex) = Tester->Results;
            
            if(++Series->ColumnIndex >= Series->ColumnCount)
            {
                Series->ColumnIndex = 0;
                ++Series->RowIndex;
            }
        }
    }
    
    return Result;
}

inline void PrintCSVForValue(repetition_test_series *Series, repetition_value_type ValueType, FILE *Dest, f64 Coefficient = 1.0f)
{
    fprintf(Dest, "%s", Series->RowLabelLabel.Chars);
    for(u32 ColumnIndex = 0; ColumnIndex < Series->ColumnCount; ++ColumnIndex)
    {
        fprintf(Dest, ",%s", Series->ColumnLabels[ColumnIndex].Chars);
    }
    fprintf(Dest, "\n");

    for(u32 RowIndex = 0; RowIndex < Series->RowIndex; ++RowIndex)
    {
        fprintf(Dest, "%s", Series->RowLabels[RowIndex].Chars);
        for(u32 ColumnIndex = 0; ColumnIndex < Series->ColumnCount; ++ColumnIndex)
        {
            repetition_test_results *TestResults = GetTestResults(Series, ColumnIndex, RowIndex);
            fprintf(Dest, ",%f", Coefficient*TestResults->Min.PerCount[ValueType]);
        }
        fprintf(Dest, "\n");
    }
}
