#ifndef PIXMAP_H
#define PIXMAP_H

#include <Tempest/CopyWhenNeedPtr>
#include <Tempest/Color>
#include "../utils/mempool.h"

#include <vector>
#include <string>

#include <Tempest/Utility>

namespace Tempest{

class IDevice;
class ODevice;

class Pixmap {
  public:
    Pixmap();
    Pixmap( const std::string& p );
    Pixmap( const char*        p );
    Pixmap( const std::u16string& p );
    Pixmap( const char16_t*       p );

    Pixmap( int w, int h, bool alpha );

    Pixmap( const Pixmap& p );
    Pixmap& operator = ( const Pixmap & p );

    bool load( const std::u16string & f );
    bool save( const std::u16string & f, const char *ext=nullptr ) const;

    bool load( const std::string & f );
    bool save( const std::string & f, const char *ext=nullptr ) const;

    bool load( const char* f );
    bool save( const char* f, const char *ext=nullptr ) const;

    bool load( const char16_t* f );
    bool save( const char16_t* f, const char *ext=nullptr ) const;

    bool load( IDevice& f );
    bool save( ODevice &f, const char *ext=nullptr) const;

    inline int width()  const { return info.w; }
    inline int height() const { return info.h; }

    inline Size size()  const { return Size(info.w, info.h); }

    inline bool isEmpty() const {
      return info.w==0||0==info.h;
      }

    inline int mipsCount() const{ return info.mipLevels; }

    void downsample();
    void toPOT( int maxSize );

    struct Pixel{
      uint8_t r,g,b,a;

      uint8_t* data(){
        return &r;
        }

      const uint8_t* data() const {
        return &r;
        }

      template< class T >
      void assign( T it ){
        r = uint8_t(*it); ++it;
        g = uint8_t(*it); ++it;
        b = uint8_t(*it); ++it;
        a = uint8_t(*it); ++it;
        }
      };

    inline const uint8_t* const_data() const {
      if(imgData.const_value()==nullptr ||
         imgData.const_value()->bytes.size()==0)
        return nullptr;
      return &imgData.const_value()->bytes[0];
      }

    inline void* data() {
      if(imgData.value()==nullptr ||
         imgData.value()->bytes.size()==0)
        return nullptr;
      return &imgData.value()->bytes[0];
      }

    void fill( const Pixel& p );

    inline Pixel at( int x, int y ) const {
      verifyFormatEditable();

      const uint8_t * p = &rawPtr[ (x + y*info.w)*info.bpp ];
      Pixel r;

      r.r = p[0];
      r.g = p[1];
      r.b = p[2];

      if( info.bpp==4 )
        r.a = p[3]; else
        r.a = 255;

      return r;
      }

    inline void  set(int x, int y, const Color & p ) {
      int px[4] = { int(p.r()*255),
                    int(p.g()*255),
                    int(p.b()*255),
                    int(p.a()*255) };
      Pixel pixel;
      pixel.assign(px);
      set(x,y, pixel);
      }

    inline void  set(int x, int y, const Pixel & p ) {
      setupRawPtrs();
      uint8_t * v = &mrawPtr[ (x + y*info.w)*info.bpp ];

      v[0] = p.r;
      v[1] = p.g;
      v[2] = p.b;

      if( info.bpp==4 )
        v[3] = p.a;
      }

    bool  hasAlpha() const;

    enum Format{
      Format_RGB,
      Format_RGBA,
      Format_DXT1,
      Format_DXT3,
      Format_DXT5,
      Format_ETC1_RGB8,

      Format_User = 1024
      };

    Format format() const{ return info.format; }
    void setFormat( Format f );

    struct ImgInfo{
      ImgInfo();
      int  w, h, bpp, mipLevels;
      bool alpha;
      Pixmap::Format format;
      };
  private:
    struct Data {
      std::vector<uint8_t> bytes;
      };

    ImgInfo info;

    struct MemPool;
    static MemPool pool;

    struct DbgManip{
      typedef Data* T;

      struct Ref{
        Ref( const T& t ):data(t), count(1){}

        T   data;
        Detail::atomic_counter count;
        Detail::Spin spin;
        };

      Ref * newRef();
      Ref * newRef( const Ref * base );
      void  delRef( Ref * r );

      bool isValid() const {
        return true;
        }

      static Tempest::MemPool<Ref> ref_pool;
      };

    mutable Detail::Ptr<Data*, DbgManip> imgData;
    mutable const uint8_t * rawPtr;
    mutable uint8_t       * mrawPtr;

    void verifyFormatEditable() const {
      if( info.format==Format_RGB || info.format==Format_RGBA )
        return;

      Pixmap* p = const_cast<Pixmap*>(this);
      p->makeEditable();
      }

    void  makeEditable();

    void  addAlpha();
    void  removeAlpha();

    void setupRawPtrs() const;

  friend class PixEditor;
  };

class PixEditor{
  public:
    PixEditor( Pixmap & out );

    void draw( int x, int y, const Pixmap &p );
    void copy( int x, int y, const Pixmap &p );
  private:
    Pixmap & out;
  };
}

#endif // PIXMAP_H
