/*****************************************************************************
Copyright (c) netsecsp 2012-2032, All rights reserved.

Developer: Shengqian Yang, from China, E-mail: netsecsp@hotmail.com, last updated 01/15/2024
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
#include <conio.h>
#include <memory>

#define AAPIDLL_USING

#include "Ping.h"

#ifdef AAPIDLL_USING
#include <frame/asm/ITypedef_i.c>
#include <frame/asm/IAsynFrame_i.c>
#include <frame/asm/IAsynNetwork_i.c>
#endif

#ifdef AAPIDLL_USING
#ifdef _DEBUG
#pragma comment(lib, "asynsdk_mini-MDd.lib")
#else
#pragma comment(lib, "asynsdk_mini-MD.lib")
#endif
#pragma comment(lib, "asyncore_dll.lib")
#else
#pragma comment(lib, "asynframe-MT_lib.lib")
#endif
STDAPI_(extern HRESULT) Initialize( /*[in ]*/IAsynMessageEvents *param1, /*[in ]*/IUnknown *param2 );
STDAPI_(extern HRESULT) Destory();
STDAPI_(extern InstancesManager*) GetInstancesManager();

static void ShowUsage(std::string name)
{
    std::string::size_type i = name.find_last_of("/\\");
    if( i != std::string::npos )
        name.erase(0, i + 1);

    printf("  Usage: %s [-4|6] [-i TTL] [-dns URL] host\n", name.c_str());
    printf("Options:\n");
    printf("      -4 Enforce IPv4\n");
    printf("      -6 Enforce IPv6\n");
    printf("      -i TTL Time to live\n");
    printf("      -dns URL[protocol://[host][:port]/uri] Use DNS on given url\n\n");
    printf("example: %s www.test.com -6 -dns \"udp://*:53\"\n", name.c_str());
    printf("         %s www.test.com -6 -dns \"tcp://*:53\"\n", name.c_str());
    printf("         %s www.test.com -4 -dns nil\n", name.c_str());
    printf("         %s www.test.com -4 -dns \"http://119.29.29.29/d?dn=[host].&ip=[ip]&ttl=1\"\n\n", name.c_str());
}

int _tmain(int argc, _TCHAR *argv[])
{
    printf("Copyright (c) netsecsp 2012-2032, All rights reserved.\n");
    printf("Developer: Shengqian Yang, from China, E-mail: netsecsp@hotmail.com, last updated " STRING_UPDATETIME "\n");
    printf("http://pingx.sf.net\n\n");

    char ipvx = AF_INET, nttl = 0, *host = 0, *durl = "udp://*:53/";
    for(int i = 1; i < argc; ++ i)
    {
        if( strcmp(argv[i], "/?") == 0 || 
            strcmp(argv[i], "--help") == 0 )
        {
            host = 0;
            break;
        }

        if( argv[i][0] == '-' )
        {
            if( strcmp(argv[i], "-4") == 0 )
            {
                ipvx = AF_INET;
                continue;
            }
            if( strcmp(argv[i], "-6") == 0 )
            {
                ipvx = 23;
                continue;
            }
            if( strcmp(argv[i], "-i") == 0 )
            {
                if( argc > ++ i)
                    nttl = atoi(argv[i]);
                continue;
            }
            if( strcmp(argv[i], "-dns") == 0 )
            {
                if( argc > ++ i)
                    durl = argv[i];
                continue;
            }
        }
        else
        {
            host = argv[i];
        }
    }

    if(!host )
    {
        ShowUsage(argv[0]);
        return 0;
    }

    if( Initialize(NULL, NULL) != NO_ERROR )
    {
        printf("fail to Initialize asynframe\n");
        return 0;
    }

    do
    {
        InstancesManager *lpInstancesManager = GetInstancesManager();

        if( lpInstancesManager->Require(STRING_from_string(IN_AsynNetwork)) != S_OK )
        {
            printf("can't load plugin: %s\n", IN_AsynNetwork);
            break;
        }

        CComPtr<IAsynFrameThread> spAsynFrameThread;
        lpInstancesManager->NewInstance(0, asynsdk::TC_Iocp, IID_IAsynFrameThread, (IUnknown**)&spAsynFrameThread);

        CComPtr<IAsynNetwork    > spAsynNetwork;
        lpInstancesManager->GetInstance(STRING_from_string(IN_AsynNetwork), IID_IAsynNetwork, (IUnknown **)&spAsynNetwork);

        std::unique_ptr<CAsynPingHandler> pEvent(new CAsynPingHandler(spAsynFrameThread, spAsynNetwork, ipvx, nttl));
        if( pEvent->Start(host, durl) )
        {
            while( WAIT_OBJECT_0 != WaitForSingleObject(pEvent->m_hNotify, 0) &&
                  _kbhit() == 0 )
            {
                Sleep(100); //0.1sec
            }
        }
        pEvent->Shutdown();
    }while(0);

    if( Destory() != NO_ERROR )
    {
        printf("fail to Destory asynframe\n");
    }

    return 0;
}
