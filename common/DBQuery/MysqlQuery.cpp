#include "stdafx.h"
#include "IDataBase.h"
#include "MysqlQuery.h"

CMysqlQuery::CMysqlQuery()
{
}

CMysqlQuery::~CMysqlQuery()
{
}

bool CMysqlQuery::Execute(char *pstrSQL)
{
	if (NULL == pstrSQL)
		return false;

	size_t	uStrLen	= strlen(pstrSQL);
	if (0 == mysql_real_query(m_pSock, pstrSQL, (unsigned long)uStrLen))
		return false;

    return true;
}

IDBQuery *CreateDBQuery(char *pstrHost, char *pstrAccount, char *pstrPassword, char *pstrDBName)
{
	CMysqlQuery	*pMysqlQuery	= new CMysqlQuery;
	if (NULL == pMysqlQuery)
		return NULL;

	return pMysqlQuery;
}
