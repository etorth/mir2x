#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "fltableimpl.hpp"

Fl_TableImpl::Fl_TableImpl(int argX, int argY, int argW, int argH, const char *labelCPtr)
    : Fl_Table_Row(argX, argY, argW, argH, labelCPtr)
{
    // begin
    // put a blank table in ctor
    {
        rows(0);
        row_header(0);
        row_height_all(20);
        row_resize(0);

        cols(0);
        col_header(1);
        col_resize_min(80);
        col_width_all(100);
        col_resize(0);
    }
    end();

    // register callbacks
    callback([](Fl_Widget *, void *dataPtr) -> void
    {
        ((Fl_TableImpl *)(dataPtr))->onClick();
    }, this);
    when(FL_WHEN_RELEASE);
}

int Fl_TableImpl::getFontWidth() const
{
    // disble the @SYMBOL handling
    int nW = 0;
    int nH = 0;

    const char *sz10WidthChars = "MMMMMHHHHH";
    fl_measure(sz10WidthChars, nW, nH, 0);

    return nW / 10;
}

void Fl_TableImpl::onClick()
{
    // I don't know the reason
    // the document says if setup when(FL_WHEN_RELEASE) we should only receive on release the button
    // but I get event on both press and release

    if(Fl::event() != FL_RELEASE){
        return;
    }

    const auto nRow = callback_row();
    const auto nCol = callback_col();
    const auto nCtx = callback_context();

    // setup new UID to hightlight
    // seems if click on table's dead zone, this will return row and col both as ZERO
    // but should I relay on this???
    if((nCtx == CONTEXT_TABLE) /* ((nRow == 0 ) && (nCol == 0)) */){
        setSortByCol(-1);
        setSortOrder(true);

        onClickOff();

        sortTable();
        redraw();
        return;
    }

    if((nCtx == CONTEXT_CELL) && checkRow(nRow) && checkCol(nCol)){
        onClickCell(nRow, nCol);
        if(Fl::event_clicks() >= 1){
            onDoubleClickCell(nRow, nCol);
        }

        sortTable();
        redraw();
        return;
    }

    // setup sort col
    // need to make sure this is the click on col header

    if((nCtx == CONTEXT_COL_HEADER) && (nRow == 0) && checkCol(nCol)){
        if(m_sortByCol == nCol){
            flipSortOrder();
        }
        else{
            setSortByCol(nCol);
            setSortOrder(true);
        }

        onClickCol(nCol);

        sortTable();
        redraw();
        return;
    }
}

void Fl_TableImpl::draw_cell(TableContext nContext, int nRow, int nCol, int nX, int nY, int nW, int nH)
{
    switch(nContext){
        case CONTEXT_STARTPAGE:
            {
                return;
            }
        case CONTEXT_COL_HEADER:
            {
                drawHeader(getColName(nCol).c_str(), nX, nY, nW, nH);
                return;
            }
        case CONTEXT_ROW_HEADER:
            {
                drawHeader("???", nX, nY, nW, nH);
                return;
            }
        case CONTEXT_CELL:
            {
                drawData(nRow, nCol, nX, nY, nW, nH);
                return;
            }
        default:
            {
                return;
            }
    }
}

void Fl_TableImpl::drawHeader(const char *szInfo, int nX, int nY, int nW, int nH)
{
    fl_push_clip(nX, nY, nW, nH);
    {
        fl_draw_box(FL_THIN_UP_BOX, nX, nY, nW, nH, row_header_color());
        fl_color(FL_BLACK);
        fl_font(FL_COURIER, 14);
        fl_draw(szInfo, nX, nY, nW, nH, FL_ALIGN_CENTER);
    }
    fl_pop_clip();
}

void Fl_TableImpl::drawData(int nRow, int nCol, int nX, int nY, int nW, int nH)
{
    int fg_color = FL_BLACK;
    int bg_color = FL_WHITE;

    const bool selectedRow = rowSelected(nRow);
    const bool selectedCol = colSelected(nCol);

    if(selectedRow){
        fg_color = FL_WHITE;
        bg_color = 0xaa4444;
    }

    if(selectedCol){
        fg_color = FL_WHITE;
        bg_color = 0x44aa44;
    }

    if(selectedRow && selectedCol){
        fg_color = FL_WHITE;
        bg_color = 0x4444aa;
    }

    fl_push_clip(nX, nY, nW, nH);
    {
        fl_color(bg_color);
        fl_rectf(nX, nY, nW, nH);

        fl_color(fg_color);
        fl_font(FL_COURIER, 14);
        fl_draw(getGridData(nRow, nCol).c_str(), nX, nY, nW, nH, FL_ALIGN_CENTER);

        fl_color(color());
        fl_rect(nX, nY, nW, nH);
    }
    fl_pop_clip();
}
