/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 58
   ======================================================================== */

/* To prevent compilers from vectorizing the loops (which will use SIMD
   instructions we have not yet learned), you can use the following switches
   to disable vectorization in your compiler:

   MSVC: -O1
   GCC: -O3 -fno-unroll-loops -fno-tree-vectorize
   CLANG: -O3 -fno-unroll-loops -fno-vectorize -fno-slp-vectorize
   
   Alternatively, to use MSVC at -O2, you can use #pragma loop(no_vector)
   on every loop to disable just vectorization for that loop.
*/

typedef int unsigned u32;

int add(int A, int B)
{
	return A + B;
}

u32 SingleScalar(u32 Count, u32 *Input)
{
	u32 Sum = 0;
	for(u32 Index = 0; Index < Count; ++Index)
	{
		Sum += Input[Index];
	}
	
	return Sum;
}

u32 Unroll2Scalar(u32 Count, u32 *Input)
{
	u32 Sum = 0;
	for(u32 Index = 0; Index < Count; Index += 2)
	{
		Sum += Input[Index];
		Sum += Input[Index + 1];
	}
	
	return Sum;
}

u32 DualScalar(u32 Count, u32 *Input)
{
	u32 SumA = 0;
	u32 SumB = 0;
	for(u32 Index = 0; Index < Count; Index += 2)
	{
		SumA += Input[Index + 0];
		SumB += Input[Index + 1];
	}
	
	u32 Sum = SumA + SumB;
	return Sum;
}

u32 QuadScalar(u32 Count, u32 *Input)
{
	u32 SumA = 0;
	u32 SumB = 0;
	u32 SumC = 0;
	u32 SumD = 0;
	for(u32 Index = 0; Index < Count; Index += 4)
	{
		SumA += Input[Index + 0];
		SumB += Input[Index + 1];
		SumC += Input[Index + 2];
		SumD += Input[Index + 3];
	}
	
	u32 Sum = SumA + SumB + SumC + SumD;
	return Sum;
}

u32 QuadScalarPtr(u32 Count, u32 *Input)
{
	u32 SumA = 0;
	u32 SumB = 0;
	u32 SumC = 0;
	u32 SumD = 0;
	
	Count /= 4;
	while(Count--)
	{
		SumA += Input[0];
		SumB += Input[1];
		SumC += Input[2];
		SumD += Input[3];
		Input += 4;
	}
	
	u32 Sum = SumA + SumB + SumC + SumD;
	return Sum;
}

u32 TreeScalarPtr(u32 Count, u32 *Input)
{
	u32 Sum = 0;
	
	Count /= 4;
	while(Count--)
	{
		u32 TempA = Input[0] + Input[1];
		u32 TempB = Input[2] + Input[3];
		u32 Temp = TempA + TempB;
		Sum += Temp;
		Input += 4;
	}
	
	return Sum;
}

