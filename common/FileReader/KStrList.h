#ifndef	KStrList_H
#define	KStrList_H
//---------------------------------------------------------------------------
#include "KList.h"
#include "KStrNode.h"
//---------------------------------------------------------------------------
class ENGINE_API KStrList : public KList
{
public:
	KStrNode* Find(char* str);
};
//---------------------------------------------------------------------------
#endif
