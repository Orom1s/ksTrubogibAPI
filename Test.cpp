#include "stdafx.h"
#include <afxdllx.h>
#include "LHead3d.h"
#include "Test.h"
#include "Object.h"
#include "SArray.H"
#include "SSArray.h"
#include <array>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_VAL 50.0
#define MIN_VAL     50.0
#define MAX_VAL     500.0

//-------------------------------------------------------------------------------
// ����������� ��������� ������������ � ������� ������������� DLL
// ---
static AFX_EXTENSION_MODULE TestDLL = { NULL, NULL };
HINSTANCE g_hInstance = NULL;
ksAPI7::IApplicationPtr newKompasAPI( NULL );               

void OnProcessDetach();                 // ���������� ����������

// ���������� ����������
int oType = o3d_edge;										// ��� �������� ��� ������� ����������

//-------------------------------------------------------------------------------
// ����������� ����� �����
// ������������� � ���������� DLL
// ---
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  g_hInstance = hInstance;

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("Test.AWX Initializing!\n");

		AfxInitExtensionModule(TestDLL, hInstance);

		new CDynLinkLibrary(TestDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("Test.AWX Terminating!\n");
    OnProcessDetach();
		AfxTermExtensionModule(TestDLL);
	}
	return 1;   // ok
}

//-------------------------------------------------------------------------------
// �������� ������ � ������ API
// ---
void GetNewKompasAPI() 
{
  if ( !( ksAPI7::IApplication * )newKompasAPI ) 
  {
    CString filename;
    
    if( ::GetModuleFileName(NULL, filename.GetBuffer(255), 255) ) 
    {
      filename.ReleaseBuffer( 255 );
      CString libname;
      
      libname = LoadStr( IDR_API7 );    // kAPI7.dll
      filename.Replace( filename.Right(filename.GetLength() - (filename.ReverseFind('\\') + 1)), 
        libname );
      
      HINSTANCE hAppAuto = LoadLibrary( filename ); // ������������� kAPI7.dll
      
      if(  hAppAuto ) 
      {
        // ��������� �� ������� ������������ ��������� KompasApplication  
        typedef LPDISPATCH ( WINAPI * FCreateKompasApplication )(); 
        
        FCreateKompasApplication pCreateKompasApplication = 
          (FCreateKompasApplication)GetProcAddress( hAppAuto, "CreateKompasApplication" );	
        
        if ( pCreateKompasApplication )
          newKompasAPI = IDispatchPtr ( pCreateKompasApplication(), false /*AddRef*/); // �������� ��������� Application
        FreeLibrary( hAppAuto );  
      }
    }
  }
}

//-------------------------------------------------------------------------------
// ������ ������������� ��������
// ---
unsigned int WINAPI LIBRARYID()
{
  return IDR_LIBID;
}

//-------------------------------------------------------------------------------
// �������� ������� ��������
// ---
IDocument3DPtr GetCurrentDocument()
{
  reference rDoc = ::ksGetCurrentDocument( 3/*cPartOrAssemblyType*/ );
  IDocument3DPtr doc(::ksGet3dDocumentFromReference(rDoc), false/*AddRef*/);
  return doc;
}


//-------------------------------------------------------------------------------
// �������� ������������ �������
// ---
void RunCommand( IDocument3DPtr & doc  );

//-----------------------------------------------------------------------------
// ������� 3D �������� � �����
// ---
IDocument3DPtr Select3DDocument()
{
  IDocument3DPtr res = NULL;

  CString docFileName;
  bool done = !!::ksSelectD3ModelT( docFileName.GetBuffer( _MAX_PATH ), _MAX_PATH, false/*onlyDetail*/, true/*showAddNum*/ );
  docFileName.ReleaseBuffer();
  if (done && !docFileName.IsEmpty())
  {
    reference frgDoc = OpenDocumentT( _bstr_t( docFileName ), 1 );
    res.Attach( ::ksGet3dDocumentFromReference( frgDoc ), false /*AddRef*/ );
  }

  return res;
}

