#ifndef LIB_FSQLF_H
#define LIB_FSQLF_H


#include <stdio.h> // FILE
#include "../utils/map/uthash.h" // UT_hash_handle, HASH_ADD_KEYPTR, HASH_FIND_STR


// Return value (in case function returns status code).
// At the moment only OK and FAIL,
// but later might add some more granular codes.
enum fsqlf_status
{
    FSQLF_OK = 0,
    FSQLF_FAIL = 1
};


enum fsqlf_kwcase
{
    // Use whatever was in the input.
    FSQLF_KWCASE_ORIGINAL = 0,

    // Example: "select".
    FSQLF_KWCASE_LOWER = 1,

    // Example: "SELECT".
    FSQLF_KWCASE_UPPER = 2,

    // Example: "Select".
    FSQLF_KWCASE_INITCAP = 3
};


// Keyword text variation to be used.
// (e.g. "left outer join" vs "left join")
enum fsqlf_kwspelling
{
    // Use whatever was in the input.
    FSQLF_KWSPELLING_USE_ORIGINAL = 0,

    // Convert some keywords to fsqlf defaults.
    // Defaults are hard-coded in "kw_defaults.def".
    // Current defaults prefer:
    // - full words (e.g. convert "sel" to "select")
    // - lower word count (e.g. convert "left outer join" to "left join")
    FSQLF_KWSPELLING_USE_HARDCODED_DEFAULT = 1,
};


struct fsqlf_spacing
{
    int global_indent_change;
    int new_line;
    int indent;
    int space;
};


struct FSQLF_out_buffer
{
    char *buffer;
    size_t len_alloc; // allocated length
    size_t len_used;
};


struct fsqlf_kw_conf
{
    struct fsqlf_spacing before;
    struct fsqlf_spacing after;

    enum fsqlf_kwspelling print_original_text;
    enum fsqlf_kwcase print_case;
    unsigned short int is_word; // two adjacent words MUST be separated by some spacing
    char *text;

    const char *name;
    UT_hash_handle hh;  // makes this structure hashable
};


// Typedef to be used when map of all 'kw's is needed.
typedef struct fsqlf_kw_conf * fsqlf_kwmap_t;


#ifdef STATIC_LIBFSQLF
#    define FSQLF_API
#elif defined(_WIN32)
#    ifdef BUILDING_LIBFSQLF
#        define FSQLF_API __declspec(dllexport)
#    else
#        define FSQLF_API __declspec(dllimport)
#    endif
#else
#    define FSQLF_API
#endif


// Create and retrieve  new struct fsqlf_kw_conf and add it to global map.
void FSQLF_kw_create(fsqlf_kwmap_t *kwmap, const char *name);
FSQLF_API struct fsqlf_kw_conf * fsqlf_kw_get(fsqlf_kwmap_t kwmap, const char *name);

// Init and destroy kwmap
FSQLF_API void fsqlf_kwmap_init(fsqlf_kwmap_t *kwmap);
FSQLF_API void fsqlf_kwmap_destroy(fsqlf_kwmap_t kwmap);

// Set case for keyword text.
FSQLF_API void fsqlf_kwmap_set_case(fsqlf_kwmap_t kwmap, enum fsqlf_kwcase keyword_case);

// Set used variation of keyword text. (e.g. "left outer join" vs "left join")
FSQLF_API void fsqlf_kwmap_set_spelling(fsqlf_kwmap_t kwmap, enum fsqlf_kwspelling kw_text_to_use);

// Set new line count before major clauses (from, join, where)
FSQLF_API void fsqlf_kwmap_set_major_clause_nl(fsqlf_kwmap_t kwmap, int nl_count);


// Create & read formatting configuration file.
#ifndef FSQLF_CONFFILE_NAME
#define FSQLF_CONFFILE_NAME "formatting.conf"
#endif


#ifndef FSQLF_CONFFILE_LINELENGTH
#define FSQLF_CONFFILE_LINELENGTH (100)
#endif


FSQLF_API enum fsqlf_status fsqlf_kwmap_conffile_create(char *config_file_name);
FSQLF_API enum fsqlf_status fsqlf_kwmap_conffile_read(fsqlf_kwmap_t kwmap, const char *file);
// "read_default" looks in working directory or "~/.fslqf" for formatting.conf.
FSQLF_API enum fsqlf_status fsqlf_kwmap_conffile_read_default(fsqlf_kwmap_t kwmap);


// Actual formatting
FSQLF_API void fsqlf_format_file(fsqlf_kwmap_t kwmap, FILE *fin, FILE *fout);
FSQLF_API void fsqlf_format_bytes(fsqlf_kwmap_t kwmap,
    const char *bytes_in, int len, char **bytes_out
);


#endif
