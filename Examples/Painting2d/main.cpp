#include "mainwindow.h"

#include <Tempest/Application>
#include <Tempest/DirectX9>
#include <Tempest/DirectX11>
#include <Tempest/Opengl2x>
#include <Tempest/Opengl4x>

enum APIType{
  OpenGL,
  OpenGL4,
  Direct3D,
  Direct3D11
  };

Tempest::AbstractAPI& api( APIType a ){
  using namespace Tempest;

  switch( a ){
    case Direct3D:{
      static DirectX9 api;
      return api;
      }

    case Direct3D11:{
      static DirectX11 api;
      return api;
      }

    case OpenGL4:
      static Opengl4x api;
      return api;
      break;

    case OpenGL:
    default:{//avoid gcc 4.8 warning
      static Opengl2x api;
      return api;
      }
    }
  }

int main() {
  using namespace Tempest;
  Application app;

  MainWindow w( api(Direct3D) );
  w.show();

  return app.exec();
  }