//-----------------------------------------------------------------------------
// ������� ��������� �������� � �����
// ---
reference SelectTextDocument() {
    CString docFileName;
    if (!!::ksChoiceFileT(0, 0, docFileName.GetBuffer(_MAX_PATH), _MAX_PATH, false)) {
        docFileName.ReleaseBuffer();
        reference frgDoc = ::OpenTextFileT(_bstr_t(docFileName));
        return frgDoc;
    }
}

//-----------------------------------------------------------------------------
// �������� ������������� ��������� � ���������.
// ---
IPartPtr GetWorkPart( const IDocument3DPtr doc )
{
  IPartPtr res;

  if (doc)
  {
    res.Attach( doc->GetPart( pInPlace_Part ), false /*AddRef*/ );
    if (!res)
      res.Attach( doc->GetPart( pTop_Part ), false /*AddRef*/ );
  }

  return res;
}

// ---
VARIANT CreateSafeArray(std::vector<double>& points)
{
    VARIANT varRow;
    VariantInit(&varRow);

    int varsCount = points.size() * 3;

    SAFEARRAYBOUND sabNewArray;
    sabNewArray.cElements = varsCount;
    sabNewArray.lLbound = 0;

    SAFEARRAY* pSafe = ::SafeArrayCreate(VT_R8, 1, &sabNewArray);

    if (pSafe)
    {
        for (long i = 0, count = points.size(); i < count; i++)
        {
            double tmp = points[i];
            ::SafeArrayPutElement(pSafe, &i, &tmp);
        }

        V_VT(&varRow) = VT_ARRAY | VT_R8;
        V_ARRAY(&varRow) = pSafe;
    }
    return varRow;
}

