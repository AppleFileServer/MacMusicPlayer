#include "ui_nav.h"
#include "ui_lists.h"
#include "ui_menus.h"
#include "ipod_lcd.h"
#include "lcd_draw.h"
#include "lcd_layout.h"

/* Deepest planned path is 7 screens (Main > Music > Genres > Genre >
   Artist > Album > Now Playing). */
#define kMaxNavDepth    8

static Screen gNav[kMaxNavDepth];
static short gNavItems[kMaxNavDepth][kMaxListItems];
static short gNavDepth;

static Screen *CurrentScreen(void)
{
    return &gNav[gNavDepth - 1];
}

/* Pushes a screen whose kind, menuID, filter and title are set. */
static void PushScreen(const Screen *target)
{
    Screen *s;

    if (gNavDepth >= kMaxNavDepth)
        return;

    s = &gNav[gNavDepth];
    *s = *target;
    s->items = gNavItems[gNavDepth];
    gNavDepth++;

    UIList_Build(s);

    /* PROVISIONAL (D7): entering a menu highlights its first item.
       Inferred from the screenshots, not documented. */
    s->selected = 0;
    s->top = 0;
}

void UI_Init(void)
{
    Screen main;

    main.kind = kScreenMenu;
    main.menuID = kMenuMain;
    LibFilter_Clear(&main.filter);
    main.title = (ConstStringPtr)UIMenus_Get(kMenuMain)->title;

    gNavDepth = 0;
    PushScreen(&main);
}

static Boolean MoveSelection(short delta)
{
    Screen *s = CurrentScreen();
    short next = s->selected + delta;

    /* PROVISIONAL (D1, spec unknown #3): the highlight stops at the first
       and last items rather than wrapping. Real 4G behaviour unknown. */
    if (next < 0 || next >= s->count)
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
    Screen *s = CurrentScreen();
    Screen next;

    /* Items with nothing to open yet (stubs, songs) do nothing. */
    if (s->count == 0 || !UIList_ItemTarget(s, s->selected, &next))
        return false;

    PushScreen(&next);
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
    Screen *s = CurrentScreen();
    Boolean hasScrollBar = s->count > kVisibleRows;
    short position;
    short index;

    LCD_BeginDraw();

    LCDDraw_Clear();
    LCDDraw_Header(s->title);

    for (position = 0; position < kVisibleRows; position++)
    {
        index = s->top + position;
        if (index >= s->count)
            break;

        LCDDraw_Row(position, UIList_ItemTitle(s, index),
                    index == s->selected,
                    UIList_ItemHasArrow(s, index),
                    hasScrollBar);
    }

    if (hasScrollBar)
        LCDDraw_ScrollBar(s->count, s->top);

    LCD_EndDraw();
}
