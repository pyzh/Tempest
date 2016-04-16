#include "layout.h"

using namespace Tempest;

Layout::Layout(){
  ow       = 0;
  mspacing = 0;
  }

Layout::~Layout() {
  removeAll();
  }

Widget *Layout::owner() {
  return ow;
  }

const Widget *Layout::owner() const {
  return ow;
  }

void Layout::add(Widget *widget, size_t pos) {
  T_ASSERT_X(widget!=nullptr,"widget is null");

  if(widget->parentLay==this)
    return;

  size_t hasReleaseReciver = -1, hasLeavePtr = -1;
  if(widget->parentLay){
    Widget* ow = widget->parentLay->owner();

    for(size_t i=0; i<ow->mouseReleseReciver.size(); ++i)
      if(widget==ow->mouseReleseReciver[i])
        hasReleaseReciver=i;

    for(size_t i=0; i<ow->mouseLeaveReciver.size(); ++i)
      if(widget==ow->mouseLeaveReciver[i])
        hasLeavePtr=i;

    widget->parentLay->take(widget);
    }

  if(pos>=w.size())
    pos=w.size();

  if( widget->hasFocus() || widget->hasChildFocus() )
    widget->unsetChFocus(widget, 0);

  w.insert(w.begin()+pos,widget);

  widget->parentLay = this;
  if( hasReleaseReciver!=size_t(-1) )
    widget->setupMouseReleasePtr(hasReleaseReciver);

  if( hasLeavePtr!=size_t(-1) )
    widget->setupMouseReleasePtr(hasLeavePtr);

  if( owner() )
    widget->setContext( owner()->context() );

  if( widget->nToUpdate ){
    widget->nToUpdate    = false;
    widget->update();
    }

  applyLayout();
  }

void Layout::add(Widget *widget) {
  add(widget,w.size());
  }

void Layout::del(Widget *widget) {
  if(widget->parentLay!=this)
    return;

  w.resize( std::remove( w.begin(), w.end(), widget ) - w.begin() );
  widget->deleteLater();
  widget->detachMouseReleasePtr();
  widget->detachMouseLeavePtr();

  applyLayout();

  if( owner() )
    owner()->update();
  }

void Tempest::Layout::removeAll() {
  std::vector<Widget*> wx;
  std::swap(wx,w);

  if( owner() ){
    Widget* w = owner();

    w->lockDelete();
    w->mouseReleseReciver.clear();
    w->update();

    std::vector<Widget*> lv = w->mouseLeaveReciver;
    for(size_t i=0;i<lv.size();++i){
      Widget*& wx = lv[i];
      while( wx && i<wx->mouseLeaveReciver.size() && wx->mouseLeaveReciver[i]!=nullptr )
        wx = wx->mouseLeaveReciver[i];
      }

    for(size_t i=0;i<lv.size();++i){
      Widget* wx = lv[i];
      if( wx )
        wx->lockDelete();
      }

    for(size_t i=0;i<lv.size();++i){
      Widget* wx = lv[i];
      MouseEvent e(0,0,Event::ButtonNone,0,i,Event::MouseLeave);
      if( wx )
        wx->event(e);
      }

    for(size_t i=0;i<lv.size();++i){
      Widget* wx = lv[i];
      if( wx )
        wx->unlockDelete();
      }

    w->mouseLeaveReciver.clear();
    w->unlockDelete();
    }

  for( size_t i=0; i<wx.size(); ++i ){
    wx[i]->parentLay = 0;
    wx[i]->deleteLater();
    }
  wx.clear();

  applyLayout();
  }

Widget *Layout::take(Widget *widget) {
  if(widget->parentLay!=this)
    return widget;

  if( widget->hasFocus() )
    widget->setFocus(0);

  widget->detachMouseReleasePtr();
  widget->detachMouseLeavePtr();
  w.resize( std::remove( w.begin(), w.end(), widget ) - w.begin() );
  widget->parentLay = 0;

  applyLayout();

  if( owner() ){
    owner()->update();
    }

  return widget;
  }

const std::vector<Widget*> &Layout::widgets() {
  return w;
  }

int Layout::spacing() const {
  return mspacing;
  }

void Layout::setMargin(const Margin &m){
  mmargin = m;
  applyLayout();
  }

void Layout::setMargin(int l, int r, int t, int b) {
  setMargin( Margin(l,r,t,b) );
  }

const Margin &Layout::margin() const {
  return mmargin;
  }

void Layout::placeIn(Widget *wx, int x, int y, int w, int h) {
  placeIn(wx, Rect(x,y,w,h) );
  }

void Layout::placeIn(Widget *wx, const Rect &r) {
  int w = std::max( wx->sizePolicy().minSize.w, r.w ),
      h = std::max( wx->sizePolicy().minSize.h, r.h );

  w = std::min( wx->sizePolicy().maxSize.w, w );
  h = std::min( wx->sizePolicy().maxSize.h, h );

  wx->setGeometry( r.x + (r.w-w)/2,
                   r.y + (r.h-h)/2,
                   w,
                   h );
  }

Size Layout::sizeHint(const Widget *wx) {
  Size sz = wx->sizePolicy().minSize;
  if( wx->sizePolicy().typeH==FixedMax )
    sz.w = wx->sizePolicy().maxSize.w;
  if( wx->sizePolicy().typeH==FixedMin)
    sz.w = wx->sizePolicy().minSize.w;

  if( wx->sizePolicy().typeV==FixedMax )
    sz.h = wx->sizePolicy().maxSize.h;
  if( wx->sizePolicy().typeV==FixedMin)
    sz.h = wx->sizePolicy().minSize.h;

  return sz;
  }

