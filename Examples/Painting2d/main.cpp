#include "mainwindow.h"

#include <Tempest/Application>
#ifdef __WINDOWS__
#include <Tempest/DirectX9>
#include <Tempest/DirectX11>
#include <Tempest/Opengl4x>
#endif
#include <Tempest/Opengl2x>


enum APIType{
  OpenGL,
  OpenGL4,
  Direct3D,
  Direct3D11
  };

Tempest::AbstractAPI& api( APIType a ){
  using namespace Tempest;

  static std::unique_ptr<Tempest::AbstractAPI> api;

  switch( a ){
#ifdef __WINDOWS__
    case Direct3D:
      api.reset(new DirectX9());
      break;

    case Direct3D11:
      api.reset(new DirectX11());
      break;

    case OpenGL4:
      api.reset(new Opengl4x());
      break;
#endif
    case OpenGL:
    default: //avoid gcc 4.8 warning
      api.reset(new Opengl2x());
      break;
    }

  return *api;
  }

int main() {
  using namespace Tempest;
  Application app;

  MainWindow w( api(Direct3D11) );
  w.show();

  return app.exec();
  }

