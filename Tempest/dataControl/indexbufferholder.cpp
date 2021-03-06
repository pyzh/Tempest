#include "indexbufferholder.h"

#include <Tempest/Texture2d>
#include <Tempest/Device>

#include <unordered_map>

#include <cstring>

using namespace Tempest;

/// \cond HIDDEN_SYMBOLS
struct IndexBufferHolder::Data {
  struct LDData{
    int vsize, size;

    int lockBegin, lockSize;
    std::vector< char > data;
    // const void * data;
    };

  LDData noData;
  std::unordered_map< AbstractAPI::IndexBuffer*, LDData* > ibos, restore;
  typedef std::unordered_map< AbstractAPI::IndexBuffer*, LDData* >::iterator Iterator;
  };
/// \endcond

IndexBufferHolder::IndexBufferHolder( Device& d):BaseType(d) {
  data = new Data();
  }

IndexBufferHolder::~IndexBufferHolder(){
  delete data;
  }

IndexBufferHolder::IndexBufferHolder( const IndexBufferHolder& h)
                   :BaseType( h.device() ) {
  data = new Data();
  }

void IndexBufferHolder::createObject( AbstractAPI::IndexBuffer*& t,
                                      const char * src,
                                      int size,
                                      int vsize,
                                      AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags ){
  t = allocBuffer( size, vsize, src );

  if( !t )
    return;

  Data::LDData *d = 0;

  //d.data  = src;

  if( (flg & AbstractAPI::BF_NoReadback) && device().hasManagedStorge() ){
    d = &data->noData;
    d->vsize = 0;
    d->size  = 0;
    } else {
    d = new Data::LDData;
    d->vsize = vsize;
    d->size  = size;
    d->data.resize( size*vsize );
    std::copy( src, src + size*vsize, d->data.begin() );
    }

  data->ibos[t] = d;
  }

void IndexBufferHolder::deleteObject( AbstractAPI::IndexBuffer* t ){
  if( t ){
    Data::Iterator i = data->ibos.find(t);
    if( i->second!=&data->noData )
      delete i->second;

    data->ibos.erase(i);
    device().deleteIndexBuffer(t);
    }
  }

AbstractAPI::IndexBuffer *IndexBufferHolder::allocBuffer( size_t size,
                                                          size_t vsize,
                                                          const void *src,
                                                          AbstractAPI::BufferFlag flg ) {
  return allocBuffer( size, vsize, src,
                      AbstractAPI::BU_Static, flg );
  }

AbstractAPI::IndexBuffer *IndexBufferHolder::allocBuffer( size_t size,
                                                          size_t vsize,
                                                          const void *src,
                                                          AbstractAPI::BufferUsage u,
                                                          AbstractAPI::BufferFlag /*flg*/ ) {
  return device().createIndexBuffer( size, vsize, src, u );
  }

void IndexBufferHolder::reset( AbstractAPI::IndexBuffer* t ){
  if( t ){
    Data::Iterator i = data->ibos.find(t);
    data->restore[t] = i->second;

    data->ibos.erase(i);
    device().deleteIndexBuffer(t);
    }
  }

AbstractAPI::IndexBuffer* IndexBufferHolder::restore( AbstractAPI::IndexBuffer* t ){
  if( t ){
    Data::LDData* ld = data->restore[t];
    data->restore.erase(t);

    createObject( t, &ld->data[0], ld->size, ld->vsize );

    if( ld!=&data->noData )
      delete ld;
    }

  return t;
  }

AbstractAPI::IndexBuffer* IndexBufferHolder::copy( AbstractAPI::IndexBuffer* t ){
  if( t ){
    Data::LDData& ld = *data->ibos[t];

    AbstractAPI::IndexBuffer* ret = 0;
    createObject( ret, &ld.data[0], ld.size, ld.vsize );
    return ret;
    }
  return 0;
  }

char *IndexBufferHolder::lockBuffer( AbstractAPI::IndexBuffer *t,
                                     int b, int sz ) {
  if( t==0 )
    return 0;

  Data::LDData & ld = *data->ibos[t];
  ld.lockBegin = b;
  ld.lockSize  = sz;

  return &ld.data[0];//(char*)device().lockBuffer( t, b, sz);
  }

void IndexBufferHolder::unlockBuffer(AbstractAPI::IndexBuffer *t) {
  if( t==0 )
    return;

  Data::LDData & ld = *data->ibos[t];
  device().updateBuffer(t,&ld.data[ld.lockBegin],ld.lockBegin,ld.lockSize);
  }

void IndexBufferHolder::updateBuffer(GraphicsSubsystem::IndexBuffer* t, const void* data, int b, int sz) {
  device().updateBuffer(t,data,b,sz);
  }

void IndexBufferHolder::unlockNWBuffer(AbstractAPI::IndexBuffer *) {
  }
