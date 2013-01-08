//------------------------------------------------------------------------------
//
//  SimConnect Data Request Sample
//  
//	Description:
//				After a flight has loaded, request the lat/lon/alt of the user 
//				aircraft
//------------------------------------------------------------------------------

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#include "SimConnect.h"

int     quit = 0;
HANDLE  hSimConnect = NULL;

struct Struct1
{
    double  kohlsmann;
    double  latitude;
    double  longitude;
};

static enum DATA_DEFINE_ID {
    DEFINITION_1,
};

static enum DATA_REQUEST_ID {
    REQUEST_1,
};

void CALLBACK MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
    HRESULT hr;
    
    switch(pData->dwID)
    {
        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
        {
            SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;            
            switch(pObjData->dwRequestID)
            {
                case REQUEST_1:
                {
                    DWORD ObjectID = pObjData->dwObjectID;
                    Struct1 *pS = (Struct1*)&pObjData->dwData;
                    printf("\nObjectID=%d  Lat=%f  Lon=%f  Kohlsman=%.2f", ObjectID, pS->latitude, pS->longitude, pS->kohlsmann );
                    break;
                }

                default:
                   break;
            }
            break;
        }
        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
        {
            SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;            
            switch(pObjData->dwRequestID)
            {
                case REQUEST_1:
                {
                    DWORD ObjectID = pObjData->dwObjectID;
                    Struct1 *pS = (Struct1*)&pObjData->dwData;
                    printf("\nObjectID=%d  Lat=%f  Lon=%f  Kohlsman=%.2f", ObjectID, pS->latitude, pS->longitude, pS->kohlsmann );
                    break;
                }

                default:
                   break;
            }
            break;
        }
    }
}

void testDataRequest()
{
    HRESULT hr;

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Request Data", NULL, 0, 0, 0)))
    {
        printf("\nConnected to Flight Simulator!");   
        
        // Set up the data definition, but do not yet do anything with it
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Kohlsman setting hg", "inHg");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Plane Latitude", "degrees");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Plane Longitude", "degrees");

        // hr = SimConnect_RequestDataOnSimObjectType(hSimConnect, REQUEST_1, DEFINITION_1, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
        hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST_1, DEFINITION_1, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_ONCE);

        while( 0 == quit )
        {
            SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, NULL);
            Sleep(1);
        } 

        hr = SimConnect_Close(hSimConnect);
    }
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{

    testDataRequest();

	return 0;
}
