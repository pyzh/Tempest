#include "widget.h"

#include <Tempest/Layout>
#include <Tempest/Painter>
#include <Tempest/Shortcut>
#include <Tempest/Assert>

#include <algorithm>
#include <functional>

using namespace Tempest;

struct Widget::DeleteGuard{
  DeleteGuard( Widget* w):w(w){
    w->lockDelete();
    }

  ~DeleteGuard(){
    w->unlockDelete();
    }

  Widget *w;
  };

size_t Widget::count = 0;

Widget::Widget(ResourceContext *context):
  fpolicy(BackgroundFocus), rcontext(context), deleteLaterFlagGuard(0){
  ++Widget::count;

  parentLay    = 0;
  focus        = false;
  chFocus      = false;
  uscissor     = true;
  nToUpdate    = false;
  multiTouch   = false;

  deleteLaterFlag = false;

#ifdef __MOBILE_PLATFORM__
  mouseReleseReciver.reserve(8);
#else
  mouseReleseReciver.reserve(1);
#endif
  mouseLeaveReciver.reserve(1);
  wvisible = true;
  mlay = 0;
  setLayout( new Layout() );

  update();
  }

Widget::~Widget() {
  T_ASSERT_X( deleteLaterFlagGuard<=0, "bad time to delete, use deleteLater");
  deleteLaterFlagGuard = -1;

  if( parentLayout() )
    parentLayout()->take(this);

  for( size_t i=0; i<skuts.size(); ++i ){
    Shortcut *s = skuts[i];
    s->m.owner = 0;
    delete s;
    }

  onDestroy( this );
  delete mlay;

  --Widget::count;
  }

int Widget::x() const {
  return wrect.x;
  }

int Widget::y() const {
  return wrect.y;
  }

int Widget::w() const {
  return wrect.w;
  }

int Widget::h() const {
  return wrect.h;
  }

Point Widget::pos() const {
  return wrect.pos();
  }

Rect Widget::rect() const {
  return wrect;
  }

Size Widget::size() const {
  return wrect.size();
  }

void Widget::setPosition(int x, int y) {
  int ox = wrect.x, oy = wrect.y;

  if( ox==x && oy==y )
    return;

  wrect.x = x;
  wrect.y = y;
  onPositionChange(ox,oy);

  nToUpdate = false;
  update();
  }

void Widget::setPosition(const Point &p) {
  setPosition( p.x, p.y );
  }

void Widget::resize(int w, int h) {
  if(w<0)
    w=0;
  if(h<0)
    h=0;

  int ow = wrect.w, oh = wrect.h;

  if( ow==w && oh==h )
    return;

  nToUpdate &= wrect.w>0 && wrect.h>0;
  wrect.w = w;
  wrect.h = h;

  layout().applyLayout();
  onResize(ow,oh);

  SizeEvent e(w,h);
  resizeEvent( e );

  update();
  }

void Widget::resize(const Size &s) {
  resize( s.w, s.h );
  }

void Widget::setGeometry(const Rect &r) {
  if( r==wrect )
    return;

  int ox = wrect.x, oy = wrect.y, ow = wrect.w, oh = wrect.h;
  wrect = r;
  if(wrect.w<0)
    wrect.w = 0;
  if(wrect.h<0)
    wrect.h = 0;

  if( ox!=r.x && oy!=r.y )
    onPositionChange(ox,oy);

  if( ow!=r.w || oh!=r.h ){
    layout().applyLayout();
    onResize(ow,oh);
    SizeEvent e(w(),h());
    resizeEvent( e );
    }

  update();
  }

void Widget::setGeometry(int x, int y, int w, int h) {
  setGeometry( Rect(x,y,w,h) );
  }

void Widget::setMaximumSize(const Size &s) {
  if(sp.maxSize==s)
    return;

  sp.maxSize = s;

  if( owner() )
    owner()->layout().applyLayout();

  if( wrect.w > s.w || wrect.h > s.h )
    setGeometry( wrect.x, wrect.y, std::min(s.w, wrect.w), std::min(s.h, wrect.h) );
  }

