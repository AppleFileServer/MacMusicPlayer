#include "ui_nav.h"
#include "ui_menus.h"
#include "ipod_lcd.h"
#include "lcd_draw.h"
#include "lcd_layout.h"

typedef struct
{
    short menuID;
    short selected;     /* highlighted item */
    short top;          /* first visible item */
} ScreenState;

/* Deepest planned path is 7 screens (Main > Music > Genres > Genre >
   Artist > Album > Now Playing). */
#define kMaxNavDepth    8

static ScreenState gNav[kMaxNavDepth];
static short gNavDepth;

static ScreenState *CurrentScreen(void)
{
    return &gNav[gNavDepth - 1];
}

static void PushScreen(short menuID)
{
    ScreenState *s;

    if (gNavDepth >= kMaxNavDepth)
        return;

    s = &gNav[gNavDepth++];
    s->menuID = menuID;

    /* PROVISIONAL (D7): entering a menu highlights its first item.
       Inferred from the screenshots, not documented. */
    s->selected = 0;
    s->top = 0;
}

void UI_Init(void)
{
    gNavDepth = 0;
    PushScreen(kMenuMain);
}

static Boolean MoveSelection(short delta)
{
    ScreenState *s = CurrentScreen();
    const MenuDef *menu = UIMenus_Get(s->menuID);
    short next = s->selected + delta;

    /* PROVISIONAL (D1, spec unknown #3): the highlight stops at the first
       and last items rather than wrapping. Real 4G behaviour unknown. */
    if (next < 0 || next >= menu->count)
        return false;

    s->selected = next;

    /* Inferred: the list scrolls one row only when the highlight
       would leave the visible rows. */
    if (s->selected < s->top)
        s->top = s->selected;
    else if (s->selected >= s->top + kVisibleRows)
        s->top = s->selected - kVisibleRows + 1;

    return true;
}

static Boolean SelectItem(void)
{
    ScreenState *s = CurrentScreen();
    const MenuItemDef *item = &UIMenus_Get(s->menuID)->items[s->selected];

    /* kNoSubmenu and kSubmenuStub items do nothing in Stage 1. */
    if (item->submenu < 0)
        return false;

    PushScreen(item->submenu);
    return true;
}

static Boolean GoBack(void)
{
    /* PROVISIONAL (D2, spec: unknown): Menu on the main menu does nothing. */
    if (gNavDepth <= 1)
        return false;

    /* PROVISIONAL (D6, Inferred): the previous screen keeps its saved
       highlight and scroll position. */
    gNavDepth--;
    return true;
}

Boolean UI_HandleEvent(UIEvent event)
{
    switch (event)
    {
        case kEvtWheelUp:   return MoveSelection(-1);
        case kEvtWheelDown: return MoveSelection(1);
        case kEvtSelect:    return SelectItem();
        case kEvtMenu:      return GoBack();
        default:            return false;
    }
}

void UI_Render(void)
{
    ScreenState *s = CurrentScreen();
    const MenuDef *menu = UIMenus_Get(s->menuID);
    Boolean hasScrollBar = menu->count > kVisibleRows;
    short position;
    short index;

    LCD_BeginDraw();

    LCDDraw_Clear();
    LCDDraw_Header((ConstStringPtr)menu->title);

    for (position = 0; position < kVisibleRows; position++)
    {
        index = s->top + position;
        if (index >= menu->count)
            break;

        LCDDraw_Row(position, (ConstStringPtr)menu->items[index].title,
                    index == s->selected,
                    menu->items[index].submenu != kNoSubmenu,
                    hasScrollBar);
    }

    if (hasScrollBar)
        LCDDraw_ScrollBar(menu->count, s->top);

    LCD_EndDraw();
}
