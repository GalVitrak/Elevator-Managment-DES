/*
 * text_table.c - ASCII box tables for logs and reports
 */
#include "text_table.h"

#include <string.h>

#define BANNER_WIDTH 80

static void text_table_hline(FILE* out, const int* widths, int colCount)
{
    int col;
    int pad;

    fputs("+", out);
    for (col = 0; col < colCount; col++) {
        for (pad = 0; pad < widths[col] + 2; pad++) {
            fputc('-', out);
        }
        fputs("+", out);
    }
    fputc('\n', out);
}

static void text_table_print_cell(FILE* out, int width, const char* text)
{
    size_t len;
    char truncated[256];

    if (text == NULL) {
        text = "";
    }

    len = strlen(text);
    fputs("| ", out);
    if ((int)len <= width) {
        fprintf(out, "%-*s", width, text);
        fputs(" ", out);
        return;
    }

    if (width < 4) {
        fprintf(out, "%.*s", width, text);
        fputs(" ", out);
        return;
    }

    snprintf(truncated, sizeof(truncated), "%.*s...", width - 3, text);
    fprintf(out, "%-*s", width, truncated);
    fputs(" ", out);
}

void text_table_print_banner(FILE* out, const char* title)
{
    int titleLen;
    int leftPad;
    int i;

    if (out == NULL || title == NULL) {
        return;
    }

    titleLen = (int)strlen(title);
    if (titleLen > BANNER_WIDTH - 4) {
        titleLen = BANNER_WIDTH - 4;
    }
    leftPad = (BANNER_WIDTH - titleLen) / 2;
    if (leftPad < 0) {
        leftPad = 0;
    }

    for (i = 0; i < BANNER_WIDTH; i++) {
        fputc('=', out);
    }
    fputc('\n', out);

    for (i = 0; i < leftPad; i++) {
        fputc(' ', out);
    }
    fprintf(out, "%.*s\n", titleLen, title);

    for (i = 0; i < BANNER_WIDTH; i++) {
        fputc('=', out);
    }
    fputc('\n', out);
}

void text_table_print_section(FILE* out, const char* title)
{
    if (out == NULL || title == NULL) {
        return;
    }
    fputc('\n', out);
    fprintf(out, "%s\n", title);
}

void text_table_begin(TextTable* table, FILE* out, const int* widths, int colCount)
{
    int i;

    if (table == NULL || out == NULL || widths == NULL) {
        return;
    }

    table->out = out;
    table->colCount = colCount;
    if (colCount > TEXT_TABLE_MAX_COLS) {
        table->colCount = TEXT_TABLE_MAX_COLS;
    }

    for (i = 0; i < table->colCount; i++) {
        table->widths[i] = widths[i];
    }

    text_table_hline(out, table->widths, table->colCount);
}

void text_table_header(TextTable* table, const char* const* headers)
{
    int col;

    if (table == NULL || table->out == NULL || headers == NULL) {
        return;
    }

    for (col = 0; col < table->colCount; col++) {
        text_table_print_cell(table->out, table->widths[col], headers[col]);
    }
    fputs("|\n", table->out);
    text_table_separator(table);
}

void text_table_row_line(FILE* out, const int* widths, int colCount,
                         const char* const* cells)
{
    int col;

    if (out == NULL || widths == NULL || cells == NULL || colCount <= 0) {
        return;
    }

    for (col = 0; col < colCount; col++) {
        text_table_print_cell(out, widths[col], cells[col]);
    }
    fputs("|\n", out);
}

void text_table_row(TextTable* table, const char* const* cells)
{
    if (table == NULL || table->out == NULL || cells == NULL) {
        return;
    }
    text_table_row_line(table->out, table->widths, table->colCount, cells);
}

void text_table_separator(TextTable* table)
{
    if (table == NULL || table->out == NULL) {
        return;
    }
    text_table_hline(table->out, table->widths, table->colCount);
}

void text_table_end(TextTable* table)
{
    if (table == NULL || table->out == NULL) {
        return;
    }
    text_table_hline(table->out, table->widths, table->colCount);
}

void text_table_print_key_values(FILE* out, const char* const* keys,
                                 const char* const* values, int rowCount)
{
    static const int widths[2] = { 34, 40 };
    TextTable table;
    int i;
    const char* rowCells[2];

    if (out == NULL || keys == NULL || values == NULL || rowCount <= 0) {
        return;
    }

    text_table_begin(&table, out, widths, 2);
    rowCells[0] = "Metric";
    rowCells[1] = "Value";
    text_table_header(&table, rowCells);

    for (i = 0; i < rowCount; i++) {
        rowCells[0] = keys[i];
        rowCells[1] = values[i];
        text_table_row(&table, rowCells);
    }
    text_table_end(&table);
}
