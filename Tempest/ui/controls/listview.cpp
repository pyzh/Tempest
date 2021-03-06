#include "listview.h"

using namespace Tempest;

ListView::ListView(Orientation ori)
  : orientation(ori), lay(nullptr) {
  if(orientation==Horizontal)
    setSizePolicy(FixedMin,Preferred);
  else
    setSizePolicy(Preferred,FixedMin);
  }

ListView::~ListView() {
  removeDelegate();
  }

void ListView::removeDelegate() {
  if(delegate){
    delegate->onItemSelected.ubind(onItemSelected);
    removeAll();
    lay = nullptr;
    setLayout(new Tempest::Layout());
    delegate.reset();
    }
  }

void ListView::invalidateView() {
  if(delegate)
    delegate->invalidateView();
  }

void ListView::updateView() {
  if(delegate)
    delegate->updateView();
  }

void ListView::setOrientation(Orientation ori) {
  orientation = ori;
  if(orientation==Horizontal)
    setSizePolicy(FixedMin,Preferred);
  else
    setSizePolicy(Preferred,FixedMin);

  if(delegate){
    lay = new Layout(*this,*delegate,orientation,defaultRole);
    setLayout(lay);
    }
  }

void ListView::setDefaultItemRole(ListDelegate::Role role) {
  if(defaultRole!=role) {
    defaultRole=role;
    invalidateView();
    }
  }

ListDelegate::Role ListView::defaultItemRole() const {
  return defaultRole;
  }

void ListView::removeAll() {
  if(lay)
    lay->removeAll(); else
    layout().removeAll();
  }


ListView::Layout::Layout( ListView &view,
                          ListDelegate &delegate,
                          Orientation ori,
                          ListDelegate::Role &defaultRole)
  :LinearLayout(ori),
    view(view), delegate(delegate),
    busy(false), defaultRole(defaultRole) {
  delegate.invalidateView.bind(this,&Layout::invalidate);
  delegate.updateView    .bind(this,&Layout::update    );
  }

ListView::Layout::~Layout(){
  delegate.invalidateView.ubind(this,&Layout::invalidate);
  delegate.updateView    .ubind(this,&Layout::update    );
  removeAll();
  }

void ListView::Layout::applyLayout() {
  if(busy)
    return;
  busy = true;

  int w=margin().xMargin(), h=margin().yMargin();

  size_t widgetsCount = delegate.size();
  if(widgets().size()!=widgetsCount){
    removeAll();
    if(orientation()==Horizontal){
      for(size_t i=0; i<widgetsCount; ++i){
        Widget*       view=delegate.createView(i,defaultRole);
        if( view->isVisible() ){
          const Size    sz  =view->minSize();
          w += sz.w;
          h = std::max(h,sz.h);
          }
        add(view);
        }
      } else {
      for(size_t i=0; i<widgetsCount; ++i){
        Widget*       view=delegate.createView(i,defaultRole);
        if( view->isVisible() ){
          const Size    sz  =view->minSize();
          w = std::max(w,sz.w);
          h += sz.h;
          }
        add(view);
        }
      }
    } else {
    const std::vector<Widget*>& wx = widgets();
    if(orientation()==Horizontal){
      for(size_t i=0; i<wx.size(); ++i){
        if( wx[i]->isVisible() ){
          const Size sz = wx[i]->minSize();
          w += sz.w;
          h = std::max(h,sz.h);
          }
        }
      } else {
      for(size_t i=0; i<wx.size(); ++i){
        if( wx[i]->isVisible() ){
          const Size sz = wx[i]->minSize();
          w = std::max(w,sz.w);
          h += sz.h;
          }
        }
      }
    }

  const int sp=summarySpacings();

  if(orientation()==Horizontal)
    owner()->setMinimumSize(w+sp, h); else
    owner()->setMinimumSize(w, h+sp);

  LinearLayout::applyLayout();
  busy = false;
  }

void ListView::Layout::invalidate(){
  removeAll();
  applyLayout();
  view.onItemListChanged();
  }

void ListView::Layout::update() {
  busy = true;
  for(size_t i=0;i<widgets().size(); ++i){
    Widget* w = take(widgets()[i]);
    add(delegate.update(w,i),i);
    }
  busy = false;
  applyLayout();
  view.onItemListChanged();
  }

void ListView::Layout::removeAll() {
  bool b = busy;
  busy = true;

  while(widgets().size()){
    size_t id = widgets().size()-1;
    delegate.removeView(take(widgets().back()), id);
    }

  busy = b;
  }

Tempest::Button *Tempest::ListView::createItemButton(Tempest::ListDelegate::Role /*r*/) {
  return new Button();
  }