void Widget::setMinimumSize(const Size &s) {
  if(sp.minSize==s)
    return;

  sp.minSize = s;
  if( owner() )
    owner()->layout().applyLayout();

  if( wrect.w < s.w || wrect.h < s.h )
    setGeometry( wrect.x, wrect.y, std::max(s.w, wrect.w), std::max(s.h, wrect.h) );
  }

void Widget::setMaximumSize(int w, int h) {
  setMaximumSize( Size(w,h) );
  }

void Widget::setMinimumSize(int w, int h) {
  setMinimumSize( Size(w,h) );
  }

Size Widget::minSize() const {
  return sizePolicy().minSize;
  }

Size Widget::maxSize() const {
  return sizePolicy().maxSize;
  }

const SizePolicy Widget::sizePolicy() const {
  return sp;
  }

void Widget::setSizePolicy(const SizePolicy &s) {
  sp = s;

  if( owner() )
    owner()->layout().applyLayout();
  }

void Widget::setSizePolicy(SizePolicyType f) {
  setSizePolicy(f,f);
  }

void Widget::setSizePolicy(SizePolicyType f0, SizePolicyType f1) {
  sp.typeH = f0;
  sp.typeV = f1;

  if( owner() )
    owner()->layout().applyLayout();
  }

void Widget::setSpacing(int s) {
  layout().setSpacing(s);
  }

int Widget::spacing() const {
  return layout().spacing();
  }

void Widget::setMargin(const Margin &m) {
  return layout().setMargin(m);
  }

void Widget::setMargin(int l, int r, int t, int b) {
  return layout().setMargin(l,r,t,b);
  }

const Margin &Widget::margin() const {
  return layout().margin();
  }

FocusPolicy Widget::focusPolicy() const {
  return fpolicy;
  }

void Widget::setFocusPolicy( FocusPolicy f ) {
  fpolicy = f;
  }

void Widget::setLayout(Orientation ori) {
  setLayout( new LinearLayout(ori) );
  }

void Widget::setLayout(Layout *l) {
  l->swap( mlay );

  delete mlay;
  mlay = l;
  l->rebind(this);

  l->applyLayout();
  }

Layout &Widget::layout() {
  return *mlay;
  }

const Layout &Widget::layout() const {
  return *mlay;
  }

Layout *Widget::parentLayout() {
  return parentLay;
  }

const Layout *Widget::parentLayout() const {
  return parentLay;
  }

Widget *Widget::owner() {
  if( parentLay )
    return parentLay->owner(); else
    return 0;
  }

const Widget *Widget::owner() const {
  if( parentLay )
    return parentLay->owner(); else
    return 0;
  }

bool Widget::hasFocus() const {
  return focus;
  }

bool Widget::hasChildFocus() const {
  return chFocus;
  }

void Widget::useScissor(bool u) {
  uscissor = u;
  }

bool Widget::isScissorUsed() const {
  return uscissor;
  }

Point Widget::mapToRoot(const Point &p) const {
  if( owner()==0 )
    return p;

  return owner()->mapToRoot( p + pos() );
  }

void Widget::paintEvent( PaintEvent &pe ) {
  if( pe.pass )
    return;

  nToUpdate = false;
  paintNested(pe);
  }

void Widget::multiPaintEvent(PaintEvent &pe) {
  paintNested(pe);
  }

