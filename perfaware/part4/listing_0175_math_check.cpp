/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 175
   ======================================================================== */

typedef double math_func(double);

struct math_test_result
{
    f64 TotalDiff;
    f64 MaxDiff;
    u32 DiffCount;
    
    f64 InputValueAtMaxDiff;
    f64 OutputValueAtMaxDiff;
    f64 ExpectedValueAtMaxDiff;
    
    char Label[64];
};

struct math_tester
{
    math_test_result Results[256];
    math_test_result ErrorResult;
    
    u32 ResultCount;
    u32 ProgressResultCount;

    b32 Testing;
    u32 StepIndex;
    u32 ResultOffset;
    
    f64 InputValue;
};

struct reference_answer
{
    f64 Input;
    f64 Output;
};

/* NOTE(casey): 64-bit floating point can't really get past 17 decimal
   significant figures, so these constants are overkill. However, I've left
   them that way so that anyone who wants to copy the values will have
   extra precision in case they are using something with support for
   greater-than-64-bit-float precision.
*/

#define Pi64 3.14159265358979323846264338327950288419716939937510582097494459230781640628

reference_answer RefTableSinX[] =
{
    {-3.141592653589793238,  0},
    { 3.141592653589793238,  0},
    {                    0,  0},
    {-1.980178972189184705, -0.917366731335869517815085353888790023576601790034023255791022208329207},
    { 3.054525069643740487,  0.086957619164997496582615743019798967746841890514906184002599101538091},
    { 1.086970485805371700,  0.885221741900538559272075429983319559526944117797203913074755462845114},
    { 2.641530792209640488,  0.479479826155304798209951367606254396982233350750339094287968202329608},
    {-1.602757367730838656, -0.999489289407951573478453844716195585524302110967832803171811344497130},
    { 0.120932552417232664,  0.120638001193843219705110020728492991685797262599607933668881821068756},
    { 0.895519891829380299,  0.780534177837081816112569020824721291854568093695852302589388519326879},
    {-2.949439680459583712, -0.190972684367099218768029197195911262538402925084493397140256672313270},
    { 0.503627961923384326,  0.482606212617209464263999008171573117899524228441095779842774224225518},
    {-2.528819019434219939, -0.575138657376855041732163890801836369778531074662663197297007109447792},
    {-1.751958649112110855, -0.983634938183362555493150892239262244107789376588795611670603940034962},
    { 1.947592546574743899,  0.929848215207483943032722155935456092132577372068404175480345560354313},
    { 1.894204562555628080,  0.948157789969379084909086224345238571337294063982883486226633841066968},
    { 0.507425356398133243,  0.485928625291467153109494460243839877015541666373516405963355955426745},
    {-1.049757340382687865, -0.867302459656594852549820907450201155086718237517631210752207676190523},
    {-1.105406126779538090, -0.893646522982279028728058617603741066082503194431138593753976426322044},
};

reference_answer RefTableCosX[] =
{
    {-1.570796326794896619, 0},
    { 1.570796326794896619, 0},
    {                    0, 1},
    { 0.208072336856456319, 0.978430937816387477248102570292246364378999020468068458971097843341180},
    { 0.675088834008283012, 0.780651435918642620809400723560211477763738877205387021374281911110632},
    {-0.649360937006398853, 0.796470388197960097328844883016591560580109899910365476752619186695158},
    { 0.390133363762819041, 0.924858348274675427445809760699324903800219286567403122024940342549265},
    {-0.188138252300037112, 0.982354140663335481830359331970627174904756614773823639001937622713535},
    { 0.442986346936877662, 0.903475627687927634454514957318468496493340523664390142064630365402663},
    { 0.112398563843224331, 0.993689928778741432913410074191544830779297929699686516751849675294918},
    { 0.524929410773476324, 0.865359319705350534313008389757968472863054384632312313657833212614387},
    { 1.320125892438205106, 0.248053495780160407692468071070965093565915026210044809604624709318974},
    { 0.310157986917311224, 0.952285362727283883185505025556855374044760621624723325610556183149260},
    {-0.512059907572585904, 0.871737056726822070557404540819763574221828980211554421761791178391164},
    {-0.019024769422439469, 0.999819034532558315713420675129207239252363966391577405851241620798968},
    {-1.188535161113987382, 0.373019383992923540083712639985839360268200818757685607903578873561327},
    {-1.287695642837729970, 0.279334244494834992843483526739369934553169740835814349988860879427578},
    { 0.140794903151348016, 0.990104760118359770996576344513837951249406493555781987399213139735929},
    {-1.561169615584349746, 0.009626562520955350208912506462712435006323009077835337480272238678485},
};

