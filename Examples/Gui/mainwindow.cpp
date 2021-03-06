#include "mainwindow.h"

#include <Tempest/Assert>
#include <Tempest/RenderState>
#include <Tempest/Painter>

#include <Tempest/Application>
#include <Tempest/Layout>
#include <Tempest/Button>
#include <Tempest/LineEdit>
#include <Tempest/Panel>
#include <Tempest/ListView>
#include <Tempest/ListBox>
#include <Tempest/ScrollWidget>
#include <Tempest/CheckBox>
#include <Tempest/Label>

using namespace Tempest;

MainWindow::MainWindow(Tempest::AbstractAPI &api)
   :device( api, handle() ),
    texHolder(device),
    vboHolder(device),
    iboHolder(device),
    shHolder (device),
    spHolder (texHolder),
    uiRender (shHolder),
    listBoxDelegate(listBoxItems),
    listViewDelegate(listViewItems) {
  Application::showHint.bind(*this,&MainWindow::setHint);
  Application::setMainFont(Font("data/arial",16));

  texture = texHolder.load("data/texture.png");

  for(int i=0; i<5; ++i )
    listBoxItems.emplace_back("item "+std::to_string(i));

  for(int i=0; i<16; ++i )
    listViewItems.push_back(1.0/(i+1.0));

  setupUi();
  }

void MainWindow::setupUi() {
  Panel *panel = new Panel();
  panel->setDragable(true);
  panel->setLayout(Vertical);

  Button* button = new Button();
  button->setText("Button");
  button->setHint("button hint");
  button->setIcon(Icon("data/icon.png",spHolder));
  button->onClicked.bind(this,&MainWindow::buttonClick);
  panel->layout().add(button);

  button = new CheckBox();
  button->setText("Checkbox");
  button->setHint("checkbox hint");
  button->setIcon(Icon("data/icon.png",spHolder));
  panel->layout().add(button);

  Label* label = new Label();
  label->setText("Label");
  panel->layout().add(label);

  LineEdit* edit = new LineEdit();
  edit->setText("LineEdit");
  edit->setHint("some edit");
  panel->layout().add(edit);

  ListBox* listBox = new ListBox();
  listBox->setDelegate(listBoxDelegate);
  panel->layout().add(listBox);

  scroll = new ScrollWidget();

  ListView* list = new ListView();
  list->setDelegate(listViewDelegate);
  scroll->centralWidget().layout().add(list);
  panel->layout().add(scroll);

  /*
  LineEdit* edit1 = new LineEdit();
  edit1->setText("LineEdit1");
  edit1->setHint("some edit");
  panel->layout().add(edit1);

  LineEdit* edit2 = new LineEdit();
  edit2->setText("LineEdit2");
  edit2->setHint("some edit");
  panel->layout().add(edit2);
  */

  layout().add(panel);
  }

void MainWindow::paintEvent(PaintEvent &e) {
  Painter p(e);

  p.setTexture(texture);
  p.drawRect( Rect(100,100, 256, 256), texture.rect() );

  p.setFont( Application::mainFont() );
  p.drawText(100, 80, hint);

  paintNested(e);
  }

void MainWindow::buttonClick() {
  scroll->setEnabled(!scroll->isEnabledTo(scroll));
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  if( needToUpdate() )
    uiRender.buildWindowVbo(*this, vboHolder, iboHolder, spHolder);

  device.clear( Color(0,0,1), 1 );

  device.beginPaint();
  device.draw( uiRender );
  device.endPaint();

  device.present();
  }

void MainWindow::resizeEvent( SizeEvent & ) {
  device.reset();
  }

void MainWindow::setHint(const std::u16string &h, const Rect &) {
  hint = h;
  update();
  }