Widget* Widget::impl_mouseMoveEvent(Widget *root, MouseEvent &e) {
  DeleteGuard guard(root);
  (void)guard;

  if( !root->isVisible() )
    return 0;

  for( size_t i=root->layout().widgets().size(); i>=1; --i ){
    Widget* w = root->layout().widgets()[i-1];
    DeleteGuard guard(w);
    (void)guard;

    if( !w->isScissorUsed() ||
        w->rect().contains( e.x, e.y, true ) ){
      MouseEvent et( e.x - w->x(),
                     e.y - w->y(),
                     e.button,
                     e.delta,
                     e.mouseID,
                     e.type() );
      et.ignore();

      Widget* deep = impl_mouseMoveEvent(w,et);

      if( w->mouseLeaveReciver.size() < size_t(et.mouseID+1) )
        w->mouseLeaveReciver.resize( et.mouseID+1 );
      Widget*& leave = w->mouseLeaveReciver[et.mouseID];
      if(leave!=deep){
        if(deep!=nullptr){
          DeleteGuard g(deep);
          (void)g;
          impl_leaveEvent(leave,e);
          leave = nullptr;
          impl_enterEvent(deep,e,true);
          leave = deep;
          if(leave->deleteLaterFlag)
            leave = nullptr;
          } else {
          impl_leaveEvent(leave,e);
          leave = nullptr;
          }
        }

      if( et.isAccepted() ){
        e.accept();
        if(w->deleteLaterFlag)
          return nullptr; else
          return w;
        }

      if( w->isVisible() && (et.mouseID==0 || w->multiTouch) ){
        et.accept();
        w->event(et);
        }

      if( et.isAccepted() ){
        e.accept();

        if(w->deleteLaterFlag)
          return nullptr; else
          return w;
        }
      }
    }

  e.ignore();
  return 0;
  }

void Widget::impl_leaveEvent(Widget *w, MouseEvent &e) {
  if(w==nullptr)
    return;

  MouseEvent l(e.x - w->x(),
               e.y - w->y(),
               e.button,
               e.delta,
               e.mouseID,
               Event::MouseLeave);
  l.accept();
  if(size_t(e.mouseID) < w->mouseLeaveReciver.size()) {
    impl_leaveEvent(w->mouseLeaveReciver[e.mouseID],l);
    w->mouseLeaveReciver[e.mouseID] = nullptr;
    }
  w->event(l);
  }

void Widget::impl_enterEvent(Widget *w, MouseEvent &e, bool emitEv) {
  if(w==nullptr)
    return;

  if(size_t(e.mouseID) < w->mouseLeaveReciver.size()) {
    MouseEvent l(e.x - w->x(),
                 e.y - w->y(),
                 e.button,
                 e.delta,
                 e.mouseID,
                 Event::MouseLeave);
    l.accept();
    impl_leaveEvent(w->mouseLeaveReciver[e.mouseID],l);
    w->mouseLeaveReciver[e.mouseID] = nullptr;
    }

  if(emitEv){
    MouseEvent x(e.x - w->x(),
                 e.y - w->y(),
                 e.button,
                 e.delta,
                 e.mouseID,
                 Event::MouseEnter);
    x.accept();
    w->event(x);
    }
  }

Widget* Widget::impl_mouseEvent( Tempest::MouseEvent & e,
                                 Widget* root,
                                 bool focus,
                                 bool mpress ){
  DeleteGuard guard(root);
  (void)guard;

  if( !root->isVisible() )
    return 0;

  for( size_t i=root->layout().widgets().size(); i>=1; --i ){
    Widget* w = root->layout().widgets()[i-1];
    DeleteGuard guard(w);
    (void)guard;

    if( !w->isScissorUsed() ||
        w->rect().contains( e.x, e.y, true ) ){
      MouseEvent et( e.x - w->x(),
                     e.y - w->y(),
                     e.button,
                     e.delta,
                     e.mouseID,
                     e.type() );
      et.ignore();

      impl_mouseEvent(et, w, focus, mpress);
      if( et.isAccepted() ){
        e.accept();
        if(w->deleteLaterFlag)
          return nullptr; else
          return w;
        }

      if( w->isVisible() && (et.mouseID==0 || w->multiTouch) ){
        et.accept();
        w->event(et);
        }

      if( et.isAccepted() ){
        e.accept();
        if( mpress )
          w->setupMouseReleasePtr(et.mouseID);

        if( focus && w->focusPolicy()==ClickFocus )
          w->setFocus(1);

        if(w->deleteLaterFlag)
          return nullptr; else
          return w;
        }
      }
    }

  e.ignore();
  return 0;
  }

