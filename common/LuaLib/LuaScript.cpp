#include "stdafx.h"
#include "LuaScript.h"
#include "IFileLog.h"

CLuaScript::CLuaScript()
{
	m_LuaState	= NULL;
}

CLuaScript::~CLuaScript()
{
	if (m_LuaState)
	{
		lua_close(m_LuaState);
		m_LuaState = NULL;
	}
}

bool CLuaScript::Initialize()
{
	m_LuaState	= lua_open();

	if (NULL == m_LuaState)
	{
		g_pFileLog->WriteLog("Open Lua Failed!\n");
		return false;
	}

	RegisterStandardFunctions();

	return true;
}

void CLuaScript::Release()
{
	delete this;
}

bool CLuaScript::LoadBuffer(BYTE *pBuffer, unsigned int uiLen)
{
	if (uiLen < 0)	
	{
		return false;
	}

	if (luaL_loadbuffer(m_LuaState, (char*) pBuffer, uiLen, NULL) != 0)
	{
		return false;
	}

	return true;
}

bool CLuaScript::Load(char *pstrFileName)
{
	if (luaL_loadfile(m_LuaState, pstrFileName))
	{
		g_pFileLog->WriteLog("[%s][%d] luaL_loadfile[%s] Failed\n", __FILE__, __LINE__, pstrFileName);
		return false;
	}

	if (!ExecuteCode())
	{
		return false;
	}

	return true;
}

bool CLuaScript::ExecuteCode()
{
	if (!m_LuaState)
	{
		g_pFileLog->WriteLog("m_LuaState Did Not Init!\n");
		return false;
	}

	int nState;
	if (nState = lua_pcall(m_LuaState, 0, 0, 0) != 0)
	{
		const char	*pstrError	= lua_tostring(m_LuaState,-1);
		g_pFileLog->WriteLog("%s:%d, Lua Script Execute Error[%s]\n", __FILE__, __LINE__, pstrError);
		return false;
	}

	return	true;
}

bool CLuaScript::CallFunction(const char *pstrFuncName, int nResults, char *pstrFormat, va_list vlist)
{
	double nNumber;
	char * cString	= NULL;
	void * pPoint	= NULL;
	BYTE * pByte = NULL;
	lua_CFunction CFunc;
	int nFormatIndex=0;
	int nArgnum = 0;
	int nIndex = 0;
	int nRetcode;		//调用脚本函数后的返回码

	if (!m_LuaState)
	{
		g_pFileLog->WriteLog("%s:%d, Lua CallFunction Failed!\n", __FILE__, __LINE__);
		return false;
	}

	lua_getglobal(m_LuaState, pstrFuncName); //在堆栈中加入需要调用的函数名

	while (pstrFormat[nFormatIndex] != '\0')
	{
		switch(pstrFormat[nFormatIndex])
		{
		case 'n'://输入的数据是double形 NUMBER，Lua来说是Double型
			{ 
				nNumber = va_arg(vlist, double );
				lua_pushnumber(m_LuaState, nNumber);
				nArgnum ++;

			}
			break;

		case 'd'://输入的数据为整形
			{
				nNumber = (double)(va_arg(vlist, int));
				lua_pushnumber(m_LuaState, (double) nNumber);
				nArgnum ++;
			}
			break;

		case 's'://字符串型
			{
				cString = va_arg(vlist, char *);
				lua_pushstring(m_LuaState, cString);
				nArgnum ++;
			}
			break;
		case 'N'://NULL
			{
				lua_pushnil(m_LuaState);
				nArgnum ++;
			}
			break;

		case 'f'://输入的是CFun形，即内部函数形
			{
				CFunc = va_arg(vlist, lua_CFunction);
				lua_pushcfunction(m_LuaState, CFunc) ;
				nArgnum ++;
			}
			break;

		case 'v'://输入的是堆栈中Index为nIndex的数据类型
			{
				nNumber = va_arg(vlist, int);
				int nIndex1 = (int) nNumber;
				lua_pushvalue(m_LuaState, nIndex1);
				nArgnum ++;
			}
			break;
		case 't'://输入为一Table类型
			{
				//传过来的是一个拼好的数据流，目前只支持table中拥有'd'和's'格式
				//流的头部有该table的长度
				pByte = va_arg(vlist, BYTE*);
				int nReadPos = 0;
				int nLen = *pByte;
				nReadPos += sizeof(int);

				lua_newtable(m_LuaState);
				int nCount = 1;
				while ( nReadPos < nLen )
				{
					switch( pByte[nReadPos] )
					{
					case 'd':
						{
							++nReadPos;
							int nNumValue = *((int*)(pByte + nReadPos));
							nReadPos += sizeof(int);
							lua_pushnumber(m_LuaState, nCount++);
							lua_pushnumber(m_LuaState, nNumValue);
							lua_rawset(m_LuaState, -3);
						}
						break;
					case 's':
						{
							++nReadPos;
							char * szValue = (char*)(pByte + nReadPos);
							int nStrLen = strlen(szValue);
							nReadPos += nStrLen;
							lua_pushnumber(m_LuaState, nCount++);
							lua_pushstring(m_LuaState, szValue);
							lua_rawset(m_LuaState, -3);
						}
						break;
					default:
						++nReadPos;
					}
				}
				++nArgnum;
			}
			break;
		}

		nFormatIndex++;	
	}

	nRetcode = lua_pcall(m_LuaState, nArgnum, nResults, 0);

	if (nRetcode != 0)
	{
		g_pFileLog->WriteLog("%s:%d, Lua Script Execute Error[%s]\n", __FILE__, __LINE__, lua_tostring(m_LuaState,-1));
		return false;
	}

	return	true;
}

