/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 109
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
    
    RepValue_Count,
};

struct repetition_value
{
    u64 E[RepValue_Count];
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

static f64 SecondsFromCPUTime(f64 CPUTime, u64 CPUTimerFreq)
{
    f64 Result = 0.0;
    if(CPUTimerFreq)
    {
        Result = (CPUTime / (f64)CPUTimerFreq);
    }
    
    return Result;
}
 
static void PrintValue(char const *Label, repetition_value Value, u64 CPUTimerFreq)
{
    u64 TestCount = Value.E[RepValue_TestCount];
    f64 Divisor = TestCount ? (f64)TestCount : 1;
    
    f64 E[RepValue_Count];
    for(u32 EIndex = 0; EIndex < ArrayCount(E); ++EIndex)
    {
        E[EIndex] = (f64)Value.E[EIndex] / Divisor;
    }
    
    printf("%s: %.0f", Label, E[RepValue_CPUTimer]);
    if(CPUTimerFreq)
    {
        f64 Seconds = SecondsFromCPUTime(E[RepValue_CPUTimer], CPUTimerFreq);
        printf(" (%fms)", 1000.0f*Seconds);
    
        if(E[RepValue_ByteCount] > 0)
        {
            f64 Gigabyte = (1024.0f * 1024.0f * 1024.0f);
            f64 Bandwidth = E[RepValue_ByteCount] / (Gigabyte * Seconds);
            printf(" %fgb/s", Bandwidth);
        }
    }

    if(E[RepValue_MemPageFaults] > 0)
    {
        printf(" PF: %0.4f (%0.4fk/fault)", E[RepValue_MemPageFaults], E[RepValue_ByteCount] / (E[RepValue_MemPageFaults] * 1024.0));
    }
}

static void PrintResults(repetition_test_results Results, u64 CPUTimerFreq)
{
    PrintValue("Min", Results.Min, CPUTimerFreq);
    printf("\n");
    PrintValue("Max", Results.Max, CPUTimerFreq);
    printf("\n");
    PrintValue("Avg", Results.Total, CPUTimerFreq);
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
                        PrintValue("Min", Results->Min, Tester->CPUTimerFreq);
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
            
            printf("                                                          \r");
            PrintResults(Tester->Results, Tester->CPUTimerFreq);
        }
    }
    
    b32 Result = (Tester->Mode == TestMode_Testing);
    return Result;
}
