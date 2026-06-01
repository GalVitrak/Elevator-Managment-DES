#ifndef TEXT_TABLE_H
#define TEXT_TABLE_H

#include <stdio.h>

#define TEXT_TABLE_MAX_COLS 8

typedef struct {
    FILE* out;
    int colCount;
    int widths[TEXT_TABLE_MAX_COLS];
} TextTable;

/* Centered banner line (width 80). */
void text_table_print_banner(FILE* out, const char* title);

/* Large ASCII "ELEVATOR" title (console, log, results file). */
void text_table_print_project_ascii_art(FILE* out);

/* Section heading with blank line after. */
void text_table_print_section(FILE* out, const char* title);

/*
 * Begin a boxed table: prints top border and optional header row.
 * widths[] has colCount entries; each cell is width chars (+ padding).
 */
void text_table_begin(TextTable* table, FILE* out, const int* widths, int colCount);
void text_table_header(TextTable* table, const char* const* headers);
void text_table_row(TextTable* table, const char* const* cells);
void text_table_row_line(FILE* out, const int* widths, int colCount,
                         const char* const* cells);
void text_table_separator(TextTable* table);
void text_table_end(TextTable* table);

/* Two-column metric / value table (convenience). */
void text_table_print_key_values(FILE* out, const char* const* keys,
                                 const char* const* values, int rowCount);

#endif /* TEXT_TABLE_H */