/* To open this listing on godbolt with compilers already configured, use the following link:

https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxBJmpAAOqAqETgwe3r56icmOAkEh4SxRMVxxdpgOqUIETMQE6T5%2BXLaY9rkM1bUE%2BWGR0bG2NXUNmc0KQ93BvUX9pQCUtqhexMjsHAD0AFQA1CYAzAAi%2B0eHx2enFydX59eXN/d3j7fPDy9Prx/vX28/n7/ff3uGgAgiYQdsIRA3HNth54gBPYh4YAIAjbMwaMx7bYReHbACynloeIASqhkABrTAEUjbACSDGQADpacDaLRtiTkaiFJzMEpiAA3TDoJlg4EQ3bgiEAFQQeF5ClQVAIAHdaphtgrtvFiKhBXgsOhtmAOEwFABaBWm2mqwgIJZowx4zCqXX83kkLUseK0PAi8WS9XEYiGAjwpn0hjbBiobaYYWCbZ29nbAgITVMLzpki8qLbDO0Y1%2BpgRejbfjEbbO7boVhMYD8wMQ2oK4LACt6lhpjPbLxKbbKnvapUq4OYMXSqUSiHKejmzVKTWogjxBQgdbrNA%2B7PRRgIQyrJnbitelgkTXBSssJjtZvTyWAgHP/6vp9vl/vr%2Bfn8fv/fg5tk2dZxXFLZdl/ACoMgmD/1g6C4MQhCPinAI6SEGU6VCABxbYAFYAA572Q%2BDSKQsiSPIqjKIAoCQJBMCdhlON3UTNFt3iOhol5Kgu22YUHBIPAAC92x7TVaFQVA122CBVXlZAEGTOgOX7TUhDpfEjinYJxmILxKgEXlVWXJhhRjVA0XhaltnnYgQnQBZtnhJZtlEaM1PE092VQO0jG2BQ7QIRSmynIhawVUtywEogkWE29Ui1aMXOWNzUB9LiYlAqd8SEAA1NwQG2C0AHkuHvbC3EK4qSuxC0qFjC0vAYPV2QtSTpN5erGoIYhMEwC0YqE4TMHvNwAmBHCitKuqGtQJqWqJdqpJk7r5qGuLNTWi0FFoeJBoqWKRNGqd7zZAhomYRxhWJWlws83KCurNFSriZzXPcvsB3MPZdQbG9bJWiBYwAfQ2uZ7wEeNhWIPEOviNM43wBQos1bR%2BzRDaRISqHKx7W9AekydgWA7LgXDeIRUwKgkrRZrkmABy%2Bz2Mx9isBiQWCJ10HQCAue2YFaX5iwIYYgB2dnQQ0ABOPqCGWaMJWsbYrD2SWTDF7SpeBLwWe2IR23oEQxFqCBdbMWElkEWlzaAhl4mzUXQQl7LpdtoQfAgwCNDZ13KzNvWGSwVQve2H21ajYOILcS3moINndksawg9dJ2wWljX1Zl9OPe7axjijB347wiwU9UEw8JOLOM8113XblhX9Z8X3xa18VbbkRb2TMY2DGIAOLY8OObb1zZ7cd0CXY5t29dz0Pw%2Br/3bbL%2BeE5X/YY6HwQ14YKP88OdE0%2BzqepYzmW5/3wDx%2BL0vd9dCuq7r8/Pcvwvswr2%2B98sbZysrlvT41m3bO08G72Sbiwf%2BgCya2wOF4MQvdTa2y3jSZmFsx4MCLmnE%2B6d3Y%2BCVgfBertcEsAsKvNWfsSAD0jq6MhpD157E3lbeOEd15JwPmYI%2BNdq45zwYnVWV8MHvxLqw0hPs/7kOnjwkhfCC7Xw/iIn%2BD9IHHyAWfU%2BxDQ65yVt/XOqtq6gOjLnSBtcOY6z1gARTgegBB/ckFMJHmg6%2BWDq7EPwd7ZRM8La6NoUQ2ePgY4F0IdPYhgFAkeKXoHO%2BIcwksKidHWO29Ylf34dsSQnDM5P2llomRB85HCLicrMRj9JHP2ka/PJn8aHK1/sUgBpSAlsIEUXeRBTv6s3Edw0poTGlvxvgovYSiJEAJMXU9Rfi84H2ycrbx0z/F8PAbU9OBjwHGKAWYi2limDWNEH3ZQvUqHIIcXbQRBBnG%2BK8bwmJLjxmkKuec8BASCEeJCT4kpp9kHbBAgfSQHj5JcShEwi0Fp0nYOAVky5PSKlFI8VI25kKTkfxqTC%2BpOSmlCIsO0xZYLc7dJSRUgZHTMnX1RakmFIz073IvpMiFpCZk6LmbMlgWLZbUkbkYoZUDTG2xlH1TANi9m2L1oc1BxzMGT2uRciZ7iOXAPeUwz5Bcfkyuln8%2BgAK45ApBZ0zx2wZSYB9G43pH8inzIqUi5VOC9Z6p9Lc3JCKS6YtNfaiwBLmWWottahGBdPXaNIZ6vRmSL49M9ci6WxLX5Ku4eS2VSzWVgPZeraNwIOALFoJwPCvA/AcC0KQVAnA3D52/kqZYqxE57B4KQAgmgU0LApBIDQGh9CcEkLwFgIA8KNqzTmvNHBeDrkbVW7NKbSBwFgDARAKB0qcXoGQCgEAOKZRAIKZA8R4hgy4NLEGew9gg1UAANmkFgA0qw8r%2BlVCVSmWaK00FoBdYg64IARGraQCIwRajwk4BW19zBYYlQiNoQ6n7eDbjYIIEqDBiTPqwDeIw4gh2kHwH1Sowp1zwddBUXcQHyCCFaM%2Bv0ERQyww8FgZ9vU8Btu4MOqgBhgAKFPZgc9l6sP8EEL3dgUgZCCEUCodQ8HdB7H0IYYwhabD4fXJABY0l2iodKhW8ohk/AQFcCMJopBAhTEKMULISQUgCBU9pnIqQeiadmC0NoVQJj6bGK0Q6FmujGb6CUQYXQrPObqA5mYJQFjFpWOx1N6bM3Pp7dsfdkhtgsAUMu/iG6mTbtkrgQgXofpcDmLwQdWg5gLAzFs/oEBa0gG3UyaWewNCSDFpIPYkguAaD2GLMweFpBpo4C20gXbeA9r7SAAd1aFijonQumd5BKADf6MAUocQj14BPWei9jBmN0DvQ%2Bp98Hv3vqw6t39/7AOUdICBxgBBwOQfg9BoTcGc2Idsyh596HkCYZ21zXD8H8OEfhMRtYOayMUZ4AsajDY6MzaYztljwgdnsekMD7jahn26DiAYIwKARP6DwBEcTeXc3xGk5wGaxU5oLVarQZanUcc9V5QdQSm0%2B02YUy4Xern1MFEcwZ3TaRPCNCZ%2B0DzWnrPmYEJ0YYrPRhmds7ziYnPTPjBcwL1TEv3MacZylxYJa/NNo4Bm1rQXOAhYInui0B7tjAGQMgH%2BZgmQWwgAl2KZaUtpZ61lzAOWYho7rZVpke6CJ4Q3VwardXJBmGlo15tgX4MddsF1ytPWR3jogEgEbs7htTsXcgOHY290NoQwmKbmB6OMbm0Dhb3FKDLZzRtj9O2S9/oAw4LDe2wMQdL%2Bd/Vp33u8Au8h/k13VAYYulhh7TWc3PffW90jSIvtUZo/9hjs2r28GB2xiQ4PZCQ94zm3QzRk8I6TpYJHKP4CSYx6kGTtVifzWavjwnq1cdYxGsfnae0ydHRGpTnninlNS/8LvMXTnsjM9c9/jncvPM9B5N2g%2Bd6g39udhcOhRcACuc3MwCMhpdoCGdACFcfNVgJB/NVcg9u1NdVBtddcwsk8hMf490mQNAyD4t8BLdktUtw8h1MtSBsssBHd8ty1YsCIKtKs8IPcxYN0CIuAxYVcWs2tc1OBOtut6DMCzBW121O0Nde06CMtMC9hsD2sxDFCa1SAYZkhnBJAgA%3D%3D

*/