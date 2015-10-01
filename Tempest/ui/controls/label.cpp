#include "label.h"

#include <Tempest/Application>

using namespace Tempest;

Label::Label() {
  resize(100,27);

  const UiMetrics& uiMetrics = Application::uiMetrics();
  fnt = Application::mainFont();
  fnt.setSize( int(uiMetrics.normalTextSize*uiMetrics.uiScale) );

  SizePolicy p = sizePolicy();
  p.maxSize.h = fnt.size() + fnt.size()/2;
  p.minSize.h = p.maxSize.h;
  p.typeV = FixedMax;

  setSizePolicy(p);
  }

void Label::setFont(const Font &f) {
  fnt = f;
  }

const Font &Label::font() const {
  return fnt;
  }

void Label::setText( const std::string &t ) {
  std::u16string s;
  s.assign( t.begin(), t.end() );
  setText( s );
  }

void Label::setText(const std::u16string &t) {
  txt = t;
  }

void Label::paintEvent(PaintEvent &e) {
  Painter p(e);

  p.setFont( fnt );

  const Margin& m = margin();

  p.setColor(1,1,1,1);
  int dY = (h()-fnt.size()-m.yMargin())/2;
  Rect sc = p.scissor();
  p.setScissor(sc.intersected(Rect(m.left, 0, w()-m.xMargin(), h())));
  p.drawText( m.left, m.top+dY, w()-m.xMargin(), fnt.size(),
              txt, AlignBottom );
  }

const std::u16string &Label::text() const {
  return txt;
  }

