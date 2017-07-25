#ifndef STYLE_H
#define STYLE_H

#include <Tempest/Utility>
#include <string>
#include <cstdint>

namespace Tempest {

struct Margin;
class  Painter;
class  Font;
class  Color;
class  Icon;

class  WidgetState;

class  Widget;
class  Button;
class  CheckBox;
class  Panel;
class  Label;
class  LineEdit;

class Style {
  public:
    Style();
    Style(const Style* parent);
    virtual ~Style();

    struct Extra {
      public:
        Extra(const Widget&   owner);
        Extra(const Button&   owner);
        Extra(const Label&    owner);
        Extra(const LineEdit& owner);

        const std::u16string& txt;
        const Margin&         margin;
        const Icon&           icon;
        const Font&           font;
        const Color&          fontColor;

      private:
        static const std::u16string  emptyTxt;
        static const Tempest::Margin emptyMargin;
        static const Tempest::Icon   emptyIcon;
        static const Tempest::Font   emptyFont;
        static const Tempest::Color  emptyColor;
      };

    void setParent(const Style* stl);

    virtual void draw(Painter& p, Widget*   w, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, Panel *   w, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, Button*   w, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, CheckBox* w, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, Label*    w, const WidgetState& st, const Rect& r, const Extra& extra) const;
    virtual void draw(Painter& p, LineEdit* w, const WidgetState& st, const Rect& r, const Extra& extra) const;

  protected:
    virtual void polish  (Widget& w) const;
    virtual void unpolish(Widget& w) const;

  private:
    mutable uint32_t polished=0;
    mutable int32_t  counter =0;
    const   Style*   parent  =nullptr;

    void    addRef() const { counter++;                             }
    void    decRef() const { counter--; if(counter==0) delete this; }

    static void drawCursor(Painter &p, bool emptySel, const WidgetState &st, int x1, int x2, int h, bool animState);

  friend class Widget;
  };

}

#endif // STYLE_H