void Widget::setupMouseReleasePtr(size_t mouseID) {
  Widget* w = this;
  while(w){
    if(w->deleteLaterFlag)
      return;
    Widget* root = w->owner();
    if(root){
      if( root->mouseReleseReciver.size() < size_t(mouseID+1) )
        root->mouseReleseReciver.resize( mouseID+1 );
      root->mouseReleseReciver[mouseID] = w;
      }
    w = root;
    }
  }

void Widget::detachMouseReleasePtr() {
  Widget* widget = this;

  while( widget->owner() ){
    bool hasNext = false;
    std::vector<Widget*> & mr = widget->owner()->mouseReleseReciver;
    for( size_t i=0; i<mr.size(); ++i )
      if( mr[i]==widget ){
        mr[i] = nullptr;
        hasNext = true;
        }
    if(!hasNext)
      return;
    widget = widget->owner();
    }
  }

void Widget::detachMouseLeavePtr() {
  Widget* widget = this;

  while( widget->owner() ){
    bool hasNext = false;
    std::vector<Widget*> & mr = widget->owner()->mouseLeaveReciver;
    for( size_t i=0; i<mr.size(); ++i )
      if( mr[i]==widget ){
        mr[i] = nullptr;
        hasNext = true;
        }
    if(!hasNext)
      return;
    widget = widget->owner();
    }
  }

void Widget::lockDelete() {
  if(deleteLaterFlagGuard>=0)
    ++deleteLaterFlagGuard;
  }

void Widget::unlockDelete() {
  if(deleteLaterFlagGuard>0)
    --deleteLaterFlagGuard;

  if( deleteLaterFlagGuard==0 && deleteLaterFlag ){
    deleteLaterFlagGuard = -1;
    delete this;
    }
  }

void Widget::mouseMoveEvent(MouseEvent &e){
  e.ignore();
  }

void Widget::mouseDragEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseDownEvent(MouseEvent &e){
  e.ignore();
  }

void Widget::mouseUpEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseEnterEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseLeaveEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseWheelEvent(MouseEvent &e){
  e.ignore();
  }

void Widget::keyDownEvent(KeyEvent &e){
  e.ignore();
  }

void Widget::keyUpEvent(KeyEvent &e) {
  e.ignore();
  }

void Widget::customEvent( CustomEvent &e ) {
  e.ignore();
  }

void Widget::closeEvent(CloseEvent &e) {
  e.ignore();
  }

void Widget::update() {
  if( nToUpdate || wvisible==false )
    return;

  nToUpdate = wrect.w>0 && wrect.h>0;
  intentToUpdate();

  if( owner() )
    owner()->update();
  }

bool Widget::needToUpdate() const {
  return nToUpdate;
  }

void Widget::impl_keyPressEvent(Widget *wd, KeyEvent &e) {
  DeleteGuard guard(wd);
  (void)guard;

  const std::vector<Widget*> & w = wd->layout().widgets();

  for( size_t i=w.size(); i>0; --i ){
    if( w[i-1]->hasFocus() ){
      e.accept();
      w[i-1]->event(e);
      if( e.isAccepted() )
        return;
      }

    if( w[i-1]->chFocus ){
      impl_keyPressEvent(w[i-1], e);
      }
    }

  // e.ignore();
  }

void Widget::impl_customEvent( Widget*w, CustomEvent &e ) {
  DeleteGuard guard(w);
  (void)guard;

  size_t sz = w->layout().widgets().size();
  for( size_t i=0; i<sz; ++i ){
    Widget *wx = w->layout().widgets()[sz-i-1];
    wx->event(e);
    impl_customEvent( wx, e );
    }
  }

