#include "stdafx.h"
#include "common.h"
#include <ctype.h>

static UINT g_uRandomSeed = 42;

void g_RandomSeed(UINT nSeed)
{
	g_uRandomSeed = nSeed;
}

UINT g_Random(UINT nMax)
{
	int64 f = g_uRandomSeed * 0x08088405 + 1;
	g_uRandomSeed = f;
	int64 t = f * nMax;
	t = t >> 32;
	return (unsigned int)t;
}

UINT g_GetRandomSeed()
{
	return g_uRandomSeed;
}

char g_szAddivOp[] =
{
	'\x09',
	'*',
	'/',
	'+',
	'-'
};

void ClearChar( char* szAction, char cOne, char cTwo )
{
	int nPos = 0;
	int nScanPos = 0;

	while( szAction[nScanPos] )
	{
		szAction[nPos] = szAction[nScanPos];

		if( szAction[nPos] != cOne &&
			szAction[nPos] != cTwo )
			nPos++;

		nScanPos++;
	}

	szAction[nPos] = 0;
}

int	FindDelimter( char* szAction, char cOne, char cTwo )
{
	int nPos = 0;

	while( szAction[nPos] )
	{
		if( szAction[nPos] == cOne ||
			szAction[nPos] == cTwo )
			return nPos;

		nPos++;
	}

	return 0;
}

void ConvLowerCase( char* szAction )
{
	int nLoopCount = 0;

	while( szAction[nLoopCount] )
	{
		szAction[nLoopCount] = tolower( szAction[nLoopCount] );
		nLoopCount++;
	}
}

int	CheckAct( char* szAction )
{
	int nRet	= TRUE;
	int nPos	= 0;
	int nClose	= 0;

	while( szAction[nPos] )
	{
		if( szAction[nPos] < '!' ||
			szAction[nPos] > '~' )
		{
			nRet = 0;
			break;
		}

		if( szAction[nPos] == '(' )
		{
			nClose++;
			if( szAction[nPos+1] == ',' )
			{
				nRet = 0;
				break;
			}
		}

		if( szAction[nPos] == ')' )
			nClose--;

		if( szAction[nPos] == ',' && szAction[nPos+1] == ')' )
		{
			nRet = 0;
			break;
		}

		nPos++;
	}

	if( nPos && szAction[nPos-1] != ')' )
		nRet = 0;

	if( nRet && nClose )
		nRet = FALSE;

	return nRet;
}

char* FindCloseComma( char* szAction )
{
	static char szInBuff[100]  = {0};
	static int nPos	= 0;
	static int nPrevPos = 0;
	int nClose	= 0;

	if( szAction )
	{
		strcpy( szInBuff, szAction );
		nPos		= 0;
		nPrevPos	= 0;
	}

	nPrevPos = nPos;

	while( szInBuff[nPos] )
	{
		if( szInBuff[nPos] == '(' )
			nClose++;

		if( szInBuff[nPos] == ')' )
			nClose--;

		if( szInBuff[nPos] == ',' &&
			!nClose )
		{
			szInBuff[nPos] = 0;
			nPos++;
			return szInBuff + nPrevPos;
		}

		nPos++;
	}

	if( szInBuff[nPrevPos] )
		return szInBuff + nPrevPos;
	else
		return 0;
}

char* FindAddiOp( char* szAction, int& nAddiOp )
{
	static char szInBuff[100]  = {0};
	static int nPos	= 0;
	static int nPrevPos = 0;
	static int nPrevAddiOp = 0;
	int nClose	= 0;

	if( szAction )
	{
		strcpy( szInBuff, szAction );
		nPos		= 0;
		nPrevPos	= 0;
		nPrevAddiOp	= 0;
	}

	nPrevPos = nPos;

	while( szInBuff[nPos] )
	{
		int nLoopCount = 0;
		while( g_szAddivOp[nLoopCount] )
		{
			if( szInBuff[nPos] == 
				g_szAddivOp[nLoopCount])
			{
				nAddiOp = nPrevAddiOp;
				nPrevAddiOp = nLoopCount;
				szInBuff[nPos] = 0;
				nPos++;
				return szInBuff + nPrevPos;
			}

			nLoopCount++;
		}

		nPos++;
	}

	nAddiOp = nPrevAddiOp;

	if( szInBuff[nPrevPos] )
		return szInBuff + nPrevPos;
	else
		return 0;
}