reference_answer RefTableArcSinX[] =
{
    {                   0, 0},
    {                   1, 1.57079632679489661923132169163975144209858469968755291048747229615390820},
    {0.307293958335019157, 0.31234808998780550368494688635355987269549952968782884596351104311416574},
    {0.370147297943510867, 0.37916757565850061059847811520129805736148573000635768089845144976637260},
    {0.248485618794449969, 0.25111652400209270955704411677393828084368931052035998410889672644140320},
    {0.067035381464994367, 0.06708568988673765274538799798675257070917655156857971953653616092879825},
    {0.737810179297302415, 0.82982044114707095465067261186321307217465919578892529567717599056824297},
    {0.155809553609185275, 0.15644696330495471023083842817817472720324984855729228626226143616346565},
    {0.301281112549193486, 0.30603590899714265909301842483547697212401127459838263448098204594194647},
    {0.570105060602097091, 0.60663372710649292015719932738392281691348150593229457170121638754674419},
    {0.203965590848579431, 0.20540696935013139767036547522178516300483232354126681359761689317602328},
    {0.777691050613692703, 0.89098453759088223212250762906500848111117240999298288157439297740035855},
    {0.808304252291169090, 0.94126621673334667922259356094219879964690544180985435299259035461021061},
    {0.197167432680066534, 0.19846779235970994841836150941560064266235330591407265580362130981617815},
    {0.470300390494591658, 0.48963113051612299432953986768851488755724963531697523330619213630803819},
    {0.986877470739855767, 1.40861537694986024126474446139211020550408390090131186822026815270909344},
    {0.752587876225027208, 0.85198330750493896359177636813964949991957114078161931569484825917412546},
    {0.423156465269428794, 0.43692623716706648420399396679546505269710883382357663318397328001932281},
};

reference_answer RefTableSqrtX[] =
{
    {                   0, 0},
    {                   1, 1},
    {0.748214140708608144, 0.86499372293017718984946019959885979208177693068271877892114853320073872},
    {0.295610455166457786, 0.54370070366559006873032299574529326171064507194665174162115258405384736},
    {0.074127153743553706, 0.27226302309265888631416233629900440004307122751842176968327763432225451},
    {0.001595712034403279, 0.03994636447041556496492642282447894689435970838650463355066810309142590},
    {0.356969093521351255, 0.59746890590335431697817895476659808917220187339255989422386088479560546},
    {0.853139722918425658, 0.92365563004748997090925797889795671515205333230235590320249548049293593},
    {0.927537194669138865, 0.96308732452936938495089571663634022846289760666137383585821049973342001},
    {0.127922274434920935, 0.35766223512543358118906014316843859318873347702001499289862614031656062},
    {0.824461576022315179, 0.90799866520954488625602271157741762985557399267299227097349184715016139},
    {0.453623607394663508, 0.67351585534021655131210882794533116146327550886450346196181629722363426},
    {0.772190216875679347, 0.87874354442902130887003177603740369222489828657022283415216647106370672},
    {0.564153768253548460, 0.75110170300269487539691594711148930594057884957418089440646797040443379},
    {0.956991909294620080, 0.97825963286574391901846741371136601124675369605279184664817479599190955},
    {0.505690656254461635, 0.71111929818734467740805282705664431333275750249760190797796391338997076},
    {0.129707999324823742, 0.36014996782565973457146811665176803598744277714218474293903671269080010},
    {0.051231188245869981, 0.22634307642574353990563961017436878164407501492708325367084901372998301},
};

inline f64 GetAvgDiff(math_test_result From)
{
    f64 Result = (From.DiffCount) ? (From.TotalDiff / (f64)From.DiffCount) : 0;
    return Result;
}

inline void PrintDecimalBars(void)
{
    printf("   ________________             ________________\n");
}
    
inline void PrintResult(math_test_result Result)
{
    printf("%+.24f (%+.24f) at %+.24f [%s] \n", Result.MaxDiff, GetAvgDiff(Result), Result.InputValueAtMaxDiff, Result.Label);
}

