/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 103
   ======================================================================== */

enum test_mode : u32
{
    TestMode_Uninitialized,
    TestMode_Testing,
    TestMode_Completed,
    TestMode_Error,
};

struct repetition_test_results
{
    u64 TestCount;
    u64 TotalTime;
    u64 MaxTime;
    u64 MinTime;
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
    u64 TimeAccumulatedOnThisTest;
    u64 BytesAccumulatedOnThisTest;

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
 
static void PrintTime(char const *Label, f64 CPUTime, u64 CPUTimerFreq, u64 ByteCount)
{
    printf("%s: %.0f", Label, CPUTime);
    if(CPUTimerFreq)
    {
        f64 Seconds = SecondsFromCPUTime(CPUTime, CPUTimerFreq);
        printf(" (%fms)", 1000.0f*Seconds);
    
        if(ByteCount)
        {
            f64 Gigabyte = (1024.0f * 1024.0f * 1024.0f);
            f64 BestBandwidth = ByteCount / (Gigabyte * Seconds);
            printf(" %fgb/s", BestBandwidth);
        }
    }
}

static void PrintTime(char const *Label, u64 CPUTime, u64 CPUTimerFreq, u64 ByteCount)
{
    PrintTime(Label, (f64)CPUTime, CPUTimerFreq, ByteCount);
}

static void PrintResults(repetition_test_results Results, u64 CPUTimerFreq, u64 ByteCount)
{
    PrintTime("Min", (f64)Results.MinTime, CPUTimerFreq, ByteCount);
    printf("\n");
    
    PrintTime("Max", (f64)Results.MaxTime, CPUTimerFreq, ByteCount);
    printf("\n");
    
    if(Results.TestCount)
    {
        PrintTime("Avg", (f64)Results.TotalTime / (f64)Results.TestCount, CPUTimerFreq, ByteCount);
        printf("\n");
    }
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
        Tester->Results.MinTime = (u64)-1;
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
    Tester->TimeAccumulatedOnThisTest -= ReadCPUTimer();
}

static void EndTime(repetition_tester *Tester)
{
    ++Tester->CloseBlockCount;
    Tester->TimeAccumulatedOnThisTest += ReadCPUTimer();
}

static void CountBytes(repetition_tester *Tester, u64 ByteCount)
{
    Tester->BytesAccumulatedOnThisTest += ByteCount;
}

static b32 IsTesting(repetition_tester *Tester)
{
    if(Tester->Mode == TestMode_Testing)
    {
        u64 CurrentTime = ReadCPUTimer();
        
        if(Tester->OpenBlockCount) // NOTE(casey): We don't count tests that had no timing blocks - we assume they took some other path
        {
            if(Tester->OpenBlockCount != Tester->CloseBlockCount)
            {
                Error(Tester, "Unbalanced BeginTime/EndTime");
            }
            
            if(Tester->BytesAccumulatedOnThisTest != Tester->TargetProcessedByteCount)
            {
                Error(Tester, "Processed byte count mismatch");
            }
    
            if(Tester->Mode == TestMode_Testing)
            {
                repetition_test_results *Results = &Tester->Results;
                u64 ElapsedTime = Tester->TimeAccumulatedOnThisTest;
                Results->TestCount += 1;
                Results->TotalTime += ElapsedTime;
                if(Results->MaxTime < ElapsedTime)
                {
                    Results->MaxTime = ElapsedTime;
                }
                
                if(Results->MinTime > ElapsedTime)
                {
                    Results->MinTime = ElapsedTime;
                    
                    // NOTE(casey): Whenever we get a new minimum time, we reset the clock to the full trial time
                    Tester->TestsStartedAt = CurrentTime;
                    
                    if(Tester->PrintNewMinimums)
                    {
                        PrintTime("Min", Results->MinTime, Tester->CPUTimerFreq, Tester->BytesAccumulatedOnThisTest);
                        printf("               \r");
                    }
                }
                
                Tester->OpenBlockCount = 0;
                Tester->CloseBlockCount = 0;
                Tester->TimeAccumulatedOnThisTest = 0;
                Tester->BytesAccumulatedOnThisTest = 0;
            }
        }
        
        if((CurrentTime - Tester->TestsStartedAt) > Tester->TryForTime)
        {
            Tester->Mode = TestMode_Completed;
            
            printf("                                                          \r");
            PrintResults(Tester->Results, Tester->CPUTimerFreq, Tester->TargetProcessedByteCount);
        }
    }
    
    b32 Result = (Tester->Mode == TestMode_Testing);
    return Result;
}
