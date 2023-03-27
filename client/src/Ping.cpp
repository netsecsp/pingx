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
#include <iphlpapi.h>
#include <icmpapi.h>

BEGIN_ASYN_MESSAGE_MAP(CAsynPingHandler)
    ON_QUERY_RESULT(OnQueryResult, IAsynIoOperation)
	ON_IOMSG_NOTIFY(OnIomsgNotify)
	ON_TIMER(OnTimer)
END_ASYN_MESSAGE_MAP()

HRESULT CAsynPingHandler::OnQueryResult( uint64_t lparam1, uint64_t lparam2, IAsynIoOperation** ppAsynIoOperation )
{
    if( lparam1 == (uint64_t)(m_spAsynFrame.p))
        return m_spAsynNetwork->CreateAsynIoOperation(m_spAsynFrame, m_iaf, 0, IID_IAsynIoOperation, (void**)ppAsynIoOperation);
    else
        return E_NOTIMPL;
}

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
            printf("resolver %.*s, error=%d\n", host.len, host.ptr, lErrorCode);
            SetEvent(m_hNotify);
            return S_OK;
        }

        CComPtr< IStringStack > lstIps;
        lpAsynIoOperation->GetCompletedObject(1, IID_IStringStack, (void**)&lstIps);
        asynsdk::CStringSetterRef ipvx(1,&m_ipvx);
        lstIps->Pop(&ipvx);
        lstIps->Get(&host);
        printf("start to ping %.*s[%s]...\n", host.len, host.ptr, m_ipvx.c_str());
        m_spAsynFrame->CreateTimer(1, 0, 0, 0);
        return  m_spAsynFrame->Add(lpAsynIoOperation, 0);
    }
    else
    {
        asynsdk::CStringSetter host(1);
        spAsynIoOperation->GetPeerAddress(&host, 0, 0, 0);

        if( lErrorCode == NO_ERROR )
        {
            printf("from %s: seq=%lld ttl=%lld rtt=%lldms\n", host.m_val.c_str(), lparam1, lparam2 >> 56, (lparam2 << 8) >> 8);
        }
        else
        {
            const char *message;
            switch(lErrorCode) {
                case IP_DEST_HOST_UNREACHABLE:
                     message = "Destination host is unreachable";
                     break;
                case IP_DEST_NET_UNREACHABLE:
                     message = "Destination Network is unreachable";
                     break;
                case IP_REQ_TIMED_OUT:
                case ERROR_TIMEOUT:
                     message = "Request timed out";
                     break;
                case IP_TTL_EXPIRED_TRANSIT:
                     message = "TTL expired in transit";
                     break;
                case IP_GENERAL_FAILURE:
                     message = "Unable to access destination host";
                     break;
                default:
                     message = 0;
            }
            if( message )
                printf("from %s: seq=%lld %s\n", host.m_val.c_str(), lparam1, message);
            else
                printf("from %s: seq=%lld error=%d\n", host.m_val.c_str(), lparam1, lErrorCode);
        }

        m_spAsynFrame->CreateTimer(lparam1 + 1, 0, lErrorCode==ERROR_TIMEOUT || lErrorCode==IP_REQ_TIMED_OUT? 0 : 1000, 0);
        return  m_spAsynFrame->Add(lpAsynIoOperation, 0);
    }
}

HRESULT CAsynPingHandler::OnTimer(uint64_t lparam1, uint64_t lparam2)
{
    CObjPtr<IAsynNetIoOperation> spAsynIoOperation; m_spAsynFrame->Pop(0, (IAsynIoOperation**)&spAsynIoOperation.p);
    m_spAsynFrameThread->BindAsynIoOperation(spAsynIoOperation, 0, 0, 1000); //设定Io定时器1000ms

    spAsynIoOperation->SetPeerAddress(0,&STRING_from_string(m_ipvx), 0, 0, 0);
    spAsynIoOperation->SetOpParam1(lparam1);
    m_spAsynRawSocket->Write(spAsynIoOperation, m_ttl/*ttl*/);
    return S_OK;
}
