#include <stdlib.h>
#include <sys/types.h>

#ifdef _LINUX
#include <linux/limits.h>
const size_t MAX_LEN_PATH = PATH_MAX;
#else
const size_t MAX_LEN_PATH = 128;
#endif

#ifndef LIST_H
#define LIST_H

#include "DebugUtils.h"


#define INIT( name ) ON_DEBUG( .var_info={ LIST_ERR_NONE, #name, __func__, __FILE__, __LINE__ } )

#ifdef _DEBUG
#define VERIFY( ... )                                           \
    ListVerify( list );                                              \
    ListLog( list, DUMP, __VA_ARGS__ );                              \
    if ( list->var_info.error_code != LIST_ERR_NONE ) {              \
        PRINT_STATUS_FAIL;                                           \
        return FAIL_VERIFY;                                          \
    }

#define CHECK_STATUS( command )                                                                                                                \
    {                                                                                                                                          \
    ListStatus_t status = command;                                                                                                             \
    if ( status == FAIL ) {                                                                                                                    \
        fprintf( stderr, COLOR_BRIGHT_RED "Incorrect command `%s` \n", #command );                                                             \
        ListDtor( &list );                                                                                                                     \
        ON_DEBUG( ListLog( &list, END_OF_PROGRAM, nullptr ); )                                                                                 \
        return EXIT_FAILURE;                                                                                                                   \
    }                                                                                                                                          \
    else if ( status != SUCCESS ) {                                                                                                            \
        fprintf(                                                                                                                               \
            stderr,                                                                                                                            \
            COLOR_BRIGHT_RED "ERROR: Verification failed while executing `%s`. See logs for details: ./%s. \n",                                \
            #command, list.logging.log_file_path                                                                                               \
        );                                                                                                                                     \
        ListDtor( &list );                                                                                                                     \
        ON_DEBUG( ListLog( &list, END_OF_PROGRAM, nullptr ); )                                                                                 \
        return EXIT_FAILURE;                                                                                                                   \
    }                                                                                                                                          \
    }

#else
#define VERIFY( str, ... ) 
#define CHECK_STATUS( command )                                                                                                                \
    if ( command == FAIL ) {                                                                                                                   \
        fprintf( stderr, COLOR_BRIGHT_RED "Incorrect command `%s` \n", #command );                                                             \
        ListDtor( &list );                                                                                                                     \
        return EXIT_FAILURE;                                                                                                                   \
    }         
#endif

#ifdef _DEBUG
    struct VarInfo_t {
        uint        error_code;
        const char* name;
        const char* func;
        const char* file;
        size_t      line;
    };

    struct LogStat {
        char* common_log_directory;
        char* log_file_path;
        char* log_img_directory;
        char* log_img_src_directory;

        FILE* log_file;

        size_t image_number;
    };

    enum ListErrorCodes {
        LIST_ERR_NONE                      = 0,
        LIST_ERR_SIZE_CAPACITY_CORRUPTION  = 1 << 0,
        LIST_ERR_POISON_CORRUPTION         = 1 << 1,
        LIST_ERR_STRAIGHT_LOOP             = 1 << 2,
        LIST_ERR_REVERSE_LOOP              = 1 << 3,
        LIST_ERR_FREE_LOOP                 = 1 << 4,
        LIST_ERR_STRAIGHT_LACK_OF_ELEMENTS = 1 << 5,
        LIST_ERR_REVERSE_LACK_OF_ELEMENTS  = 1 << 6,
        LIST_ERR_FREE_LACK_OF_ELEMENTS     = 1 << 7
    };

    enum LogModes {
        BEGIN_OF_PROGRAMM = 0,
        DUMP              = 1,
        END_OF_PROGRAM    = 2
    };

    enum CellColorsForGraphviz {
        COLOR_FREE     = 0xE6F3FF,
        COLOR_OCCUPIED = 0xB3D9FF
    };
#endif

enum ListStatus_t {
    SUCCESS     = 0,
    FAIL        = 1,
    FAIL_VERIFY = 2
};

typedef double ElementData_t;

// TODO: hide struct fields from users: transfer struct fo .cpp and add typedef
// struct list;
// typedef list List_t;

const ElementData_t poison = 777;

struct Element_t {
    int           prev;
    ElementData_t value;
    int           next;
};

struct List_t {    
    int size;
    int capacity;
    
    int free;

    Element_t* elements;

    #ifdef _DEBUG
        VarInfo_t var_info;
        LogStat logging;
    #endif
};


ListStatus_t ListCtor( List_t* list, const int data_capacity );
ListStatus_t ListDtor( List_t* lsit );

ListStatus_t ListLinearizer( List_t* list, const int new_capacity );

ListStatus_t ListInsertAfter ( List_t* list, const int index, const ElementData_t number  );
ListStatus_t ListInsertBefore( List_t* list, const int index, const ElementData_t number );
ListStatus_t ListDelete      ( List_t* list, const int index );

int ListGetHead    ( const List_t* list );
int ListGetTail    ( const List_t* list );
int ListGetFree    ( const List_t* list );

int ListGetSize    ( const List_t* list );
int ListGetCapacity( const List_t* list );

int           ListGetNext   ( const List_t* list, const int index );
int           ListGetPrev   ( const List_t* list, const int index );
ElementData_t ListGetElement( const List_t* list, const int index );

#ifdef _DEBUG
    void ListLog( List_t* list, LogModes mode, const char* service_message, ... );
#endif


#endif // LIST_H