#ifndef __MYSQL_QUERY_H_
#define __MYSQL_QUERY_H_

#include "MysqlDB.h"

class CMysqlQuery : public CMysqlDB
{
private:
public:
	CMysqlQuery();
	~CMysqlQuery();

	bool		Initialize(char *pstrHost, char *pstrUser, char *pstrPassword, char *pstrDBName);
	void		Release();

    bool        Connect();
    bool        Reconnect();
    void        Disconnect();

	bool        Execute(char *pstrSQL);
};

#endif