void CreateLocalCS(IDocument3DPtr& doc) {
    IPartPtr part(doc->GetPart(pTop_Part));
    if (part) {
        ksAPI7::IPart7Ptr part7(doc->GetPart(pTop_Part));
        MessageT(_T("Ok0")); //�����������
        if (part7)
        {
            MessageT(_T("Ok1")); //�� �����������
            ksAPI7::IAuxiliaryGeomContainerPtr container(part7);
            if (container)
            {
                MessageT(_T("Ok2"));
                ksAPI7::ILocalCoordinateSystemsPtr systems(container->LocalCoordinateSystems);
                if (systems)
                {
                    MessageT(_T("Ok3"));
                    ksAPI7::ILocalCoordinateSystemPtr localCS = systems->Add();
                    if (localCS)
                    {
                        MessageT(_T("Ok4"));
                        ksAPI7::IPoint3DPtr point(localCS);
                        point->put_ParameterType(ksPParamCoord);
                        point->put_X(0);
                        point->put_Y(0);
                        point->put_Z(-200);
                        localCS->PutOrientationType(ksAxisOrientation);
                        localCS->Update();
                        ksAPI7::ILocalCSAxesDirectionParamPtr axespar(localCS->LocalCSParameters);
                        /* std::vector<double> matrix({ 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, -200.0, 1.0 });
                        localCS->InitByMatrix3D(CreateSafeArray(matrix));
                        localCS->Update();*/
                        //std::array<double, 16> mat = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, -200.0, 1.0 };
                        _variant_t saMatrixes; //  �������� ������� � ���� ����������� ������� �� 16 ���������. ������� ����� ������ 4�4
                        /*V_VT(&saMatrixes) = VT_ARRAY | VT_R8;
                        V_ARRAY(&saMatrixes) = mat;
                        saMatrixes.Attach(mat);*/
                        localCS->InitByMatrix3D(saMatrixes);
                        localCS->Update();
                        MessageT(_T("Ok5"));
                    }
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
// ������� ������������ (��� ������������ ������ � ������)
// ---
IEntityPtr CreateExtrusion( IPartPtr& part )
{ 
    
  IEntityPtr res = NULL;

  if (part)
  {
    double val = DEFAULT_VAL;

    // ���� ������� ������� �����
    if ( ::ReadDoubleW(_bstr_t(::LoadStr(IDS_CIRCLE_RADIUS)), DEFAULT_VAL, MIN_VAL, MAX_VAL,  &val) )
    {
      // ����� �������� ������������
      IEntityPtr entitySketch(part->NewEntity(o3d_sketch), false /*AddRef*/); // 

      if (entitySketch)
      {
        // �������� ��������� �� ��������� ���������� �������� � ���������
        ISketchDefinitionPtr sketchDefinition(IUnknownPtr(entitySketch->GetDefinition(), false /*AddRef*/));
        if (sketchDefinition)
        {
          // ������� ��������� ������� ��������� XOY
          IEntityPtr basePlane(part->GetDefaultEntity(o3d_planeXOY), false /*AddRef*/);
          
          // ��������� ���������� ������
          sketchDefinition->SetPlane(basePlane); // ��������� ��������� XOY ������� ��� ������
          sketchDefinition->SetAngle(0);      // ���� �������� ������

          entitySketch->Create(); // �������� �����

          // ����� � ����� �������������� ������
          if (sketchDefinition->BeginEdit())
          {
            // �������� ����� ����� - �������
            /*::LineSeg( .0, .0, val, .0, 1 );
            ::LineSeg( val, .0, val, val, 1 );
            ::LineSeg( val, val, .0, val, 1 );
            ::LineSeg( .0, val, .0, .0, 1 );*/
            ::Circle(200, .0, val, 1);
            // ����� �� ������ �������������� ������
            sketchDefinition->EndEdit();
          }

          // ������� ������������
          IEntityPtr entityExtrusion(part->NewEntity(o3d_bossExtrusion), false /*AddRef*/);

          if (entityExtrusion)
          {
            // ��������� ������� �������� ������������
            IBossExtrusionDefinitionPtr extrusionDefinition(IUnknownPtr(entityExtrusion->GetDefinition(), false/*AddRef*/));
            if (extrusionDefinition)
            {
              // ��������� ���������� �������� ������������
              extrusionDefinition->SetDirectionType(dtNormal);
              extrusionDefinition->SetSideParam(true, etBlind, 200.0, .0, false); // ������ ������������

              // extrusionDefinition->SetThinParam(true, dtBoth, 5.0, 5.0); // ��������� ������ ������
              extrusionDefinition->SetSketch(entitySketch);
              entityExtrusion->Create();
              res.Attach(entityExtrusion, false);
            }
          }
        }
      }
    }
  }

  return res;
}


//IEntityPtr OperationExtrusionAndRotate(IDocument3DPtr& doc, double length, double angle) {
//    IEntityPtr res = NULL;
//
//    IPartPtr part = GetWorkPart(doc);
//
//    if (part) {
//
//    }
//    return res;
//}



//-------------------------------------------------------------------------------
// �������� ��������
// ---
void OperationRotated(IPartPtr& part , double angle)
{
    
    // �������� ����� �����
    IEntityPtr entitySketch(part->NewEntity(o3d_sketch), false/*AddRef*/);
    
    if (entitySketch)
    {
        // �������� ��������� �� ��������� ���������� �������� � ���������
        // ��������� ������� ������
        ISketchDefinitionPtr sketchDefinition(IUnknownPtr(entitySketch->GetDefinition(), false /*AddRef*/));
        if (sketchDefinition)
        {
            // ������� ��������� ������� ��������� XOY
            IEntityPtr basePlane(part->GetDefaultEntity(o3d_planeYOZ), false /*AddRef*/);
            // ��������� ���������� ������
            sketchDefinition->SetPlane(basePlane); // ��������� ��������� XOY ������� ��� ������
            sketchDefinition->SetAngle(-90);

            // �������� �����
            entitySketch->Create();

            // ����� � ����� �������������� ������
            if (sketchDefinition->BeginEdit())
            {
                // ������ ����� �����
                //ArcByAngle(0, 0, 20, -90, 90, 1, 1);
                Circle(0, 200, 50, 1);
                
                LineSeg(-10, 0, 0, 0, 3);
                // ����� �� ������ �������������� ������
                sketchDefinition->EndEdit();
            }
            // �������� ��������
            IEntityPtr entityRotate(part->NewEntity(o3d_bossRotated), false/*AddRef*/);
            if (entityRotate)
            {
                // �������� ��������� �� ��������� ���������� �������� � ���������
                // ��������� ������� �������� ��������
                IBossRotatedDefinitionPtr bossRotatedDefinition(IUnknownPtr(entityRotate->GetDefinition(), false/*AddRef*/));
                if (bossRotatedDefinition)
                {
                    bossRotatedDefinition->SetToroidShapeType(true);      // ������� ������� ( TRUE - ������, FALSE - ������� )
                    bossRotatedDefinition->SetDirectionType(dtReverse);       // ����������� �������� ( dtNormal - ������ �����������, ��� ������ ������ - ������
                    // dtReverse - �������� �����������, ��� ������ ������ - ������, dtBoth - � ��� �������,
                    // dtMiddlePlane - �� ������� ��������� )
// �������� ��������� ������ ������
                    //bossRotatedDefinition->SetThinParam(true,               // ������� ������������ ��������
                    //    dtNormal,             // ����������� ���������� ������ ������
                    //    1,                  // ������� ������ � ������ �����������
                    //    1);                // ������� ������ � �������� �����������
                    // �������� ��������� ������������ � ����� �����������
                    //baseRotatedDefinition->SetSideParam(true,               // ����������� �������� ( TRUE - ������, FALSE - �������� )
                    //    180);              // ���� ��������
                    // �������� ��������� ������������ � ����� �����������
                    bossRotatedDefinition->SetSideParam(false,              // ����������� �������� ( TRUE - ������, FALSE - �������� )
                        angle);              // ���� ��������
                    bossRotatedDefinition->SetSketch(entitySketch);        // ����� �������� ������������                                                                 
                    // ������� ��������              
                    if (entityRotate->Create()) {
                        MessageT(_T("������� �������� ��������"));
                    }
                }
            }
        }
    }
}



//-----------------------------------------------------------------------------
// �������� ���� � ��������
// ---
IBodyPtr GetBodyFromOperation( IEntityPtr obj )
{
  IBodyCollectionPtr coll( obj ? obj->BodyCollection() : NULL, false/*AddRef*/ );
  IBodyPtr body( coll && coll->GetCount() == 1 ? coll->GetByIndex( 0 ) : NULL, false/*AddRef*/ );

  return body;
}

//-----------------------------------------------------------------------------
// �������� ��������� �� ����
// ---
IEntityCollectionPtr GetCollectionFromPart( IPartPtr part, Obj3dType type )
{
  IEntityCollectionPtr entityColl( part ? part->EntityCollection( o3d_conjunctivePoint ) : NULL, false/*AddRef*/ );
  return entityColl;
}

//-------------------------------------------------------------------------------
// �������� ������� ����������
// ---
void WINAPI LIBRARYENTRY( unsigned int comm )
{
    reference file;
	// �������� ������� ��������
	IDocument3DPtr doc( GetCurrentDocument() );
	
	if ( doc )
	{
		switch ( comm )
		{
            case 1: file = SelectTextDocument(); break;
		    case 2: RunCommand( doc ); break;
		}
	}
	else
		MessageT( (LPTSTR)(LPCTSTR)LoadStr(IDS_NODOC) );
}

//-------------------------------------------------------------------------------
// ���������� ����������
// ---
void OnProcessDetach()
{
  newKompasAPI = NULL;
}

//------------------------------------------------------------------------------
// ��������� ������ �� �������
// ---
CString LoadStr( int strID ) 
{
	TCHAR temp[_MAX_PATH];
	::LoadString( TestDLL.hModule, strID, temp, _MAX_PATH ); 
	return temp;
}

//-------------------------------------------------------------------------------
void RunCommand( IDocument3DPtr & doc )
{
  // ����������� ���
  IPartPtr part(GetWorkPart(doc));

  if ( doc )
  {
    ::CreateExtrusion(part);
    ::OperationRotated(part, 90);
    ::CreateLocalCS(doc);
  }
  
}


