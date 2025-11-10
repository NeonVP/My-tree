#include <cstdarg>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>


#include "Colors.h"
#include "DebugUtils.h"
#include "List.h"

#ifdef _DEBUG
    #define PRINT_IN_LOG( str, ... )        fprintf(  list->logging.log_file, str, ##__VA_ARGS__ );
    #define PRINT_IN_LOG_VAARG( str, args ) vfprintf( list->logging.log_file, str, args );
    #define PRINT_IN_GRAPHIC( str, ... )    fprintf(  graphic_file,           str, ##__VA_ARGS__ );

    #define PASS

    static void CommandExecute( const char* format_string, ... ) {
        my_assert( format_string, "Null pointer of `format_string`" );

        char command[ MAX_LEN_PATH ] = "";
        va_list args;
        va_start( args, format_string );
        vsnprintf( command, MAX_LEN_PATH, format_string, args );
        va_end( args );
        
        system( command );
    }

    static void FolderCreate( List_t* list ) {
        my_assert( list, "Null pointer on `list`" );

        time_t current_time = time( NULL );
        struct tm* local_time;
        local_time = localtime( &current_time );
        
        char folder_name[ MAX_LEN_PATH ] = "";
        strftime( folder_name, sizeof( folder_name ), "%d.%m_%H:%M:%S", local_time );

        char buffer[ MAX_LEN_PATH ] = "";

        snprintf( buffer, MAX_LEN_PATH, "logs/%s", folder_name );
        list->logging.common_log_directory = strdup( buffer );

        snprintf( buffer, MAX_LEN_PATH, "%s/images", list->logging.common_log_directory );
        list->logging.log_img_directory = strdup( buffer );

        snprintf( buffer, MAX_LEN_PATH, "%s/image_sources", list->logging.common_log_directory );
        list->logging.log_img_src_directory = strdup( buffer );

        snprintf( buffer, MAX_LEN_PATH, "%s/LOGFILE.htm", list->logging.common_log_directory );
        list->logging.log_file_path = strdup( buffer );

        CommandExecute( "mkdir -p %s", list->logging.log_img_directory );
        CommandExecute( "mkdir -p %s", list->logging.log_img_src_directory );
    }

        static void ListPrintStats( const List_t* list ) {
        my_assert( list, "Null pointer on `list`" );

        PRINT_IN_LOG( 
            "List `%s` { %s:%lu } \n", 
            list->var_info.name,
            list->var_info.file, 
            list->var_info.line 
        );

        PRINT_IN_LOG( 
            "SIZE = %d; CAPACITY = %d \n\n", 
            list->size, 
            list->capacity 
        );

        PRINT_IN_LOG( 
            "HEAD = %d; TAIL = %d; FREE = %d \n", 
            list->elements[0].next, 
            list->elements[0].prev, 
            list->free 
        );

        // INDEX
        PRINT_IN_LOG( "IDX  | " );
        for ( int idx = 1; idx <= list->capacity; idx++ ) {
            PRINT_IN_LOG( 
                "%3d%s", 
                idx, 
                ( idx == list->capacity ) ? ( " |\n" ) : ( " | " ) 
            );
        }

        // DATA
        PRINT_IN_LOG( "DATA | " );
        for ( int idx = 1; idx <= list->capacity; idx++ ) {
            PRINT_IN_LOG( 
                "%3lg%s", 
                list->elements[ idx ].value, 
                ( idx == list->capacity ) ? ( " |\n" ) : ( " | " ) 
            );
        }

        // NEXT
        PRINT_IN_LOG( "NEXT | " );
        for ( int idx = 1; idx <= list->capacity; idx++ ) {
            PRINT_IN_LOG( 
                "%3d%s", 
                list->elements[ idx ].next, 
                ( idx == list->capacity ) ? ( " |\n" ) : ( " | " ) 
            );
        }

        // PREV
        PRINT_IN_LOG( "PREV | " );
        for ( int idx = 1; idx <= list->capacity; idx++ ) {
            PRINT_IN_LOG( 
                "%3d%s", 
                list->elements[ idx ].prev, 
                ( idx == list->capacity ) ? ( " |\n" ) : ( " | " ) 
            );
        }
    }


    static void ListDump( const List_t* list, const char* service_message, va_list args ) {
        my_assert( list, "Null pointer to `list`" );

        PRINT_IN_LOG      ( "<h3> DUMP " );
        PRINT_IN_LOG_VAARG( service_message, args );
        PRINT_IN_LOG      ( " </h3> \n" );

        uint error_code = list->var_info.error_code;

        if ( error_code != LIST_ERR_NONE ) {
            if ( error_code & LIST_ERR_SIZE_CAPACITY_CORRUPTION ) {
                PRINT( COLOR_BRIGHT_RED "Size/capacity corruption!!!" );

                return;
            }

            const char* error_message[] = {
                "Size/capacity corruption",
                "Poison corruption",
                "Straight loop",
                "Reverse loop",
                "Free loop",
                "Lack of elements in straight passage",
                "Lack of elements in reverse passage",
                "Lack of elements in passage of free elements"
            };

            for ( int n = 0; n < 8; n++ ) {
                if ( list->var_info.error_code & ( 1 << n) ) {
                    PRINT_IN_LOG( "<font color=\"red\">%s</font> \n", error_message[ n ] );
                }
            }
        }
        else {
            PRINT_IN_LOG( "<font color=\"green\">No error</font>\n" );
        }

        
        PRINT_IN_LOG( "\n\n" );

        ListPrintStats( list );

        PRINT_IN_LOG( 
            "\n<img src=\"images/image%lu.png\" height=\"200px\">\n",
            list->logging.image_number
        );

        PRINT_IN_LOG( "\n\n" );
    }

    static void ProcessConnection( FILE* graphic_file, List_t* list, int source_idx ) {
        my_assert( graphic_file, "Null pointer on `graphic file" );
        my_assert( list, "Null pointer on `list`" );

        int next_idx = list->elements[ source_idx ].next;
        int prev_idx = list->elements[ source_idx ].prev;

        if ( prev_idx == -1 ) {
            if ( next_idx == 0 ) return;

            PRINT_IN_GRAPHIC(
                "\tnode%d -> node%d [constraint=false, color=\"#643a6fff\", style=dashed, arrowsize=0.6];\n",
                source_idx, next_idx  
            );
            
            return;
        }

        if ( next_idx <= list->capacity ) {
            PRINT_IN_GRAPHIC( 
                "\tnode%d -> node%d [%s]; \n",
                source_idx, next_idx,
                ( source_idx == list->elements[ next_idx ].prev ) ? ( "constraint=false, color=black, dir=both, arrowhead=vee, arrowtail=vee, arrowsize=0.5" ) : ( "constraint=false, color=blue, penwidth=3.0" )
            );
        }
        else {
            PRINT_IN_GRAPHIC(
                "\tnode%d -> node%d[%s]\n",
                source_idx, next_idx,
                "constraint=false, color=red, penwidth=4.0"
            );
        }

        if ( prev_idx <= list->capacity ) {
            if ( source_idx == list->elements[ prev_idx ].next ) {
                PASS;
            }
            else {
                PRINT_IN_GRAPHIC(
                    "\tnode%d -> node%d[%s];\n",
                    source_idx, prev_idx, 
                    "constraint=false, color=cyan, penwidth=3.0"
                );
            }
        }
        else {
            PRINT_IN_GRAPHIC(
                "\tnode%d -> node%d[%s]\n",
                source_idx, prev_idx,
                "constraint=false, color=red, penwidth=5.0"
            );
        }        
    }

    static void GraphicPrintoutDraw( List_t* list ) {
        my_assert( list, "Null pointer on `list`" );

        char graphic_source_address[ MAX_LEN_PATH ] = "";

        snprintf( 
            graphic_source_address, MAX_LEN_PATH, 
            "%s/image%lu.dot", 
            list->logging.log_img_src_directory, list->logging.image_number
        );

        FILE* graphic_file = fopen( graphic_source_address, "w" );
        assert( graphic_file && "File opening error" );

        PRINT_IN_GRAPHIC( "digraph {\n\trankdir=LR;\tsplines=ortho;\n\tnodesep=1.5;\n\n\tnode[shape=doubleoctagon; color=\"red\"];\n\n" );
        PRINT_IN_GRAPHIC( 
            "\tnode0 [shape=Mrecord; style=filled; color=black; fillcolor=orange; label = \"prev=%d | next=%d\" ]; \n", 
            list->elements[0].prev,
            list->elements[0].next
        );

        for ( int idx = 1; idx <= list->capacity; idx++ ) {
            PRINT_IN_GRAPHIC( 
                "\tnode%d [shape=Mrecord; style=filled; color=black; fillcolor=\"#%X\"; label=\"idx=%d | value=%lg | { prev=%d | next=%d }\" ]; \n", 
                idx, 
                ( uint32_t ) ( ( list->elements[ idx ].prev == -1 ) ? COLOR_FREE : COLOR_OCCUPIED ),
                idx,
                list->elements[ idx ].value,
                list->elements[ idx ].prev,
                list->elements[ idx ].next
            );
            // PRINT_IN_GRAPHIC( "\t{ rank=same; node%d };\n", idx );
        }

        PRINT_IN_GRAPHIC( "\n\n" );

        for ( int idx = 0; idx < list->capacity; idx++ ) {
            PRINT_IN_GRAPHIC( 
                "\tnode%d -> node%d [style=invis];\n", 
                idx, 
                idx + 1
            );
        }

        PRINT_IN_GRAPHIC( "\n\n")

        for ( int idx = 0; idx <= list->capacity; idx++ ) {
            ProcessConnection( graphic_file, list, idx );
        }

        #define ANNOUNCE( string ) \
            PRINT_IN_GRAPHIC( \
                "\t%s [shape=note; style=filled; color=black; fillcolor=\"#FFFF99\"; label=\"%s\\n %d\"];\n", \
                #string, #string, ListGet##string( list ) \
            ); \
            PRINT_IN_GRAPHIC( "\t{ rank=same; %s; node%d };\n", #string, ListGet##string ( list ) ); \
            PRINT_IN_GRAPHIC( "\t%s -> node%d[arrowsize=0.7];\n\n", #string, ListGet##string ( list ) );

        ANNOUNCE( Head );
        ANNOUNCE( Tail );
        ANNOUNCE( Free );

        #undef ANNOUNCE

        PRINT_IN_GRAPHIC( "}" );

        int close_result = fclose( graphic_file );
        assert( close_result == 0 && "File closing error" );

        CommandExecute( 
            "dot %s -Tpng -o %s/image%lu.png", 
            graphic_source_address, list->logging.log_img_directory, list->logging.image_number 
        );

        list->logging.image_number++;
    }


    void ListLog( List_t* list, const LogModes mode, const char* service_message, ... ) { 
        my_assert( list, "Null pointer on `list`" );

        switch ( mode ) {   
            case BEGIN_OF_PROGRAMM: {
                FolderCreate( list );

                list->logging.log_file = fopen( list->logging.log_file_path, "w" );
                assert( list->logging.log_file && "File opening error" );

                fprintf( list->logging.log_file, "<pre>\n" );

                break;
            }
            case DUMP: {
                va_list args;
                va_start( args, service_message );
                ListDump( list, service_message, args );
                va_end( args );
                GraphicPrintoutDraw( list );

                break;
            }
            case END_OF_PROGRAM: {
                int close_result = fclose( list->logging.log_file );
                assert( close_result == 0 &&"File closing error" );            

                break;
            }
            default: {
                fprintf( stderr, "Unkmown mode\n" );
                
                break;
            }       
        }
    }
#endif