bool CLuaScript::CallFunction(const char *pstrFuncName, int nResults, char *pstrFormat,...)
{
	bool bResult	= false;

	va_list vlist;
	va_start(vlist, pstrFormat);
	bResult = CallFunction(pstrFuncName, nResults, pstrFormat, vlist);
	va_end(vlist);

	return bResult;
}

bool CLuaScript::RegisterFunction(char *pstrFuncName, lua_CFunction Func)
{
	if (!m_LuaState)
		return false;

	lua_register(m_LuaState, pstrFuncName, Func);

	return true;
}

void CLuaScript::SafeCallBegin(int *pIndex)
{
	if (!m_LuaState)
		return;

	*pIndex = lua_gettop(m_LuaState);
}

void CLuaScript::SafeCallEnd(int nIndex)
{
	if (!m_LuaState)
		return;

	lua_settop(m_LuaState, nIndex);
}

int CLuaScript::GetTop()
{
	return lua_gettop(m_LuaState);
}

void CLuaScript::SetTop(const int nIndex)
{
	if (!m_LuaState)
		return;

	lua_settop(m_LuaState, nIndex);
}

lua_Number CLuaScript::LuaToNumber(int nIndex)
{
	return lua_tonumber(m_LuaState, nIndex);
}

const char *CLuaScript::LuaToString(int nIndex)
{
	return lua_tostring(m_LuaState, nIndex);
}

void CLuaScript::RegisterStandardFunctions()
{
	if (!m_LuaState)
		return ;

	luaopen_base(m_LuaState);
	luaopen_table(m_LuaState);
	luaopen_string(m_LuaState);
	luaopen_math(m_LuaState);
#ifdef _DEBUG
	luaopen_debug(m_LuaState);
#endif
	return;	
}

ILuaScript *CreateLuaScript()
{
	CLuaScript	*pLuaScript	= new CLuaScript;
	if (NULL == pLuaScript)
		return NULL;

	if (!pLuaScript->Initialize())
	{
		delete pLuaScript;
		return NULL;
	}

	return pLuaScript;
}