inline b32 PrecisionTest(math_tester *Tester, f64 MinInputValue, f64 MaxInputValue, u32 StepCount = 100000000)
{
    if(Tester->Testing)
    {
        ++Tester->StepIndex;
    }
    else
    {
        // NOTE(casey): This is a new test
        Tester->Testing = true;
        Tester->StepIndex = 0;
    }

    if(Tester->StepIndex < StepCount)
    {
        Tester->ResultOffset = 0;
        
        f64 tStep = (f64)Tester->StepIndex / (f64)(StepCount - 1);
        Tester->InputValue = (1.0 - tStep)*MinInputValue + tStep*MaxInputValue;
    }
    else
    {
        Tester->ResultCount += Tester->ResultOffset;
        if(Tester->ResultCount > ArrayCount(Tester->Results))
        {
            Tester->ResultCount = ArrayCount(Tester->Results);
            fprintf(stderr, "Out of room to store math test results.\n");
        }
        
        if(Tester->ProgressResultCount < Tester->ResultCount)
        {
            PrintDecimalBars();
            while(Tester->ProgressResultCount < Tester->ResultCount)
            {
                PrintResult(Tester->Results[Tester->ProgressResultCount++]);
            }
        }
        
        Tester->Testing = false;
    }
    
    b32 Result = Tester->Testing;
    return Result;
}

inline void TestResult(math_tester *Tester, f64 Expected, f64 Output, char const *Format, ...)
{
    u32 ResultIndex = Tester->ResultCount + Tester->ResultOffset;
    math_test_result *Result = &Tester->ErrorResult;
    if(ResultIndex < ArrayCount(Tester->Results))
    {
        Result = Tester->Results + ResultIndex;
    }
    
    if(Tester->StepIndex == 0)
    {
        *Result = {};
        va_list ArgList;
        va_start(ArgList, Format);
        vsnprintf(Result->Label, sizeof(Result->Label), Format, ArgList);
        va_end(ArgList);
    }
    
    f64 Diff = fabs(Expected - Output);
    Result->TotalDiff += Diff;
    ++Result->DiffCount;
    
    if(Result->MaxDiff < Diff)
    {
        Result->MaxDiff = Diff;
        Result->InputValueAtMaxDiff = Tester->InputValue;
        Result->OutputValueAtMaxDiff = Output;
        Result->ExpectedValueAtMaxDiff = Expected;
    }
    
    ++Tester->ResultOffset;
}
    
inline int MathCmpGT(void *Context, const void *AIndex, const void *BIndex)
{
    math_tester *Tester = (math_tester *)Context;
    
    math_test_result *A = Tester->Results + (*(u32 *)AIndex);
    math_test_result *B = Tester->Results + (*(u32 *)BIndex);
    
    int Result = 0;
    if(A->MaxDiff > B->MaxDiff)
    {
        Result = 1;
    }
    else if(A->MaxDiff < B->MaxDiff)
    {
        Result = -1;
    }
    else if(A->TotalDiff > B->TotalDiff)
    {
        Result = 1;
    }
    else if(A->TotalDiff < B->TotalDiff)
    {
        Result = -1;
    }
    
    return Result;
}

inline void PrintResults(math_tester *Tester)
{
    if(Tester->ResultCount)
    {
        printf("\nSorted by maximum error:\n");
        
        PrintDecimalBars();

        u32 Ranking[ArrayCount(Tester->Results)];
        for(u32 ResultIndex = 0; ResultIndex < Tester->ResultCount; ++ResultIndex)
        {
            Ranking[ResultIndex] = ResultIndex;
        }
        
        qsort_s(Ranking, Tester->ResultCount, sizeof(Ranking[0]), MathCmpGT, Tester);
        
        for(u32 ResultIndex = 0; ResultIndex < Tester->ResultCount; ++ResultIndex)
        {
            math_test_result Result = Tester->Results[Ranking[ResultIndex]];
            
            printf("%+.24f (%+.24f) [%s", Result.MaxDiff, GetAvgDiff(Result), Result.Label);
            while((ResultIndex + 1) < Tester->ResultCount)
            {
                math_test_result NextResult = Tester->Results[Ranking[ResultIndex + 1]];
                if((NextResult.MaxDiff == Result.MaxDiff) &&
                   (NextResult.TotalDiff == Result.TotalDiff))
                {
                    printf(", %s", NextResult.Label);
                    ++ResultIndex;
                }
                else
                {
                    break;
                }
            }
            printf("]\n");
        }
    }
}

inline void CheckHardCodedReference(char const *Label, math_func *Func, u32 RefCount, reference_answer *Refs)
{
    printf("%s:\n", Label);
    for(u32 RefIndex = 0; RefIndex < RefCount; ++RefIndex)
    {
        reference_answer Ref = Refs[RefIndex];

        printf("  f(%+.24f) = %+.24f [reference]\n", Ref.Input, Ref.Output);
        f64 Output = Func(Ref.Input);
        printf("                                 = %+.24f (%+.24f) [%s]\n", Output, Ref.Output - Output, Label);
    }
    printf("\n");
}
