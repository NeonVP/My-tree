#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <stdarg.h>


#include "List.h"
#include "Colors.h"
#include "DebugUtils.h"


#ifdef _DEBUG
    static ListStatus_t ListVerify( List_t* list ) {
        my_assert( list, "Null pointer on `list`" );

        if ( list->size > list->capacity ) { list->var_info.error_code |= LIST_ERR_SIZE_CAPACITY_CORRUPTION; return  FAIL_VERIFY; }

        const int size     = ListGetSize    ( list );
        const int capacity = ListGetCapacity( list );

        int idx = 0;
        int counter = 0;

        // CHECK FOR STRAIGHT LOOP
        idx = ListGetHead( list );
        counter = 0;

        while ( idx > 0) {
            idx = ListGetNext( list, idx );
            counter++;

            if ( counter > size ) {
                list->var_info.error_code |= LIST_ERR_STRAIGHT_LOOP;

                break;
            }
        }

        if ( counter < size ) {
            list->var_info.error_code |= LIST_ERR_STRAIGHT_LACK_OF_ELEMENTS;
        }

        // CHECK FOR REVERSE LOOP
        idx = ListGetTail( list );
        counter = 0;

        while ( idx > 0 ) {
            idx = ListGetPrev( list, idx );
            counter++;

            if ( counter > size ) {
                list->var_info.error_code |= LIST_ERR_REVERSE_LOOP;

                break;
            }
        }

        if ( counter < size ) {
            list->var_info.error_code |= LIST_ERR_REVERSE_LACK_OF_ELEMENTS;
        }

        // CHECK FOR LOOP OF FREE ELEMENTS
        idx = ListGetFree( list );
        counter = 0;

        while ( idx > 0) {
            idx = ListGetNext( list, idx );
            counter++;

            if ( counter > capacity - size ) {
                list->var_info.error_code |= LIST_ERR_FREE_LOOP;

                break;
            }
        }

        if ( counter < capacity - size ) {
            list->var_info.error_code |= LIST_ERR_FREE_LACK_OF_ELEMENTS;
        }

        if ( list->var_info.error_code != LIST_ERR_NONE ) {
            return FAIL_VERIFY;
        }

        return SUCCESS;
    }
#endif

ListStatus_t ListCtor( List_t* list, const int data_capacity ) {
    PRINT_EXECUTING;
    my_assert( list, "Null pointer on `list`" );

    list->capacity = data_capacity;
    list->size = 0;

    list->elements = ( Element_t* ) calloc ( ( size_t ) list->capacity + 1, sizeof( *( list->elements ) ) );
    assert( list->elements && "Memory allocation error" );

    list->elements[0].next = 0;
    list->elements[0].prev = 0;
    list->free = 1;

    for ( int idx = 1; idx <= list->capacity; idx++ ) {
        list->elements[ idx ] = { .prev = -1, .value = poison, .next = idx + 1 };
    }

    list->elements[ list->capacity ].next = 0;

    list->logging.image_number = 0;

    ListLog( list, DUMP, "After <font color=\"orange\"> Constructor </font>" );

    #ifdef _DEBUG
        if ( ListVerify( list ) != SUCCESS )  {
            PRINT_STATUS_FAIL;
            return  FAIL;
        }
    #endif

    PRINT_STATUS_OK;
    return SUCCESS;
}

ListStatus_t ListDtor( List_t* list ) {
    PRINT_EXECUTING;
    my_assert( list, "Null pointer on `list`" );

    ListLog( list, DUMP, "Before <font color=\"orange\">Destructor</font>" );

    free( list->elements );
    list->elements = NULL;
    list->size = 0;
    list->capacity = 0;

    #ifdef _DEBUG
        free( list->logging.common_log_directory );
        free( list->logging.log_img_directory );
        free( list->logging.log_img_src_directory );
        free( list->logging.log_file_path );
    #endif

    PRINT_STATUS_OK;
    return SUCCESS;
}

ListStatus_t ListLinearizer( List_t* list, const int new_capacity ) {
    PRINT_EXECUTING;
    my_assert( list,             "Null pointer on `list`" );
    my_assert( new_capacity > 0, "New capacity below 0"   );

    VERIFY( "Before <font color=\"cyan\">Linearizer (%d)</font>", new_capacity );

    if ( new_capacity < ListGetSize( list ) ) {
        fprintf( stderr, COLOR_BRIGHT_RED "The new capacity is smaller than size!\n" );

        PRINT_STATUS_FAIL;
        return FAIL;
    }

    Element_t* data = ( Element_t* ) calloc ( ( size_t ) new_capacity + 1, sizeof( *data ) );
    assert( data && "Memory allocation error" );

    int old_idx = ListGetHead( list );
    int new_idx = 1;

    while ( old_idx != 0 ) {
        data[ new_idx ] = {
            .prev  = new_idx - 1,
            .value = ListGetElement( list, old_idx ),
            .next  = new_idx + 1
        };

        old_idx = ListGetNext( list, old_idx );
        new_idx++;
    }

    data[ new_idx - 1 ].next = 0;

    while ( new_idx <= new_capacity ) {
        data[ new_idx ] = {
            .prev = -1,
            .value = poison,
            .next = new_idx + 1
        };

        new_idx++;
    }

    data[ new_idx - 1 ].next = 0;
    data[0] = {
        .prev = ListGetSize( list ),
        .next = ( ListGetSize( list ) > 0 ) ? 1 : 0
    };

    free( list->elements );
    list->elements = data;
    list->capacity = new_capacity;
    list->free     = ListGetSize( list ) + 1;


    VERIFY( "After <font color=\"cyan\">Linearizer (%d)</font>", new_capacity );

    PRINT_STATUS_OK;
    return SUCCESS;
}

