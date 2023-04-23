/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 56
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

https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxBIAbKQADqgKhE4MHt6%2BekkpjgJBIeEsUTFc8XaYDmlCBEzEBBk%2Bfly2mPZ5DDV1BAVhkdFxtrX1jVktCsM9wX3FA2UAlLaoXsTI7BwA9ABUANQmAMwAIgfHRyfnZ5en1xc3V7cP9093L4%2Bvz2%2BfH9/vv19/P3%2BDw0AEETKCdpCIG55jsPAkAJ7EPDABAEHZmDRmfY7CIInYAWU8tHxACVUMgANaYAikHYASQYyAAdHSQbRaDtSSi0QouZglMQAG6YdDM8EgyF7CGQgAqCDwfIUqCoBAA7nVMDtFTsEsRUEK8Fh0DswBwmAoALSKs10tWEBDLdGGfGYVR6gV8kjalgJWh4UUSqUa4jEQwEBHMhkMHYMVA7TAiwQ7e0cnYEBBapheDMkPlRHaZ2gm/1MCL0Hb8Yg7F07dCsJjAAVByF1RXBYCV/UsdOZnZeJQ7FW9nXK1UhzDimXSyWQ5T0C1apRatEEBIKEAbDZoX056KMBCGNbMneV70sEha4JVlhMDotmdSoGAl8At/P9%2Bvj/fr%2B/z//n9Dh2LYNglCVtj2P9AOgqDYIAuCYPgpDEM%2BacAnpIRZXpUIAHEdgAVliB8UIQ0jkLIkjyKoyjAOA0DQXA3ZZXjD0k3RHcEjoaI%2BSobsdhFBwSDwAAvDtey1WhUFQdcdggNUFWQBAUzoTkBy1IR6QJY5p2CCZiC8KoBD5NUVyYEVY1QdEERpHYF2IEJ0EWHYEWWHZRBjNTxLPDlUHtIwdgUe0CEU5tpyIOtFTLCsBKIZFhLvNJtRjFyVjc1BfS4mIwOnAkhAANTcEAdktAB5LgHxwtxCuKkqcUtKg40tLwGH1DlLUk6S%2BXqxqCGITBMEtGKhOEzAHzcAIQVworSrqhrUCalriXaqSZO6%2BahrirU1stBRaASQbKlikTRunB92QIaJmEcEUSTpcLPNygqa3RUqzDpFK3MMftB3MfY9UbW9bJWiA4wAfQ2%2BYHwEBMRWIfEOoSdN43wBQoq1bQB3RDaRIS6Gq17O8gekqcQRA7KQQjBJRUwKgkvRZqUmABz%2B32MwDisBjQWCZ10HQCBuZ2EE6QFixIYYgB2DmwQ0ABOPqCBWGNJWsHYrH2KWTHF7TpZBLxWZ2IQO3oEQxDqCA9bMOFlkEOkLeAxkEhzMWwUl7KZbtoQfEgoCNHZt2q3N/XGSwVRvZ2X31ejEPILcK3moIdm9ksaxg7dZ3wRlzWNdljPPZ7awTmjR2E/wixU9UEx8NObPM61t23flxWDZ8P2Je1iU7bkRaOTME2DGIQPLY8ePbf1rYHadsDXc5939bzsOI5rgO7fLhfE9Xg5Y%2BHwR14YaOC6ODF05z6fpcz2X54PoCJ5Lsu97dSvq/ri%2Bvavouc0ru/98sHZyqr1uz6a3bjnGejd7LNxYAAoB5M7aHC8GIPuZs7bb1pCzS248GDF3TqfDOHsfDK0PovN2eCWAWDXurf2JBB5RzdOQshG99hb2tgnSOG9k6HzMMfWuNdc74KTmra%2BmCP6lzYWQ32/8KEz14aQ/hhcb6f1Eb/R%2BUCT7APPmfEhYc87Kx/nnNWNcwExjzlAuunNdb6wAIrwPQIggeyDmGj3QTfbBNcSEEJ9io2els9F0OIXPHwsdC5EJniQoCQTPHLyDvfUO4TWHRJjnHHecTv4CJ2JILhWdn4y20bIw%2B8iRHxJVuIp%2BUiX4yLfvkr%2BtCVZ/xKYAspgT2GCOLgowpP82YSJ4WUsJTT3630UfsZRkjAGmPqRo/x%2BdD45JVj4mZAT%2BEQLqRnQxECTHAPMZbKxTAbGiH7soXq1CUGOPtkIggLi/HeL4bE1xEyyHXIuRAwJhDPGhN8aUs%2BKCdigUPpITx8kuLQmYZaS0GScEgOyVc3plTimeOkXcqFpzP61NhQ03JzThEWA6Us8FecempMqYMzpWSb5orSbC0ZGcHmXymZCshszdHzLmSwbFcsaRN2McM6BZi7ayj6pgWx%2By7H6yOWgk5WCp43MuZMjxnKQEfOYV8wuvzZUy3%2BfQQF8dgWgq6V4nYspMC%2BncX0z%2BxSFmVORSq3B%2Bt9W%2BjuXkxFpcsVmodRYQlLKrWWxtYjQuXqdFkK9forJl9elepRTLElb9lU8IpXK5ZbLwEco1jGkEHBFi0E4PhXgfgOBaFIKgTgbgC4/2VCsNYSd9g8FIAQTQqbFiUgkBoDQ%2BhOCSF4CwEA%2BEm3ZtzfmjgvANxNurTm1NpA4CwBgIgFA6VOL0DIBQCAHFMogCFMgBICRwZcBlqDfY%2BxQaqFiNILAho1h5QDGqEqVNs2VpoLQC6xANwQAiDW0gERgh1ARJwStr7mBwxKhEbQh1P28B3GwQQJUGAkmfVgW8RhxDDtIPgPqVQRQbng26Soe4gPkEEG0Z9/oIhhjhh4LAz7ep4HbdwEdVADDAAUKezA57L1Yf4IIPu7ApAyEEIoFQ6h4O6H2PoQwxgi02HwxuSAixpIdFQ6VStFRDJ%2BAgK4UYzRSCBGmEUEo2RkipAECp7TuQ0i9E03MVo7RqiTH0%2BMNoh0LPdGM/0UoQxuhWec/UBzsxSiLBLasdjaaM1Zufb2nY%2B7JA7BYAoFd/FN3Mh3bJXAhBvS/S4PMXgQ6tDzEWJmbZAwIB1pADu5kMt9gaEkOLSQ%2BxJBcA0PscWZh8LSHTRwVtpBu28F7f2kAg6a2LDHZOxds7yCUAGwMYAXAzBvSPXgE9Z6L2MGY3QO9D6n3we/e%2BrDa3f3/sA5R0gIHGAEHA5B%2BD0GhNwdzYh2zKHn3oeQJh3b3NcPwfw4RhExH1i5rIxRngixqONjo7Npju2WPCF2ex6QIPuNqGfboN6BgjAoBE/oPAERxN5bzQkaTnAZrFTmgtVqtBlqdVxz1PlB1BKbX7TZhTLg96ufU4URzBndPpE8E0ZnHQPNaes%2BZgQXQRhs7GGZ2zfPJhc9MxMFzgvVOS/cxppnKWlilr882jgmbWtBc4CFgAHLES0B6djAGQMgX%2BZhmSWwgAl2K5aUtpZ61lzAOWYjo/rZV5ksRtf4U3VwardXJBmBlo1ltgX4MddsF1qtPXR0TogEgEbc7hvTqXcgeHY3YiNoQ4mabmB6OMfm8Dxb3FKArdzZtj9u2y9/oAw4LD%2B2wMQfLxdg1Z2Pu8Eu8hgUN3VAYYulhx7TXc0vffe90jyJvtUZowDhjc2r28BB2xiQEPZBQ947m3QLRU%2BI%2BTpYZHqP4CScx2kGTtUSfzWagTonq08fYxGqfnae1ydHRGlT3ninlPS4kG9BnMwtOVcSDpjoKzN6HIFncXGIP/eTDofnBoD/HnEXToMXeXTzEATENzGAzIGXRAxnZAzEbzZYXzCQfzNXEPHtLXVQXXfXMLFPITX%2BWIZkDQeg%2BLfAa3ZLVLSPYdTLUgbLLAZ3fLCtWLbXCrSrfCL3cWTdbXLgcWVXFrNrPNTgTrbrDgogswNtDtLtTXPtdgjLIg/YEg9reQrQ2tUgWGFIZwSQIAA%3D%3D%3D

*/