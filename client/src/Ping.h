#ifndef __AsynDnsHandler_H__
#define __AsynDnsHandler_H__
/*****************************************************************************
Copyright (c) 2012 - 2019, All rights reserved.

Author: Shengqian Yang, netsecsp@hotmail.com, China, last updated 07/01/2016
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
#include <frame/AsynNetwork_internal.h>

class CAsynPingHandler : public asynsdk::asyn_message_events_impl
{
public:
    CAsynPingHandler(IAsynFrameThread *lpAsynFrameThread, IAsynNetwork *lpAsynNetwork)
    {
        m_spAsynFrameThread = lpAsynFrameThread;
        CreateAsynFrame(lpAsynFrameThread, 0, &m_spAsynFrame);
        m_spAsynNetwork = lpAsynNetwork;
        m_spAsynNetwork->CreateAsynRawSocket(1, &m_spAsynRawSocket);
        m_spAsynRawSocket->Open(lpAsynFrameThread, 0, 0, 0);
        m_hNotify = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

public: // interface of asyn_message_events_impl
    DECLARE_ASYN_MESSAGE_MAP(CAsynPingHandler)
    HRESULT OnIomsgNotify( uint64_t lParam1, uint64_t lParam2, IAsynIoOperation *lpAsynIoOperation );
    HRESULT OnTimer( uint64_t lParam1, uint64_t lParam2 );

public:
    bool Start(const std::string &host, uint32_t af, const char *DNS_uri)
    {
        m_spAsynNetwork->CreateAsynIoOperation(m_spAsynFrame, af, 0, IID_IAsynNetIoOperation, (void **)&m_spAsynIoOperation);
        if( m_spAsynIoOperation->SetHost(STRING_from_string(host), TRUE) == S_OK )
        {// ipvx
            printf("start to ping %s...\n", host.c_str());
            m_spAsynFrame->CreateTimer(1, 0, 0, 0);
        }
        else
        {
            m_spAsynNetwork->CreateAsynDnsResolver(STRING_from_string("dns"), 0, STRING_from_string(DNS_uri), 0, &m_spAsynDnsResolver);
            m_spAsynDnsResolver->Commit(m_spAsynIoOperation, 0);
        }
        return true;
    }
    void Shutdown()
    {
        Stop(m_spAsynFrame);
        m_spAsynFrame = NULL;
    }

public:
    CComPtr<IAsynRawSocket     > m_spAsynRawSocket;
    CComPtr<IAsynNetwork       > m_spAsynNetwork;
    CComPtr<IAsynFrame         > m_spAsynFrame;
    CComPtr<IAsynDnsResolver   > m_spAsynDnsResolver;
    CComPtr<IAsynNetIoOperation> m_spAsynIoOperation;
    CComPtr<IAsynFrameThread   > m_spAsynFrameThread;
    HANDLE m_hNotify;
};

#endif
