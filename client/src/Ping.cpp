/*****************************************************************************
Copyright (c) 2012-2032, All rights reserved.

Author: Shengqian Yang, netsecsp@hotmail.com, China, last updated 05/01/2022
http://pingx.sf.net

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
#include "stdafx.h"
#include "Ping.h"
#include <conio.h>

BEGIN_ASYN_MESSAGE_MAP(CAsynPingHandler)
	ON_IOMSG_NOTIFY(OnIomsgNotify)
	ON_TIMER(OnTimer)
END_ASYN_MESSAGE_MAP()

HRESULT CAsynPingHandler::OnIomsgNotify( uint64_t lparam1, uint64_t lparam2, IAsynIoOperation *lpAsynIoOperation )
{
    uint32_t lErrorCode = NO_ERROR;
    lpAsynIoOperation->GetCompletedResult(&lErrorCode, 0, 0 );

    CComPtr<IAsynNetIoOperation> spAsynIoOperation;
    lpAsynIoOperation->QueryInterface(IID_IAsynNetIoOperation, (void **)&spAsynIoOperation);

    if( lparam2 == Io_hostdns )
    {
        STRING host;
        spAsynIoOperation->GetHost(&host );
        if( lErrorCode != NO_ERROR )
        {
            printf("can't to resolver %*s, error=%d\n", host.len, host.ptr, lErrorCode);
            SetEvent(m_hNotify);
            return S_OK;
        }

        CComPtr< IStringStack> lstIps;
        lpAsynIoOperation->GetCompletedObject(1, IID_IStringStack, (void**)&lstIps);
        asynsdk::CStringSetter ipvx(1);
        lstIps->Pop(&ipvx);
        spAsynIoOperation->SetPeerAddress(0,&STRING_from_string(ipvx.m_val), 0, 0, 0);
        lstIps->Get(&host);
        printf("start to ping %*s[%s]...\n", host.len, host.ptr, ipvx.m_val.c_str());
        m_spAsynFrame->CreateTimer(1, 0, 0, 0);
        return S_OK;
    }

    asynsdk::CStringSetter host(1);
    spAsynIoOperation->GetPeerAddress(&host, 0, 0, 0);

    if( lErrorCode == NO_ERROR )
        printf("from %s: seq=%lld ttl=%lld rtt=%lldms\n", host.m_val.c_str(), lparam1, lparam2 >> 56, (lparam2 << 8) >> 8);
    else
        printf("from %s: seq=%lld, error=%d\n", host.m_val.c_str(), lparam1, lErrorCode);
    return E_NOTIMPL;
}

HRESULT CAsynPingHandler::OnTimer(uint64_t lparam1, uint64_t lparam2)
{
    m_spAsynFrame->CreateTimer(1, ++ lparam2, 1000/*1 sec*/, 0);

    CComPtr<IAsynNetIoOperation> spAsynIoOperation; m_spAsynNetwork->CreateAsynIoOperation(m_spAsynFrame, 0, 0, IID_IAsynNetIoOperation, (void **)&spAsynIoOperation);
    m_spAsynFrameThread->BindAsynIoOperation(m_spAsynIoOperation, spAsynIoOperation, BM_Oneway | BM_OsAddr, 0); //单向绑定 | 直接引用地址

    spAsynIoOperation->SetOpParam1(lparam2);
    m_spAsynRawSocket->Write(spAsynIoOperation, 0/*ttl*/);
    return S_OK;
}
