#ifndef BUTTON_H
#define BUTTON_H

#include <Tempest/Widget>
#include <Tempest/Shortcut>
#include <Tempest/Sprite>
#include <Tempest/Font>

#include <ctime>

namespace Tempest {

class Button : public Tempest::Widget {
  public:
    Button();

    Tempest::signal<> clicked;

    Tempest::Sprite icon;

    const Tempest::Shortcut& shortcut() const;
    void  setShortcut( const Tempest::Shortcut & sc );

    const std::u16string text() const;
    void  setText(const std::u16string &t );
    void  setText( const std::string& t   );

    const std::u16string &hint() const;
    void  setHint( const std::u16string & str );
    void  setHint( const std::string & str    );

    const Tempest::Font& font() const;
    void  setFont( const Tempest::Font& f );

    void setFontColor(const Tempest::Color& color);
    const Tempest::Color& fontColor() const;

  protected:
    virtual void drawFrame(Tempest::Painter &p, const Tempest::Rect& r );
    virtual void drawFrame(Tempest::Painter &p);

    virtual void drawBack(Tempest::Painter &p, const Tempest::Rect& r );
    virtual void drawBack(Tempest::Painter &p);

    virtual Tempest::Rect viewRect() const;

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);
    void paintEvent( Tempest::PaintEvent &p);

    void gestureEvent(Tempest::AbstractGestureEvent &e);

    void keyUpEvent(Tempest::KeyEvent &e);

    void focusChange(bool);

    void onShortcut();
    virtual void emitClick();

    bool isPressed() const;
    void finishPaint();

  private:
    clock_t timePressed;
    bool pressed, presAnim;
    std::u16string txt, hnt;

    Tempest::Shortcut hotKey;
    Tempest::Font  fnt;
    Tempest::Color fntColor;
  };
}

#endif // BUTTON_H