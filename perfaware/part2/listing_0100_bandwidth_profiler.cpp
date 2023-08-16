/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 100
   ======================================================================== */

#include "listing_0074_platform_metrics.cpp"

#ifndef PROFILER
#define PROFILER 0
#endif

#ifndef READ_BLOCK_TIMER
#define READ_BLOCK_TIMER ReadCPUTimer
#endif

#if PROFILER

struct profile_anchor
{
    u64 TSCElapsedExclusive; // NOTE(casey): Does NOT include children
    u64 TSCElapsedInclusive; // NOTE(casey): DOES include children
    u64 HitCount;
    u64 ProcessedByteCount;
    char const *Label;
};
static profile_anchor GlobalProfilerAnchors[4096];
static u32 GlobalProfilerParent;

struct profile_block
{
    profile_block(char const *Label_, u32 AnchorIndex_, u64 ByteCount)
    {
        ParentIndex = GlobalProfilerParent;
        
        AnchorIndex = AnchorIndex_;
        Label = Label_;

        profile_anchor *Anchor = GlobalProfilerAnchors + AnchorIndex;
        OldTSCElapsedInclusive = Anchor->TSCElapsedInclusive;
        Anchor->ProcessedByteCount += ByteCount;
        
        GlobalProfilerParent = AnchorIndex;
        StartTSC = READ_BLOCK_TIMER();
    }
    
    ~profile_block(void)
    {
        u64 Elapsed = READ_BLOCK_TIMER() - StartTSC;
        GlobalProfilerParent = ParentIndex;
    
        profile_anchor *Parent = GlobalProfilerAnchors + ParentIndex;
        profile_anchor *Anchor = GlobalProfilerAnchors + AnchorIndex;
        
        Parent->TSCElapsedExclusive -= Elapsed;
        Anchor->TSCElapsedExclusive += Elapsed;
        Anchor->TSCElapsedInclusive = OldTSCElapsedInclusive + Elapsed;
        ++Anchor->HitCount;
        
        /* NOTE(casey): This write happens every time solely because there is no
           straightforward way in C++ to have the same ease-of-use. In a better programming
           language, it would be simple to have the anchor points gathered and labeled at compile
           time, and this repetative write would be eliminated. */
        Anchor->Label = Label;
    }
    
    char const *Label;
    u64 OldTSCElapsedInclusive;
    u64 StartTSC;
    u32 ParentIndex;
    u32 AnchorIndex;
};

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBandwidth(Name, ByteCount) profile_block NameConcat(Block, __LINE__)(Name, __COUNTER__ + 1, ByteCount)
#define ProfilerEndOfCompilationUnit static_assert(__COUNTER__ < ArrayCount(GlobalProfilerAnchors), "Number of profile points exceeds size of profiler::Anchors array")

static void PrintTimeElapsed(u64 TotalTSCElapsed, u64 TimerFreq, profile_anchor *Anchor)
{
    f64 Percent = 100.0 * ((f64)Anchor->TSCElapsedExclusive / (f64)TotalTSCElapsed);
    printf("  %s[%llu]: %llu (%.2f%%", Anchor->Label, Anchor->HitCount, Anchor->TSCElapsedExclusive, Percent);
    if(Anchor->TSCElapsedInclusive != Anchor->TSCElapsedExclusive)
    {
        f64 PercentWithChildren = 100.0 * ((f64)Anchor->TSCElapsedInclusive / (f64)TotalTSCElapsed);
        printf(", %.2f%% w/children", PercentWithChildren);
    }
    printf(")");
    
    if(Anchor->ProcessedByteCount)
    {
        f64 Megabyte = 1024.0f*1024.0f;
        f64 Gigabyte = Megabyte*1024.0f;
        
        f64 Seconds = (f64)Anchor->TSCElapsedInclusive / (f64)TimerFreq;
        f64 BytesPerSecond = (f64)Anchor->ProcessedByteCount / Seconds;
        f64 Megabytes = (f64)Anchor->ProcessedByteCount / (f64)Megabyte;
        f64 GigabytesPerSecond = BytesPerSecond / Gigabyte;
        
        printf("  %.3fmb at %.2fgb/s", Megabytes, GigabytesPerSecond);
    }
    
    printf("\n");
}

static void PrintAnchorData(u64 TotalCPUElapsed, u64 TimerFreq)
{
    for(u32 AnchorIndex = 0; AnchorIndex < ArrayCount(GlobalProfilerAnchors); ++AnchorIndex)
    {
        profile_anchor *Anchor = GlobalProfilerAnchors + AnchorIndex;
        if(Anchor->TSCElapsedInclusive)
        {
            PrintTimeElapsed(TotalCPUElapsed, TimerFreq, Anchor);
        }
    }
}

#else

#define TimeBandwidth(...)
#define PrintAnchorData(...)
#define ProfilerEndOfCompilationUnit

#endif

struct profiler
{
    u64 StartTSC;
    u64 EndTSC;
};
static profiler GlobalProfiler;

#define TimeBlock(Name) TimeBandwidth(Name, 0)
#define TimeFunction TimeBlock(__func__)

static u64 EstimateBlockTimerFreq(void)
{
    (void)&EstimateCPUTimerFreq; // NOTE(casey): This has to be voided here to prevent compilers from warning us that it is not used
    
	u64 MillisecondsToWait = 100;
	u64 OSFreq = GetOSTimerFreq();

	u64 BlockStart = READ_BLOCK_TIMER();
	u64 OSStart = ReadOSTimer();
	u64 OSEnd = 0;
	u64 OSElapsed = 0;
	u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
	while(OSElapsed < OSWaitTime)
	{
		OSEnd = ReadOSTimer();
		OSElapsed = OSEnd - OSStart;
	}
	
	u64 BlockEnd = READ_BLOCK_TIMER();
	u64 BlockElapsed = BlockEnd - BlockStart;
	
	u64 BlockFreq = 0;
	if(OSElapsed)
	{
		BlockFreq = OSFreq * BlockElapsed / OSElapsed;
	}
	
	return BlockFreq;
}

static void BeginProfile(void)
{
    GlobalProfiler.StartTSC = READ_BLOCK_TIMER();
}

static void EndAndPrintProfile()
{
    GlobalProfiler.EndTSC = READ_BLOCK_TIMER();
    u64 TimerFreq = EstimateBlockTimerFreq();
    
    u64 TotalTSCElapsed = GlobalProfiler.EndTSC - GlobalProfiler.StartTSC;
    
    if(TimerFreq)
    {
        printf("\nTotal time: %0.4fms (timer freq %llu)\n", 1000.0 * (f64)TotalTSCElapsed / (f64)TimerFreq, TimerFreq);
    }
    
    PrintAnchorData(TotalTSCElapsed, TimerFreq);
}