void Widget::impl_closeEvent(Widget *w, CloseEvent &e) {
  DeleteGuard guard(w);
  (void)guard;

  size_t sz = w->layout().widgets().size();
  for( size_t i=0; i<sz; ++i ){
    Widget *wx = w->layout().widgets()[sz-i-1];

    impl_closeEvent( wx, e );
    if( e.isAccepted() )
      return;
    }

  e.accept();
  w->event(e);
  }

void Widget::impl_gestureEvent(Widget *w, AbstractGestureEvent &e) {
  DeleteGuard guard(w);
  (void)guard;

  size_t sz = w->layout().widgets().size();
  for( size_t i=0; i<sz; ++i ){
    Widget *wx = w->layout().widgets()[sz-i-1];

    if( wx->isVisible() &&
        (!w->isScissorUsed() || wx->rect().contains(e.hotSpot())) ){
      Point h = e.hotSpot();
      e.setHotSpot( h - wx->pos() );
      impl_gestureEvent( wx, e );
      e.setHotSpot( h );

      if( e.isAccepted() )
        return;
      }
    }

  if( !e.isAccepted() ){
    e.accept();
    w->event(e);
    }
  }

void Widget::paintNested( PaintEvent &p ){
  Rect scissor = p.painter.scissor();

  const std::vector<Widget*> & w = layout().widgets();
  Rect s;

  for( size_t i=0; i<w.size(); ++i ){
    Widget *wi = w[i];
    if( wi->wvisible &&
        !(wi->uscissor &&
          p.painter.scissor().intersected(wi->rect()).isEmpty()) ){
      Tempest::Point pt = wi->pos();

      bool uscis = wi->uscissor;
      if( uscis ){
        s = scissor.intersected( wi->wrect );
        p.painter.setScissor( s );
        }

      wi->nToUpdate = false;
      if( !( uscis && s.isEmpty() ) ){
        p.painter.translate(  pt.x,  pt.y );
        wi->event(p);
        p.painter.translate( -pt.x, -pt.y );
        }

      if( uscis ){
        p.painter.setScissor( scissor );
        }
      }
    }
  }

void Widget::rootMouseDownEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();

  if( mouseReleseReciver.size() < size_t(e.mouseID+1) )
    mouseReleseReciver.resize( e.mouseID+1 );

  mouseReleseReciver[e.mouseID] = impl_mouseEvent( e,
                                                   this,
                                                   true,
                                                   true );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch())){
    e.accept();
    this->event(e);
    if(e.isAccepted() && this->focusPolicy()==ClickFocus)
      this->setFocus(1);
    }
  }

void Widget::rootMouseDragEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();

  if( size_t(e.mouseID) < mouseReleseReciver.size() && mouseReleseReciver[e.mouseID] )
    impl_mouseDragEvent( this, e );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->event(e);
    }
  }

void Widget::rootMouseUpEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();

  if( size_t(e.mouseID) < mouseReleseReciver.size() && mouseReleseReciver[e.mouseID] )
    impl_mouseUpEvent( this, e );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->event(e);
    }
  }

void Widget::impl_mouseDragEvent( Widget* w, Tempest::MouseEvent & e ){
  DeleteGuard guard(w);
  (void)guard;

  if( !( size_t(e.mouseID) < w->mouseReleseReciver.size() && w->mouseReleseReciver[e.mouseID]) ){
    w->event(e);
    return;
    }

  Widget *rec = 0;
  if( size_t(e.mouseID) < w->mouseReleseReciver.size() )
    rec = w->mouseReleseReciver[e.mouseID];

  if( std::find( w->layout().widgets().begin(),
                 w->layout().widgets().end(),
                 rec )
      !=w->layout().widgets().end() ){
    Widget * r = rec;
    Tempest::MouseEvent ex( e.x - r->x(), e.y - r->y(),
                            e.button, e.delta, e.mouseID,
                            e.type() );

    impl_mouseDragEvent( r, ex);

    if( ex.isAccepted() )
      e.accept();
    }
  }

