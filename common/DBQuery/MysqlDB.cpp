#include "stdafx.h"
#include "MysqlDB.h"

CMysqlDB::CMysqlDB()
{
	memset(m_strDBHost, 0, sizeof(m_strDBHost));
	memset(m_strUser, 0, sizeof(m_strUser));
	memset(m_strPassword, 0, sizeof(m_strPassword));
	memset(m_strDBName, 0, sizeof(m_strDBName));

	m_pSock         = NULL;
	m_bConnected    = false;
	m_nLastSqlError = 0;
	m_pSqlBuffer	= NULL;

	my_init();
}

CMysqlDB::~CMysqlDB()
{
}

bool CMysqlDB::Initialize(char *pstrHost, char *pstrUser, char *pstrPassword, char *pstrDBName)
{
	if (NULL == pstrHost || NULL == pstrUser || NULL == pstrPassword || NULL == pstrDBName)
		return false;

	memcpy(m_strDBHost, pstrHost, sizeof(m_strDBHost));
	memcpy(m_strUser, pstrUser, sizeof(m_strUser));
	memcpy(m_strPassword, pstrPassword, sizeof(m_strPassword));
	memcpy(m_strDBName, pstrDBName, sizeof(m_strDBName));

	m_strDBHost[sizeof(m_strDBHost)-1] = '\0';
	m_strUser[sizeof(m_strUser)-1] = '\0';
	m_strPassword[sizeof(m_strPassword)-1] = '\0';
	m_strDBName[sizeof(m_strDBName)-1] = '\0';

	m_pSock = mysql_init(NULL);
	if (NULL == m_pSock)
		return false;

	my_bool	bReconnectFlag	= 0;
	if (0 != mysql_options(m_pSock, MYSQL_OPT_RECONNECT, &bReconnectFlag))
		return false;

	my_bool	bAutoReCnn	= 0;
	if (0 != mysql_options(m_pSock, MYSQL_OPT_RECONNECT, (const char *)&bAutoReCnn))
		return false;

	unsigned int unCnnTimeOut	= 0;
	if (0 != mysql_options(m_pSock, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&unCnnTimeOut))
		return false;

	char strDefaultCharset[] = "binary";
	if (0 != mysql_options(m_pSock, MYSQL_SET_CHARSET_NAME, strDefaultCharset))
		return false;

	MYSQL	*pConnection   = mysql_real_connect(m_pSock, m_strDBHost, m_strUser, m_strPassword, m_strDBName, 3306, NULL, 0);
	if (NULL == pConnection)
	{
		// Show Error : connnect to %s db error!!!
		// ...
		return false;
	}

	return true;
}

void CMysqlDB::Release()
{
	if (m_pSqlBuffer)
	{
		m_pSqlBuffer->Release();
		m_pSqlBuffer	= NULL;
	}
}
