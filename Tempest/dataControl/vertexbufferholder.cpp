#include "vertexbufferholder.h"

#include <Tempest/Texture2d>
#include <Tempest/Device>

#include <unordered_map>
#include <iostream>

#include <cstring>

using namespace Tempest;

/// \cond HIDDEN_SYMBOLS
struct VertexBufferHolder::Data {
  struct LDData{
    int vsize, size;

    int lockBegin, lockSize;
    std::vector< char > data;
    // const void * data;
    };

  LDData noData;
  std::unordered_map< AbstractAPI::VertexBuffer*, LDData* > vbos, restore;
  typedef std::unordered_map< AbstractAPI::VertexBuffer*, LDData* >::iterator Iterator;
  };
/// \endcond

VertexBufferHolder::VertexBufferHolder( Device& d):BaseType(d) {
  data = new Data();
  }

VertexBufferHolder::~VertexBufferHolder(){
  delete data;
  }

void VertexBufferHolder::createObject( AbstractAPI::VertexBuffer*& t,
                                       const char * src,
                                       int size, int vsize,
                                       AbstractAPI::BufferFlag flg ){
  t = allocBuffer( size, vsize, src, flg );

  if( !t )
    return;

  Data::LDData *d = 0;

  if( (flg & AbstractAPI::BF_NoReadback) && device().hasManagedStorge() ){
    d = &data->noData;
    d->vsize = 0;
    d->size  = 0;
    } else {
    d = new Data::LDData();
    d->vsize = vsize;
    d->size  = size;
    d->data.resize( size*vsize );
    std::copy( src, src + size*vsize, d->data.begin() );
    }

  d->lockBegin = 0;
  d->lockSize  = 0;
  data->vbos[t] = d;
  }

AbstractAPI::VertexBuffer* VertexBufferHolder::allocBuffer( size_t size,
                                                            size_t vsize,
                                                            const void *src,
                                                            AbstractAPI::BufferFlag flg ){
  return allocBuffer( size, vsize, src,
                      AbstractAPI::BU_Static, flg );
  }

AbstractAPI::VertexBuffer *VertexBufferHolder::allocBuffer( size_t size,
                                                            size_t vsize,
                                                            const void *src,
                                                            AbstractAPI::BufferUsage u,
                                                            AbstractAPI::BufferFlag /*flg*/) {
  return device().createVertexbuffer( size, vsize, src, u );
  }

void VertexBufferHolder::deleteObject( AbstractAPI::VertexBuffer* t ){
  if( t ){
    Data::Iterator i = data->vbos.find(t);
    if( i->second!=&data->noData )
      delete i->second;

    data->vbos.erase(i);
    device().deleteVertexBuffer(t);
    }
  }

void VertexBufferHolder::reset( AbstractAPI::VertexBuffer* t ){
  if( t ){
    Data::Iterator i = data->vbos.find(t);
    data->restore[t] = i->second;

    data->vbos.erase(i);
    device().deleteVertexBuffer(t);
    }
  }

AbstractAPI::VertexBuffer* VertexBufferHolder::restore( AbstractAPI::VertexBuffer* t ){
  if( t ){
    Data::LDData* ld = data->restore[t];
    data->restore.erase(t);

    createObject( t, &ld->data[0], ld->size, ld->vsize );
    if( ld!=&data->noData )
      delete ld;
    }

  return t;
  }

AbstractAPI::VertexBuffer* VertexBufferHolder::copy( AbstractAPI::VertexBuffer* t ){
  if( t ){
    Data::LDData& ld = *data->vbos[t];

    AbstractAPI::VertexBuffer* ret = 0;
    createObject( ret, &ld.data[0], ld.size, ld.vsize );
    return ret;
    }

  return 0;
  }

char *VertexBufferHolder::lockBuffer( AbstractAPI::VertexBuffer *t,
                                      int b, int sz ) {
  if( t==0 )
    return 0;

  Data::LDData & ld = *data->vbos[t];
  ld.lockBegin = b;
  ld.lockSize  = sz;

  return &ld.data[0];//(char*)device().lockBuffer( t, b, sz);
  }

char *VertexBufferHolder::bufferData(AbstractAPI::VertexBuffer *t) {
  if( t==0 )
    return 0;

  Data::LDData & ld = *data->vbos[t];
  return ld.data.data();
  }

void VertexBufferHolder::unlockBuffer(AbstractAPI::VertexBuffer *t) {
  if( t==0 )
    return;

  Data::LDData & ld = *data->vbos[t];
  device().updateBuffer(t,&ld.data[ld.lockBegin],ld.lockBegin,ld.lockSize);
  }

void VertexBufferHolder::updateBuffer(AbstractAPI::VertexBuffer* t, const void* data, int b, int sz) {
  device().updateBuffer(t,data,b,sz);
  }

void VertexBufferHolder::unlockNWBuffer(AbstractAPI::VertexBuffer *) {
  }
