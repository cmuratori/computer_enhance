/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 81
   ======================================================================== */

#include "listing_0074_platform_metrics.cpp"

struct profile_anchor
{
    u64 TSCElapsed;
    u64 TSCElapsedChildren;
    u64 HitCount;
    char const *Label;
};

struct profiler
{
    profile_anchor Anchors[4096];
    
    u64 StartTSC;
    u64 EndTSC;
};
static profiler GlobalProfiler;
static u32 GlobalProfilerParent;

struct profile_block
{
    profile_block(char const *Label_, u32 AnchorIndex_)
    {
        ParentIndex = GlobalProfilerParent;
        
        AnchorIndex = AnchorIndex_;
        Label = Label_;
        
        GlobalProfilerParent = AnchorIndex;
        StartTSC = ReadCPUTimer();
    }
    
    ~profile_block(void)
    {
        u64 Elapsed = ReadCPUTimer() - StartTSC;
        GlobalProfilerParent = ParentIndex;
        
        profile_anchor *Parent = GlobalProfiler.Anchors + ParentIndex;
        profile_anchor *Anchor = GlobalProfiler.Anchors + AnchorIndex;
        
        Parent->TSCElapsedChildren += Elapsed;
        Anchor->TSCElapsed += Elapsed;
        ++Anchor->HitCount;
                
        /* NOTE(casey): This write happens every time solely because there is no
           straightforward way in C++ to have the same ease-of-use. In a better programming
           language, it would be simple to have the anchor points gathered and labeled at compile
           time, and this repetative write would be eliminated. */
        Anchor->Label = Label;
    }

    char const *Label;
    u64 StartTSC;
    u32 ParentIndex;
    u32 AnchorIndex;
};

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBlock(Name) profile_block NameConcat(Block, __LINE__)(Name, __COUNTER__ + 1);
#define TimeFunction TimeBlock(__func__)

static void PrintTimeElapsed(u64 TotalTSCElapsed, profile_anchor *Anchor)
{
    u64 Elapsed = Anchor->TSCElapsed - Anchor->TSCElapsedChildren;
    f64 Percent = 100.0 * ((f64)Elapsed / (f64)TotalTSCElapsed);
    printf("  %s[%llu]: %llu (%.2f%%", Anchor->Label, Anchor->HitCount, Elapsed, Percent);
    if(Anchor->TSCElapsedChildren)
    {
        f64 PercentWithChildren = 100.0 * ((f64)Anchor->TSCElapsed / (f64)TotalTSCElapsed);
        printf(", %.2f%% w/children", PercentWithChildren);
    }
    printf(")\n");
}

static void BeginProfile(void)
{
    GlobalProfiler.StartTSC = ReadCPUTimer();
}

static void EndAndPrintProfile()
{
    GlobalProfiler.EndTSC = ReadCPUTimer();
    u64 CPUFreq = EstimateCPUTimerFreq();
    
    u64 TotalCPUElapsed = GlobalProfiler.EndTSC - GlobalProfiler.StartTSC;
    
    if(CPUFreq)
    {
        printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (f64)TotalCPUElapsed / (f64)CPUFreq, CPUFreq);
    }
    
    for(u32 AnchorIndex = 0; AnchorIndex < ArrayCount(GlobalProfiler.Anchors); ++AnchorIndex)
    {
        profile_anchor *Anchor = GlobalProfiler.Anchors + AnchorIndex;
        if(Anchor->TSCElapsed)
        {
            PrintTimeElapsed(TotalCPUElapsed, Anchor);
        }
    }
}
