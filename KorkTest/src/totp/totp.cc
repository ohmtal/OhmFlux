#include "base32.h"
#include "hmac-sha1.h"
#include "platform/platform.h"
#include "console/console.h"
#include <platform/platformString.h>
#include <platform/platformProcess.h>
// #include "math/mMathFn.h"

namespace TOTP
{

	//----------------------------------------------------------------------
	// TODO test replacement:
	// void change_endianness64(U64* pi){
	// 	*pi = __builtin_bswap64(*pi);
	// }
	void change_endianness64(U64* pi){
		U64 i = *pi;
		// swap 32 bit words
		i = (i << 32) | (i >> 32);
		// swap 16 bit words XXTH ULL SUFFIX IS IMPORTANT TO COMPILE IT ON unix
		U64 m = 0xffff0000ffff0000ULL;
		i = ((i & m) >> 16) | ((i & ~m) << 16);
		// swap bytes XXTH ULL SUFFIX IS IMPORTANT TO COMPILE IT ON unix
		m = 0xff00ff00ff00ff00ULL;
		i = ((i & m) >> 8) | ((i & ~m) << 8);
		*pi = i;
	}
	//----------------------------------------------------------------------
	U32 HOTP(const U8* key, int keyLength, U64 counter){
		// we need to transform C into a 8-byte array
		// we naturally need to determine the endianness of the system

	#ifndef TORQUE_BIG_ENDIAN
		change_endianness64(&counter);
	#endif

		U8 HS[20];
		hmac_sha1(key, keyLength, (const U8*)&counter, 8, HS);
		int offset = HS[19] & 0xf;

		// we now want the 31 lsb's of the 32 bit dword starting at byte "offset"
		// of HS
		U8* offsetHS = HS + offset;
		U32 p;
	#ifdef TORQUE_BIG_ENDIAN
		memcpy(&p, offsetHS, 4);
	#else
		p = *(offsetHS+3);
		p |= *(offsetHS+2) << 8;
		p |= *(offsetHS+1) << 16;
		p |= *(offsetHS)   << 24;
	#endif

		p &= 0x7fffffff;
		// now get only the least significant 6 decimal digits
		p %= 1000000;

		return p;
	}

	//----------------------------------------------------------------------
	U32 getCode(const char * secret, U64 timeslice=0)
	{
		if (timeslice == 0)
			timeslice = Platform::getTime() / 30;

		U8 key[20];
		base32_decode((const U8*)secret, key, 20);

		int l=0;
		while (l<20 && key[l] != 0)
			l++;

		U32 result =  HOTP(key, l, Platform::getTime() / 30);

		return result;
	}
	//----------------------------------------------------------------------
	bool validate(const char * secret, U32 code, S32 discrepancy=1, U64 timeslice=0)
	{
		if (timeslice == 0)
			timeslice = Platform::getTime() / 30;

        for (int i = -discrepancy; i <= discrepancy; i++) {
            if ( code == TOTP::getCode( secret, timeslice + i) )
			{
                return true;
            }
        }
        return false;

	}
} //namespace
//===========================================================================

/*
	TEST: $skey =  "G26FPPD2YZZ2WHDG";echo (getTotpCode($skey));echo (getTotpValidate($skey,getTotpCode($skey)));

 */

ConsoleFunctionGroupBegin(TwoFactorAuth, "Two Factor Authentication functions");


ConsoleFunction(getTotpCode, S32, 2, 2, "get TOTP by secret ")
{
    const char* secret = argv[1]; 
	if (dStrlen(secret) != 16)
	{
		Con::errorf("Invalid Keylen must be 16 but is %d", dStrlen(secret));
		return 0;
	}

	S32 result = TOTP::getCode(secret);

	return result;
	
}
//===========================================================================
ConsoleFunction(getTotpValidate, bool, 3, 3, "vaidate a TOTP code by secret")
{
    const char* secret = argv[1]; 
	if (dStrlen(secret) != 16)
	{
		Con::errorf("Invalid Keylen must be 16 but is %d", dStrlen(secret));
		return false;
	}
	U32 code = dAtoi( argv[2] );

	return TOTP::validate(secret,code,3);
}
ConsoleFunctionGroupEnd(TwoFactorAuth);
