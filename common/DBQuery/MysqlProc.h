#ifndef __MYSQL_PROC_H_
#define __MYSQL_PROC_H_

#include "MysqlDB.h"

class CMysqlProc : public CMysqlDB
{
private:
public:
	CMysqlProc();
	~CMysqlProc();

	bool		CallProc();
};

#endif