int ListGetHead( const List_t* list ) {
    my_assert( list, "Null pointer on `list`" );

    return list->elements[0].next;
}

int ListGetTail( const List_t* list ) {
    my_assert( list, "Null pointer on `list`" );

    return list->elements[0].prev;
}

int ListGetSize( const List_t *list ) {
    my_assert( list, "Null pointer on `list`" );

    return list->size;
}

int ListGetCapacity( const List_t *list ) {
    my_assert( list, "Null pointer on `list`" );

    return list->capacity;
}

int ListGetFree( const List_t* list ) {
    my_assert( list, "Null pointer on `list`" );

    return list->free;
}

int ListGetNext( const List_t* list, const int index ) {
    my_assert( list, "Null pointer on `list`" );

    if ( list->capacity < index ) return -1;

    return list->elements[ index ].next;
}

int ListGetPrev( const List_t* list, const int index ) {
    my_assert( list, "Null pointer on `list`" );

    if ( list->capacity < index ) return -1;

    return list->elements[ index ].prev;
}

ElementData_t ListGetElement( const List_t* list, const int index ) {
    my_assert( list, "Null pointer on `list`" );

    return list->elements[ index ].value;
}

ListStatus_t ListInsertAfter( List_t* list, const int index, const ElementData_t number ) {
    PRINT_EXECUTING;
    my_assert( list, "Null pointer on `list`" );

    VERIFY( "Before <font color=\"blue\">INSERT ( %lg ) AFTER <%d></font>", number, index );

    if ( list->size == 0 && index > 0 ) {
        fprintf( stderr, COLOR_BRIGHT_RED "Insert after <%d> in empty list \n", index );
        PRINT_STATUS_FAIL;
        return FAIL;
    }

    if ( ListGetPrev( list, index ) == -1 ) {
        fprintf( stderr, COLOR_BRIGHT_RED "Insert after empty element \n" );
        PRINT_STATUS_FAIL;
        return FAIL;   
    }

    if ( ListGetSize( list ) >= ListGetCapacity( list ) ) {
        ListLinearizer( list, list->capacity * 2 );
    }

    int next_free_idx = ListGetNext( list, ListGetFree( list ) );

    list->elements[ list->free ] = { 
        .prev = index, 
        .value = number, 
        .next = list->elements[ index ].next 
    };

    list->elements[ index ].next = list->free;
    list->elements[ list->elements[ list->free ].next ].prev = list->free;

    list->free = next_free_idx;

    list->size++;

    VERIFY( "After <font color=\"blue\">INSERT (%lg) AFTER <%d></font>", number, index );

    PRINT_STATUS_OK;
    return SUCCESS;
}

ListStatus_t ListInsertBefore( List_t* list, const int index, const ElementData_t number ) {
    PRINT_EXECUTING;
    my_assert( list, "Null pointer on `list`" );
    my_assert( isfinite( number ), "An infinite number" );

    VERIFY( "Before <font color=\"blue\"> INSERT (%lg) BEFORE <%d></font>", number, index );

    if ( list->size == 0 && index > 2 ) {
        PRINT_STATUS_FAIL;
        return FAIL;
    }

    if ( list->elements[ index ].prev == -1 ) {
        fprintf( stderr, COLOR_BRIGHT_RED "Insert before empty element \n" );
        PRINT_STATUS_FAIL;
        return FAIL;   
    }

    if ( list->size >= list->capacity ) {
        ListLinearizer( list, list->capacity * 2 );
    }

    int next_free_idx = list->elements[ list->free ].next;

    list->elements[ list->free ].value = number;

    list->elements[ list->free ].prev = list->elements[ index ].prev;
    list->elements[ list->free ].next = index;

    list->elements[ index ].prev = list->free;
    list->elements[ list->elements[ list->free ].prev ].next = list->free;

    list->free = next_free_idx;

    list->size++;

    VERIFY( "After <font color=\"blue\">INSERT (%lg) BEFORE <%d></font>", number, index );

    PRINT_STATUS_OK;
    return SUCCESS;
}

ListStatus_t ListDelete( List_t* list, const int index ) {
    PRINT_EXECUTING;
    my_assert( list, "Null pointer on `list`" );

    VERIFY( "Before <font color=\"red\">DELETE <%d></font>", index );

    if ( list->size == 0 ) {
        fprintf( stderr, "Error: delete from empty list \n" );
        PRINT_STATUS_FAIL;
        return FAIL;
    }

    if ( list->elements[ index ].prev == -1 ) {
        fprintf( stderr, "Error: delete free element \n" );
        PRINT_STATUS_FAIL;
        return FAIL;
    }

    list->elements[ list->elements[ index ].prev ].next = list->elements[ index ].next;
    list->elements[ list->elements[ index ].next ].prev = list->elements[ index ].prev;

    list->elements[ index ].value = poison;
    list->elements[ index ].prev  = -1;
    list->elements[ index ].next  = list->free;
    list->elements[ index ] = {
        .prev = -1,
        .value = poison,
        .next = list->free
    };

    list->free = index;
    list->size--;

    VERIFY( "After <font color=\"red\">DELETE <%d></font>", index );

    PRINT_STATUS_OK;
    return SUCCESS;
}