typedef union
{
	int     i;          // as integer
	float   f;          // as float
	struct              // as bit fields
	{
		unsigned int    sign:1;
		unsigned int    biasedexponent:8;
		unsigned int    significand;
	}bits;
}INTORFLOAT;

#define SQRTTABLESIZE       256             /* Note: code below assumes this is 256. */
const unsigned int sqrttable[SQRTTABLESIZE] = 
{
	531980127, 532026288, 532072271, 532118079, 532163712, 532209174, 532254465, 532299589,
	532344546, 532389339, 532433970, 532478440, 532522750, 532566903, 532610900, 532654744,
	532698434, 532741974, 532785365, 532828607, 532871704, 532914655, 532957463, 533000129,
	533042654, 533085041, 533127289, 533169401, 533211378, 533253220, 533294931, 533336509,
	533377958, 533419278, 533460470, 533501535, 533542475, 533583291, 533623984, 533664554,
	533705004, 533745334, 533785545, 533825638, 533865615, 533905476, 533945222, 533984855,
	534024374, 534063782, 534103079, 534142267, 534181345, 534220315, 534259178, 534297934,
	534336585, 534375132, 534413574, 534451914, 534490152, 534528288, 534566324, 534604260,
	534642098, 534679837, 534717478, 534755023, 534792473, 534829827, 534867086, 534904252,
	534941325, 534978305, 535015194, 535051992, 535088699, 535125317, 535161846, 535198287,
	535234640, 535270905, 535307085, 535343178, 535379187, 535415110, 535450950, 535486706,
	535522379, 535557970, 535593480, 535628908, 535664255, 535699523, 535734711, 535769820,
	535804850, 535839803, 535874678, 535909476, 535944198, 535978844, 536013414, 536047910,
	536082331, 536116678, 536150952, 536185153, 536219281, 536253337, 536287322, 536321235,
	536355078, 536388850, 536422553, 536456186, 536489750, 536523246, 536556673, 536590033,
	536623325, 536656551, 536689709, 536722802, 536755829, 536788791, 536821688, 536854520,
	536887280, 536919921, 536952436, 536984827, 537017094, 537049241, 537081267, 537113174,
	537144963, 537176637, 537208195, 537239640, 537270972, 537302193, 537333304, 537364306,
	537395200, 537425987, 537456669, 537487246, 537517720, 537548091, 537578361, 537608530,
	537638600, 537668572, 537698446, 537728224, 537757906, 537787493, 537816986, 537846387,
	537875696, 537904913, 537934040, 537963078, 537992027, 538020888, 538049662, 538078350,
	538106952, 538135470, 538163903, 538192254, 538220521, 538248707, 538276812, 538304837,
	538332781, 538360647, 538388434, 538416144, 538443776, 538471332, 538498812, 538526217,
	538553548, 538580804, 538607987, 538635097, 538662136, 538689102, 538715997, 538742822,
	538769577, 538796263, 538822880, 538849428, 538875909, 538902322, 538928668, 538954949,
	538981163, 539007312, 539033396, 539059416, 539085373, 539111265, 539137095, 539162863,
	539188568, 539214212, 539239794, 539265316, 539290778, 539316180, 539341522, 539366806,
	539392031, 539417197, 539442306, 539467358, 539492352, 539517290, 539542171, 539566997,
	539591768, 539616483, 539641143, 539665749, 539690301, 539714800, 539739245, 539763637,
	539787976, 539812264, 539836499, 539860682, 539884815, 539908896, 539932927, 539956907,
	539980838, 540004718, 540028549, 540052332, 540076065, 540099750, 540123387, 540146976,
	540170517, 540194011, 540217458, 540240858, 540264211, 540287519, 540310780, 540333996,
};

float qsqrt( float f )
{
    INTORFLOAT      fi;
    unsigned int    e, n;

    /* Get raw bits of floating point f. */
    fi.f = f;
    n = fi.i;

    /* Divide exponent by 2 -- this is done in-place, no need to shift all
       the way down to 0 the least significant bits and then back up again.
       Note that we are also dividing the exponent bias (127) by 2, this
       gets corrected when we add in the sqrttable entry. */
    e = (n >> 1) & 0x3f800000;

    /* n is the table index -- we're using 1 bit from the original exponent
       (e0) plus 7 bits from the mantissa. */
    n = (n >> 16) & 0xff;

    /* Add calculated exponent to mantissa + re-biasing constant from table. */
    fi.i = e + sqrttable[n];

    /* Return floating point result. */
    return fi.f;
}
