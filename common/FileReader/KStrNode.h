#ifndef	KStrNode_H
#define	KStrNode_H
//---------------------------------------------------------------------------
#include "KNode.h"
#include "KStrBase.h"
//---------------------------------------------------------------------------
#define MAX_STRLEN		80
//---------------------------------------------------------------------------
class ENGINE_API KStrNode : public KNode
{
public:
	char m_Name[MAX_STRLEN];
public:
	KStrNode(void);
	virtual char* GetName(){return m_Name;};
	virtual void  SetName(char* str){g_StrCpyLen(m_Name, str, MAX_STRLEN);};
};
//---------------------------------------------------------------------------
#endif
