﻿///////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_PROXY_LUA_H__8A503072_4124_4233_9BEF_3671D8669695__INCLUDED_)
#define AFX_PROXY_LUA_H__8A503072_4124_4233_9BEF_3671D8669695__INCLUDED_
/*****************************************************************************
Copyright (c) netsecsp 2012-2032, All rights reserved.

Developer: Shengqian Yang, from China, E-mail: netsecsp@hotmail.com, last updated 01/15/2024
http://asynframe.sf.net

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above
copyright notice, this list of conditions and the
following disclaimer.

* Redistributions in binary form must reproduce the
above copyright notice, this list of conditions
and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef __cplusplus
extern "C" {
#endif
#include "v504/lua.h" //Using lua 5.4.4
#include "v504/lualib.h"
#include "v504/lauxlib.h"
#include "v504/llimits.h"
#ifdef __cplusplus
}
#endif
#include "LuaTinker.h"
#include "../AsynCore.h"
#include "../asm/IScriptHost.h"
NAMESPACE_BEGIN(lua)

/////////////////////////////////////////////////////////////////////
IScriptHost *GetScriptHost( /*[in ]*/lua_State *pState ); //get from sys.xvmhost

//仅在栈顶生成object对应的lua对象
//1.addref_for_lua=true表示在lua端持有相应c对象的应用技术器
bool         Create( /*[in ]*/lua_State *pState, /*[in ]*/IUnknown *object, /*[in ]*/const char *name, /*[in ]*/bool addref_for_lua = false );
/////////////////////////////////////////////////////////////////////

NAMESPACE_END(lua)

#endif // !defined(AFX_PROXY_LUA_H__8A503072_4124_4233_9BEF_3671D8669695__INCLUDED_)