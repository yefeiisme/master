#ifndef __I_LUA_SCRIPT_H_
#define __I_LUA_SCRIPT_H_

#include "lua.h"

class ILuaScript
{
public:
	virtual bool		Initialize() = 0;
	virtual void		Release() = 0;
	virtual bool		Load(char *pstrFileName) = 0;
	virtual bool		ExecuteCode() = 0;
	virtual bool		CallFunction(const char *pstrFuncName, int nResults, char *pstrFormat,...) = 0;
	virtual bool		RegisterFunction(char *pstrFuncName, lua_CFunction Func) = 0;
	virtual void		SafeCallBegin(int *pIndex) = 0;
	virtual void		SafeCallEnd(int nIndex) = 0;
	virtual int			GetTop() = 0;
	virtual void		SetTop(const int nIndex) = 0;
	virtual lua_Number	LuaToNumber(int nIndex) = 0;
	virtual const char	*LuaToString(int nIndex) = 0;
	virtual bool		LoadBuffer(BYTE *pBuffer, unsigned int uiLen) = 0;
	virtual void		RegisterStandardFunctions() = 0;
};

ILuaScript				*CreateLuaScript();

#endif

