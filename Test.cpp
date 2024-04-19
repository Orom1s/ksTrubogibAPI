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
// Специальная структура используемая в течении инициализации DLL
// ---
static AFX_EXTENSION_MODULE TestDLL = { NULL, NULL };
HINSTANCE g_hInstance = NULL;
ksAPI7::IApplicationPtr newKompasAPI( NULL );               

void OnProcessDetach();                 // Отключение библиотеки

// Глобальные переменные
int oType = o3d_edge;										// Тип объектов для функции фильтрации

//-------------------------------------------------------------------------------
// Стандартная точка входа
// Инициализация и завершение DLL
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
// Получить доступ к новому API
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
      
      HINSTANCE hAppAuto = LoadLibrary( filename ); // идентификатор kAPI7.dll
      
      if(  hAppAuto ) 
      {
        // Указатель на функцию возвращающую интерфейс KompasApplication  
        typedef LPDISPATCH ( WINAPI * FCreateKompasApplication )(); 
        
        FCreateKompasApplication pCreateKompasApplication = 
          (FCreateKompasApplication)GetProcAddress( hAppAuto, "CreateKompasApplication" );	
        
        if ( pCreateKompasApplication )
          newKompasAPI = IDispatchPtr ( pCreateKompasApplication(), false /*AddRef*/); // Получаем интерфейс Application
        FreeLibrary( hAppAuto );  
      }
    }
  }
}

//-------------------------------------------------------------------------------
// Задать идентификатор ресурсов
// ---
unsigned int WINAPI LIBRARYID()
{
  return IDR_LIBID;
}

//-------------------------------------------------------------------------------
// Получить текущий документ
// ---
IDocument3DPtr GetCurrentDocument()
{
  reference rDoc = ::ksGetCurrentDocument( 3/*cPartOrAssemblyType*/ );
  IDocument3DPtr doc(::ksGet3dDocumentFromReference(rDoc), false/*AddRef*/);
  return doc;
}


//-------------------------------------------------------------------------------
// Описания используемых функций
// ---
void RunCommand( IDocument3DPtr & doc  );