void Widget::rootMouseMoveEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();  
  if( mouseLeaveReciver.size() < size_t(e.mouseID+1) )
    mouseLeaveReciver.resize( e.mouseID+1 );
  impl_mouseMoveEvent( this, e );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->event(e);
    }
  }

void Widget::rootMouseWheelEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();
  impl_mouseEvent( e, this, true, false );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->event(e);
    if(e.isAccepted() && this->focusPolicy()==ClickFocus)
      this->setFocus(1);
    }
  }

void Widget::rootKeyDownEvent(KeyEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  if( chFocus && !focus ){
    e.accept();
    impl_keyPressEvent( this, e );
    } else {
    e.ignore();
    }

  if( !e.isAccepted() ){
    e.accept();
    this->event(e);
    }
  }

void Widget::rootKeyUpEvent(KeyEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  if( chFocus && !focus ){
    e.accept();
    impl_keyPressEvent( this, e );
    } else {
    e.ignore();
    }

  if( !e.isAccepted() ){
    e.accept();
    this->event(e);
    }
  }

void Widget::rootShortcutEvent(KeyEvent &e) {
  e.ignore();
  this->shortcutEvent(e);
  }

void Widget::rootCloseEvent(CloseEvent &e) {
  impl_closeEvent(this, e);
  }

void Widget::rootGestureEvent(AbstractGestureEvent &e) {
  e.ignore();
  impl_gestureEvent(this, e);
  }

void Widget::rootCustomEvent(CustomEvent &e) {
  e.ignore();
  impl_customEvent(this, e);
  }

void Widget::impl_mouseUpEvent( Widget* w, Tempest::MouseEvent & e ){
  DeleteGuard guard(w);
  (void)guard;

  if( !( size_t(e.mouseID) < w->mouseReleseReciver.size() &&
         w->mouseReleseReciver[e.mouseID]) ){
    w->mouseUpEvent(e);
    return;
    }

  Widget * rec = 0;
  if( size_t(e.mouseID) < w->mouseReleseReciver.size() ){
    rec = w->mouseReleseReciver[e.mouseID];
    if( rec )
      w->mouseReleseReciver[e.mouseID] = 0;
    }

  if( rec ){
    Widget * r = rec;
    Tempest::MouseEvent ex( e.x - r->x(), e.y - r->y(), e.button, e.delta, e.mouseID, e.type() );

    impl_mouseUpEvent( r, ex );

    if( ex.isAccepted() )
      e.accept(); else
      e.ignore();
    }
  }

Widget* Widget::findRoot(){
  Widget* root = this;

  while( root->owner() )
    root = root->owner();

  return root;
  }

bool Widget::isVisible() const {
  return wvisible;
  }

void Widget::setVisible(bool v) {
  if( wvisible==v )
    return;

  wvisible = v;
  if( !wvisible )
    nToUpdate = false;

  if( owner() )
    owner()->layout().applyLayout();

  update();
  }

ResourceContext *Widget::context() const {
  return rcontext;
  }

void Widget::setContext(ResourceContext *context) {
  rcontext = context;

  if( mlay )
    mlay->rebind(this);
  }

bool Widget::hasMultitouch() const {
  return multiTouch;
  }

void Widget::setMultiTouchTracking( bool m ) {
  multiTouch = m;
  }

