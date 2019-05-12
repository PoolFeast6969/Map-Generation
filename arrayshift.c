
#include <string.h>
#include <stdlib.h>

int shift_rows(int* map, int rows, int columns, int shift_amount) {
    int element_size = sizeof(int);

    // When you add to a pointer its mulitplied by the size of the pointers type
    // So i don't need element_size
    void* new_address = map+(shift_amount < 0 ? -shift_amount : 0);

    int* source_address = map+(shift_amount < 0 ? -shift_amount : 0)*columns;
    int* shift_address = map+(shift_amount < 0 ? 0 : shift_amount)*columns;

    // Don't move the last row down because you'll move it outside the array
    int bytes_to_move = (rows-abs(shift_amount))*columns*element_size;

    // Shift the array contents down by one row
    memmove(shift_address, source_address, bytes_to_move);

    // Zero the unmoved row(s)
    memset((shift_amount < 0 ? map+rows*columns+shift_amount*columns : map), 0, element_size * columns * abs(shift_amount));
};

int shift_columns(int* map, int rows, int columns, int shift_amount) {
    int element_size = sizeof(int);

    for (size_t i = 0; i < rows; i++) {   
        int* row_address = map+columns*i;

        int* source_address = row_address+(shift_amount < 0 ? -shift_amount : 0);
        int* shift_address = row_address+(shift_amount < 0 ? 0 : shift_amount);

        // Don't move the last row down because you'll move it outside the array
        int bytes_to_move = (columns-abs(shift_amount))*element_size;
    
        // Shift the array contents down by one row
        memmove(shift_address, source_address, bytes_to_move);
        
        // Zero the unshifted parts
        int* unshifted_start = shift_amount < 0 ? row_address+columns+shift_amount : row_address;
        memset(unshifted_start, 0, element_size * abs(shift_amount));
    }
};

int main() {
    const int rows=3,columns=3;

    int map[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    shift_columns(map, rows, columns, 2);
}