//-----------------------------------------------------------------------------
// Выбрать 3D документ с диска
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
// Выбрать текстовый документ с диска
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
// Получить редактируемый компонент в документе.
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
        MessageT(_T("Ok0")); //срабатывает
        if (part7)
        {
            MessageT(_T("Ok1")); //не срабатывает
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
                        _variant_t saMatrixes; //  Элементы матрицы в виде одномерного массива из 16 элементов. Матрица имеет размер 4х4
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
// Создать выдавливание (Для тестирования работы с телами)
// ---
IEntityPtr CreateExtrusion( IPartPtr& part )
{ 
    
  IEntityPtr res = NULL;

  if (part)
  {
    double val = DEFAULT_VAL;

    // Ввод размера радиуса круга
    if ( ::ReadDoubleW(_bstr_t(::LoadStr(IDS_CIRCLE_RADIUS)), DEFAULT_VAL, MIN_VAL, MAX_VAL,  &val) )
    {
      // Эскиз операции выдавливания
      IEntityPtr entitySketch(part->NewEntity(o3d_sketch), false /*AddRef*/); // 

      if (entitySketch)
      {
        // Получить указатель на интерфейс параметров объектов и элементов
        ISketchDefinitionPtr sketchDefinition(IUnknownPtr(entitySketch->GetDefinition(), false /*AddRef*/));
        if (sketchDefinition)
        {
          // Получим интерфейс базовой плоскости XOY
          IEntityPtr basePlane(part->GetDefaultEntity(o3d_planeXOY), false /*AddRef*/);
          
          // Установка параметров эскиза
          sketchDefinition->SetPlane(basePlane); // Установим плоскость XOY базовой для эскиза
          sketchDefinition->SetAngle(0);      // Угол поворота эскиза

          entitySketch->Create(); // Создадим эскиз

          // Войти в режим редактирования эскиза
          if (sketchDefinition->BeginEdit())
          {
            // Построим новый эскиз - квадрат
            /*::LineSeg( .0, .0, val, .0, 1 );
            ::LineSeg( val, .0, val, val, 1 );
            ::LineSeg( val, val, .0, val, 1 );
            ::LineSeg( .0, val, .0, .0, 1 );*/
            ::Circle(200, .0, val, 1);
            // Выйти из режима редактирования эскиза
            sketchDefinition->EndEdit();
          }

          // Оперция выдавливания
          IEntityPtr entityExtrusion(part->NewEntity(o3d_bossExtrusion), false /*AddRef*/);

          if (entityExtrusion)
          {
            // Интерфейс базовой операции выдавливания
            IBossExtrusionDefinitionPtr extrusionDefinition(IUnknownPtr(entityExtrusion->GetDefinition(), false/*AddRef*/));
            if (extrusionDefinition)
            {
              // Установка параметров операции выдавливания
              extrusionDefinition->SetDirectionType(dtNormal);
              extrusionDefinition->SetSideParam(true, etBlind, 200.0, .0, false); // Высота выдавливания

              // extrusionDefinition->SetThinParam(true, dtBoth, 5.0, 5.0); // Параметры тонкой стенки
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
// Операция вращения
// ---
void OperationRotated(IPartPtr& part , double angle)
{
    
    // Создадим новый эскиз
    IEntityPtr entitySketch(part->NewEntity(o3d_sketch), false/*AddRef*/);
    
    if (entitySketch)
    {
        // Получить указатель на интерфейс параметров объектов и элементов
        // Интерфейс свойств эскиза
        ISketchDefinitionPtr sketchDefinition(IUnknownPtr(entitySketch->GetDefinition(), false /*AddRef*/));
        if (sketchDefinition)
        {
            // Получим интерфейс базовой плоскости XOY
            IEntityPtr basePlane(part->GetDefaultEntity(o3d_planeYOZ), false /*AddRef*/);
            // Установка параметров эскиза
            sketchDefinition->SetPlane(basePlane); // Установим плоскость XOY базовой для эскиза
            sketchDefinition->SetAngle(-90);

            // Создадим эскиз
            entitySketch->Create();

            // Войти в режим редактирования эскиза
            if (sketchDefinition->BeginEdit())
            {
                // Введем новый эскиз
                //ArcByAngle(0, 0, 20, -90, 90, 1, 1);
                Circle(0, 200, 50, 1);
                
                LineSeg(-10, 0, 0, 0, 3);
                // Выйти из режима редактирования эскиза
                sketchDefinition->EndEdit();
            }
            // Операции вращения
            IEntityPtr entityRotate(part->NewEntity(o3d_bossRotated), false/*AddRef*/);
            if (entityRotate)
            {
                // Получить указатель на интерфейс параметров объектов и элементов
                // Интерфейс базовой операции вращения
                IBossRotatedDefinitionPtr bossRotatedDefinition(IUnknownPtr(entityRotate->GetDefinition(), false/*AddRef*/));
                if (bossRotatedDefinition)
                {
                    bossRotatedDefinition->SetToroidShapeType(true);      // Признак тороида ( TRUE - тороид, FALSE - сфероид )
                    bossRotatedDefinition->SetDirectionType(dtReverse);       // Направление вращения ( dtNormal - прямое направление, для тонкой стенки - наружу
                    // dtReverse - обратное направление, для тонкой стенки - внутрь, dtBoth - в обе стороны,
                    // dtMiddlePlane - от средней плоскости )
// Изменить параметры тонкой стенки
                    //bossRotatedDefinition->SetThinParam(true,               // Признак тонкостенной операции
                    //    dtNormal,             // Направление построения тонкой стенки
                    //    1,                  // Толщина стенки в прямом направлении
                    //    1);                // Толщина стенки в обратном направлении
                    // Изменить параметры выдавливания в одном направлении
                    //baseRotatedDefinition->SetSideParam(true,               // Направление вращения ( TRUE - прямое, FALSE - обратное )
                    //    180);              // Угол вращения
                    // Изменить параметры выдавливания в одном направлении
                    bossRotatedDefinition->SetSideParam(false,              // Направление вращения ( TRUE - прямое, FALSE - обратное )
                        angle);              // Угол вращения
                    bossRotatedDefinition->SetSketch(entitySketch);        // Эскиз операции выдавливания                                                                 
                    // Создать операцию              
                    if (entityRotate->Create()) {
                        MessageT(_T("Базовая операция вращения"));
                    }
                }
            }
        }
    }
}



//-----------------------------------------------------------------------------
// Получить тело с операции
// ---
IBodyPtr GetBodyFromOperation( IEntityPtr obj )
{
  IBodyCollectionPtr coll( obj ? obj->BodyCollection() : NULL, false/*AddRef*/ );
  IBodyPtr body( coll && coll->GetCount() == 1 ? coll->GetByIndex( 0 ) : NULL, false/*AddRef*/ );

  return body;
}

//-----------------------------------------------------------------------------
// Получить коллекцию по типу
// ---
IEntityCollectionPtr GetCollectionFromPart( IPartPtr part, Obj3dType type )
{
  IEntityCollectionPtr entityColl( part ? part->EntityCollection( o3d_conjunctivePoint ) : NULL, false/*AddRef*/ );
  return entityColl;
}

//-------------------------------------------------------------------------------
// Головная функция библиотеки
// ---
void WINAPI LIBRARYENTRY( unsigned int comm )
{
    reference file;
	// Получить текущий документ
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
// Завершение приложения
// ---
void OnProcessDetach()
{
  newKompasAPI = NULL;
}

//------------------------------------------------------------------------------
// Загрузить строку из ресурса
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
  // Тестируемый код
  IPartPtr part(GetWorkPart(doc));

  if ( doc )
  {
    ::CreateExtrusion(part);
    ::OperationRotated(part, 90);
    ::CreateLocalCS(doc);
  }
  
}


