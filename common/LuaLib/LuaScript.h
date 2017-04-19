#ifndef __LUA_SCRIPT_H_
#define __LUA_SCRIPT_H_

#include "ILuaScript.h"
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
class CLuaScript : public ILuaScript
{
private:
	lua_State	*m_LuaState;
public:
	CLuaScript();
	~CLuaScript();

	bool		Initialize();
	void		Release();
	bool		Load(char *pstrFileName);
	bool		ExecuteCode();
	bool		CallFunction(const char *pstrFuncName, int nResults, char *pstrFormat, va_list vlist);
	bool		CallFunction(const char *pstrFuncName, int nResults, char *pstrFormat,...);
	bool		RegisterFunction(char *pstrFuncName, lua_CFunction Func);
	void		SafeCallBegin(int *pIndex);
	void		SafeCallEnd(int nIndex);
	int			GetTop();
	void		SetTop(const int nIndex);
	lua_Number	LuaToNumber(int nIndex);
	const char	*LuaToString(int nIndex);
	bool		LoadBuffer(BYTE *pBuffer, unsigned int uiLen);
	void		RegisterStandardFunctions();
};

#endif
