#ifndef LISTBOX_H
#define LISTBOX_H

#include <vector>
#include <string>

#include <Tempest/AbstractListBox>

namespace Tempest{

class Button;

/** \addtogroup GUI
 *  @{
 */
class ListBox : public AbstractListBox {
  public:
    ListBox();
    ~ListBox();

    template<class D>
    void setDelegate(const D& d){
      removeDelegate();

      listDelegate.reset(new D(d));
      setupView(size_t(-1));
      setupDelegateCallback();
      }

    void setItems(const std::vector<std::u16string>& items);
    void setItems(const std::vector<std::string>&    items);

    void removeDelegate();

    void invalidateView();
    void updateView();

    Tempest::signal<size_t> onItemSelected;

    void   setCurrentItem( size_t i );
    size_t currentItem() const;

    size_t itemsCount() const;

  protected:
    void mouseWheelEvent(Tempest::MouseEvent &e);
    using AbstractListBox::layout;

    void setupView(size_t oldSelected);
    void updateSelectedView();
    const std::shared_ptr<ListDelegate>& delegate();

    Tempest::Button* createItemButton(ListDelegate &owner, size_t pos, Tempest::ListDelegate::Role r);

  private:
    size_t selected;
    bool dropListEnabled;
    std::shared_ptr<ListDelegate> listDelegate;

    Tempest::Widget *createDropList();
    Tempest::Widget *view;

    class ItemBtn;

    class BasicButton;

    template<class Array>
    class BasicDelegate;

    void onItem(size_t id,Widget *view);
    void selectItem( size_t id );

    void setupDelegateCallback();

    struct ProxyDelegate;
  };
/** @}*/

}

#endif // LISTBOX_H