void Layout::rebind( Widget* wx) {
  ow = wx;

  if( ow ){
    for( size_t i=0; i<w.size(); ++i ){
      T_ASSERT_X(w[i]!=ow,"recursive layout detected");
      w[i]->setContext( ow->context() );
      }
    }
  }

void Layout::swap(Layout *other){
  if( other==0 ){
    for(Widget* wx:w)
      wx->deleteLater();
    w.clear();
    return;
    }

  size_t s[2] = { w.size(), other->w.size() };
  std::swap( mmargin, other->mmargin );

  w.resize( std::max(s[0], s[1]) );
  other->w.resize( w.size() );

  for( size_t i=0; i<w.size(); ++i ){
    std::swap( w[i], other->w[i] );
    }

  w.resize( s[1] );
  other->w.resize( s[0] );

  for( size_t i=0; i<w.size(); ++i )
    w[i]->parentLay = this;

  for( size_t i=0; i<other->w.size(); ++i )
    other->w[i]->parentLay = other;

  }

void Layout::setSpacing(int s){
  s = std::max(s,0);

  if( mspacing!=s ){
    mspacing = s;
    applyLayout();
    }
  }

void LinearLayout::applyLayoutPrivate( int (*w)( const Size& wd ),
                                       int (*h)( const Size& wd ),
                                       SizePolicyType (*typeH)( const Widget*  ),
                                       SizePolicyType (*typeV)( const Widget*  ),
                                       bool /*hor*/,
                                       void (*setGeometry)( Widget* wd,
                                                            int x, int y,
                                                            int w, int h ) ){
      if( this->widgets().size()==0 )
        return;

      int fixedSize    = 0,
          maxSpaceNeed = 0,
          minSpaceNeedByPref = 0,
          maxSpaceNeedByExp = 0,
          spacing = this->spacing()*(widgets().size()-1),

          prefCount = 0,
          exCount   = 0;

      const std::vector<Widget*>& wd = widgets();

      for( size_t i=0; i<wd.size(); ++i )
        if( wd[i]->isVisible() ){
          if( typeH(wd[i])==FixedMin )
            fixedSize += w(wd[i]->sizePolicy().minSize);
            else
          if( typeH(wd[i])==FixedMax )
            fixedSize += w(wd[i]->sizePolicy().maxSize);
            else {
            maxSpaceNeed += w(wd[i]->sizePolicy().maxSize);

            if( typeH(wd[i])==Expanding ){
              maxSpaceNeedByExp += w(wd[i]->sizePolicy().maxSize);
              ++exCount;
              }

            if( typeH(wd[i])==Preferred ){
              minSpaceNeedByPref += w(wd[i]->sizePolicy().minSize);
              ++prefCount;
              }
            }
          }

      spacing = std::max(spacing,
                         w( owner()->size() ) -
                         (maxSpaceNeed+fixedSize+
                          ((orientation()==Vertical)? margin().yMargin() : margin().xMargin()) ) );

      int x = margin().left,
          hmarg = margin().top  + margin().bottom,
          wmarg = margin().left + margin().right,
          marg  = margin().top,
          spaceEx = w(owner()->size()) - spacing - fixedSize - minSpaceNeedByPref,
          spacePref = minSpaceNeedByPref;

      if( orientation()==Vertical ){
        x     = margin().top;
        hmarg = margin().left + margin().right;
        wmarg = margin().top  + margin().bottom;
        marg  = margin().left;
        }

      spaceEx -= wmarg;
      if( spacePref < spaceEx-maxSpaceNeedByExp )
        spacePref = spaceEx-maxSpaceNeedByExp;

      if( exCount==0 ) // TESTME
        spacePref = w(owner()->size()) - spacing - fixedSize - wmarg;

      for( size_t i=0; i<wd.size(); ++i )
        if( wd[i]->isVisible() ){
          int sp = 0, hw = h(owner()->size()) - hmarg;
          if( wd.size()-i > 1 )
            sp = spacing/(wd.size()-i-1);

          if( typeV(wd[i])==FixedMin )
            hw = h(wd[i]->sizePolicy().minSize);else//-hmarg; else
          if( typeV(wd[i])==FixedMax )
            hw = h(wd[i]->sizePolicy().maxSize);//-hmarg;

          int y = marg+( h(owner()->size())-hw-hmarg )/2,
              ww = 0;

          if( typeH(wd[i])==FixedMin ){
            ww = w(wd[i]->sizePolicy().minSize);
            }

          if( typeH(wd[i])==FixedMax ){
            ww = w(wd[i]->sizePolicy().maxSize);
            }

          if( typeH(wd[i])==Preferred ){
            ww = spacePref/prefCount;
            ww = std::max(ww, w(wd[i]->sizePolicy().minSize));
            ww = std::min(ww, w(wd[i]->sizePolicy().maxSize));
            spacePref -= ww;
            --prefCount;
            }

          if( typeH(wd[i])==Expanding ){
            ww = spaceEx/exCount;
            ww = std::max(ww, w(wd[i]->sizePolicy().minSize));
            ww = std::min(ww, w(wd[i]->sizePolicy().maxSize));
            spaceEx -= ww;
            --exCount;
            }

          setGeometry( wd[i], x, y, ww, hw );
          x       += ww;
          x       += sp;
          spacing -= sp;
          }
    }


