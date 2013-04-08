#include "textureholder.h"

#include <Tempest/Texture2d>
#include <Tempest/Device>
#include <Tempest/Pixmap>

#include <map>
#include <cassert>

#include <stdexcept>

using namespace Tempest;

struct TextureHolder::Data {
  //std::map< AbstractAPI::Texture*, std::string > textures;

  struct DynTexture{
    int w, h, mip;
    TextureUsage usage;
    AbstractTexture::Format::Type format;
    };

  struct PixmapTexture{
    Pixmap pm;
    bool mip, compress;
    AbstractAPI::Texture* owner;
    };

  std::map< AbstractAPI::Texture*, DynTexture   > dynamic_textures;
  //std::map< AbstractAPI::Texture*, PixmapTexture> pixmap_textures;
  std::vector<PixmapTexture> pixmap_textures;
  };

TextureHolder::TextureHolder( Device& d):BaseType(d) {
  data = new Data();
  }

TextureHolder::~TextureHolder(){
  delete data;
  }

TextureHolder::TextureHolder( const TextureHolder& h):BaseType( h.device() ) {
  }


Tempest::Texture2d TextureHolder::create( int w, int h,
                                          AbstractTexture::Format::Type f,
                                          TextureUsage u ){
  Tempest::Texture2d obj( *this );
  w = std::max(w,0);
  h = std::max(h,0);

  if( !(w==0 || h==0) )
    createObject( obj.data.value(), w, h, 1, f, u ); else
    obj.data.value() = 0;
  obj.w   = w;
  obj.h   = h;
  obj.frm = f;

  return obj;
  }

Texture2d TextureHolder::create( const Pixmap &p, bool mips, bool compress ) {
  Tempest::Texture2d obj( *this );

  if( !(p.width()==0 || p.height()==0) )
    createObject( obj.data.value(), p, mips, compress ); else
    obj.data.value() = 0;

  obj.w = p.width();
  obj.h = p.height();

  if( p.hasAlpha() )
    obj.frm = Texture2d::Format::RGBA; else
    obj.frm = Texture2d::Format::RGB;

  return obj;
  }

Tempest::Texture2d TextureHolder::load( const std::string & fname ){
  Pixmap p;
  p.load(fname);
  return create(p, true);
  }

Tempest::Texture2d TextureHolder::load( const char* fname ){
  Pixmap p;
  p.load(fname);
  return create(p, true);
  }

void TextureHolder::createObject( AbstractAPI::Texture*& t,
                                  const std::string &fname ){
  Pixmap p;
  p.load(fname);
  createObject( t, p, true, true );
  }

void TextureHolder::createObject( AbstractAPI::Texture*& t,
                                  int w, int h, int mip,
                                  AbstractTexture::Format::Type f,
                                  TextureUsage u ){
  t = device().createTexture( w, h, mip, f, u );

  if( !t )
    return;

  Data::DynTexture d;
  d.w      = w;
  d.h      = h;
  d.mip    = mip;
  d.usage  = u;
  d.format = f;

  if( !device().hasManagedStorge() )
    data->dynamic_textures[t] = d;
  }

void TextureHolder::createObject( AbstractAPI::Texture *&t,
                                  const Pixmap &p,
                                  bool mips,
                                  bool compress ){
  Data::PixmapTexture px;
  px.pm = p;
  px.mip = mips;
  px.compress = compress;

  t = device().createTexture( p, mips, compress );
  // assert(t);

  px.owner = t;
  //data->pixmap_textures[t] = px;
  if( !device().hasManagedStorge() )
    data->pixmap_textures.push_back(px);
  }

void TextureHolder::recreateObject( AbstractAPI::Texture *&t,
                                    AbstractAPI::Texture * old,
                                    const Pixmap &p,
                                    bool mips,
                                    bool compress ) {
  if( !device().hasManagedStorge() ){
    if( data->dynamic_textures.find(old) != data->dynamic_textures.end() )
      data->dynamic_textures.erase(old);

    for( size_t i=0; i<data->pixmap_textures.size(); ++i )
      if( data->pixmap_textures[i].owner==old ){
        data->pixmap_textures[i] = data->pixmap_textures.back();
        data->pixmap_textures.pop_back();
        i = -1;
        }
    }

  Data::PixmapTexture px;
  px.pm = p;
  px.mip = mips;
  px.compress = compress;

  t = device().recreateTexture( old, p, mips, compress );
  px.owner = t;
  //data->pixmap_textures[t] = px;
  if( !device().hasManagedStorge() )
    data->pixmap_textures.push_back(px);
  }

void TextureHolder::deleteObject( AbstractAPI::Texture* t ){
  if( !device().hasManagedStorge() ){
    if( data->dynamic_textures.find(t) != data->dynamic_textures.end() )
      data->dynamic_textures.erase(t);

    for( size_t i=0; i<data->pixmap_textures.size(); ++i )
      if( data->pixmap_textures[i].owner==t ){
        data->pixmap_textures[i] = data->pixmap_textures.back();
        data->pixmap_textures.pop_back();
        i = -1;
        }
    }

  device().deleteTexture(t);
  }

void TextureHolder::reset( AbstractAPI::Texture* t ){
  if( !device().hasManagedStorge() )
    device().deleteTexture(t);
  }

AbstractAPI::Texture* TextureHolder::restore( AbstractAPI::Texture* t ){
  if( device().hasManagedStorge() ){
    return t;
    }

  if( data->dynamic_textures.find(t)!=data->dynamic_textures.end() ){
    Data::DynTexture d = data->dynamic_textures[t];
    data->dynamic_textures.erase(t);

    createObject( t, d.w, d.h, d.mip, d.format, d.usage );
    return t;
    }

  for( size_t i=0; i<data->pixmap_textures.size(); ++i )
    if( data->pixmap_textures[i].owner==t ){
      Data::PixmapTexture d = data->pixmap_textures[i];

      data->pixmap_textures[i] = data->pixmap_textures.back();
      data->pixmap_textures.pop_back();

      createObject( t, d.pm, d.mip, d.compress );
      return t;
      }

  return 0;
  }

AbstractAPI::Texture* TextureHolder::copy( AbstractAPI::Texture* ){
#ifndef __ANDROID__
  throw std::runtime_error("TextureHolder::copy not implemented yet");
#endif
  }
