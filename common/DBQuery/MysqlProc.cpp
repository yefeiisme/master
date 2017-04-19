#include "stdafx.h"
#include "IDataBase.h"
#include "MysqlProc.h"

CMysqlProc::CMysqlProc()
{
}

CMysqlProc::~CMysqlProc()
{
}

bool CMysqlProc::CallProc()
{
	return true;
}

IDBProc *CreateDBProc(char *pstrHost, char *pstrAccount, char *pstrPassword, char *pstrDBName)
{
	CMysqlProc	*pMysqlProc	= new CMysqlProc;
	if (NULL == pMysqlProc)
		return NULL;

	return pMysqlProc;
}