void Widget::setFocus(bool f) {
  DeleteGuard g(this);
  (void)g;

  if( focus!=f ){
    if( f ){
      unsetChFocus( this, this );

      Widget * root = this, *proot = this;

      while( root && !root->chFocus ){
        if( root->hasFocus() ){
          root->focus = 0;
          root->onFocusChange( 0 );
          }

        if( !root->chFocus && root!=this ){
          root->chFocus = 1;
          root->onChildFocusChange(1);
          }

        if( root->owner() ){
          proot = root;
          root  = root->owner();
          } else {
          break;
          }
        }

      if( root && root->chFocus ){
        DeleteGuard g(root);
        (void)g;

        for( size_t i=0; i<root->layout().widgets().size(); ++i ){
          if( root->layout().widgets()[i]!=proot )
            unsetChFocus( root->layout().widgets()[i], root );
          }
        }

      } else {
      Widget * root = this, *root_owner = root->owner();

      while( root_owner && root_owner->chFocus ){
        root = root_owner;

        DeleteGuard g(root);
        (void)g;

        root->chFocus = 0;
        root->onChildFocusChange(0);

        root_owner = root->owner();
        }
      }

    focus = f;
    onFocusChange(f);
    }
  }

void Widget::unsetChFocus( Widget* root, Widget* emiter ){  
  DeleteGuard g(root);
  (void)g;

  if( root!=emiter && root->focus ){
    root->focus = 0;
    root->onFocusChange(0);
    }

  if( root->chFocus ){
    root->chFocus = 0;
    root->onChildFocusChange(0);
    }

  const std::vector<Widget*>& lx = root->layout().widgets();
  for( size_t i=0; i<lx.size(); ++i )
    unsetChFocus( lx[i], emiter );
  }

void Widget::deleteLater() {
  if( deleteLaterFlagGuard==0 )
    delete this;
  else
    deleteLaterFlag = true;
  }

void Widget::shortcutEvent(KeyEvent &e) {
  if( !isVisible() ){
    e.ignore();
    return;
    }

  DeleteGuard g(this);
  (void)g;

  size_t sz = layout().widgets().size()-1;
  for( size_t i=0; i<layout().widgets().size(); ++i ){
    layout().widgets()[sz-i]->event(e);
    if( e.isAccepted() )
      return;
    }

  for( size_t i=0; i<skuts.size(); ++i )
    if( (skuts[i]->key()==e.key &&
         e.key != KeyEvent::K_NoKey ) ||
        (e.key == KeyEvent::K_NoKey &&
         skuts[i]->lkey()==e.u16 &&
         e.u16!=0 )){
      e.accept();
      skuts[i]->activated();
      return;
      }

  e.ignore();
  }

void Widget::resizeEvent( SizeEvent &e ) {
  e.ignore();
  }

void Widget::gestureEvent(AbstractGestureEvent &e) {
  e.ignore();
  }

void Widget::event( Event &e ) {
  switch ( e.type() ) {
    case Event::NoEvent:
      break;

    case Event::MouseDown:
      mouseDownEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseUp:
      mouseUpEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseMove:
      mouseMoveEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseDrag:
      mouseDragEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseWheel:
      mouseWheelEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseEnter:
      mouseEnterEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseLeave:
      mouseLeaveEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::KeyDown:
      keyDownEvent( (Tempest::KeyEvent&)e );
      break;

    case Event::KeyUp:
      keyUpEvent( (Tempest::KeyEvent&)e );
      break;

    case Event::Resize:
      resizeEvent( (Tempest::SizeEvent&)e );
      break;

    case Event::Shortcut:
      shortcutEvent( (Tempest::KeyEvent&)e );
      break;

    case Event::Paint:{
      Tempest::PaintEvent& pe = (Tempest::PaintEvent&)e;
      if( pe.pass==0 )
        paintEvent( pe ); else
        multiPaintEvent( pe );
      }
      break;

    case Event::Close:
      closeEvent( (Tempest::CloseEvent&)e );
      break;

    case Event::Gesture:
      gestureEvent( (Tempest::AbstractGestureEvent&)e );
      break;

    case Event::Custom:
      customEvent( (Tempest::CustomEvent&)e );
      break;
    }
  }

