#ifndef __MYSQL_DB_H_
#define __MYSQL_DB_H_

#include "IDataBase.h"
#include "IRingBuffer.h"

class CMysqlDB : public IDBQuery , public IDBProc
{
private:
	char			m_strDBHost[MAX_HOST_LEN];
	char			m_strUser[MAX_USER_LEN];
	char			m_strPassword[MAX_PASSWORD_LEN];
	char			m_strDBName[MAX_DB_NAME_LEN];
protected:
	MYSQL			*m_pSock;

	bool			m_bConnected;
	int				m_nLastSqlError;

	IRingBuffer		*m_pSqlBuffer;
public:
	CMysqlDB();
	~CMysqlDB();

	bool			Initialize(char *pstrHost, char *pstrUser, char *pstrPassword, char *pstrDBName);
	void			Release();
	virtual bool	ExecuteSQL(char *pstrSQL);
	virtual bool	CallProc();
};

#endif
