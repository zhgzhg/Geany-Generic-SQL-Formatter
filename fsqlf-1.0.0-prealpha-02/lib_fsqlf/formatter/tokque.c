#include <stdio.h>      // fprintf, fputs
#include <assert.h> // assert
#include "../lex/token.h" // struct FSQLF_token, FSQLF_clear_token, FSQLF_set_token
#include "print_keywords.h" // FSQLF_print
#include "tokque.h"
#define YY_HEADER_EXPORT_START_CONDITIONS
#include "lex.yy.h" // start conditions (states)


static const size_t LOOK_BEHIND_COUNT = 1;


void FSQLF_tokque_init(struct FSQLF_queue *tq)
{
    FSQLF_queue_init(tq, sizeof(struct FSQLF_token));
}


static int tokque_print_one(struct FSQLF_queue *tq, FILE *fout,
    struct FSQLF_out_buffer *bout)
{
    if (FSQLF_queue_empty(tq)) {
        return 0; // queue was empty, so printing was not possible
    } else if (tq->length == 1) {
        struct FSQLF_token *tok;
        tok = (struct FSQLF_token *) FSQLF_queue_peek_n(tq, 0);
        struct fsqlf_kw_conf prev = {{0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0};
        FSQLF_print(fout, bout, tok->indent, tok->text, tok->kw_setting, &prev);
        return 1; // success - printing was made
    } else {
        assert(tq->length >= 2);
        struct FSQLF_token *tok, *prev;
        prev = (struct FSQLF_token *) FSQLF_queue_peek_n(tq, 0);
        tok = (struct FSQLF_token *) FSQLF_queue_peek_n(tq, 1);
        FSQLF_print(fout, bout, tok->indent, tok->text, tok->kw_setting,
            prev->kw_setting);
        if (prev->token_class == FSQLF_TOKEN_CLASS_TXT) {
            free(prev->kw_setting);
        }
        FSQLF_queue_drop_head(tq);
        FSQLF_clear_token(tok);
        return 1; // success - printing was made
    }
}


static struct FSQLF_state_change decide_new_state(
    int cur_state,
    const struct fsqlf_kw_conf *s
)
{
    if (s) {
        if (strcmp(s->name, "kw_deletefrom") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, stDELETE};
        else if (strcmp(s->name, "kw_insertinto") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, stINSERT};
        else if (strcmp(s->name, "kw_update") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, stUPDATE};
        else if (strcmp(s->name, "kw_create") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, stCREATE};
        else if (strcmp(s->name, "kw_drop") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, INITIAL};
        else if (strcmp(s->name, "kw_ifexists") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, INITIAL};
        else if (strcmp(s->name, "kw_view") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, INITIAL};
        else if (strcmp(s->name, "kw_union") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, INITIAL};
        else if (strcmp(s->name, "kw_union_all") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, INITIAL};
        else if (strcmp(s->name, "kw_minus") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, INITIAL};
        else if (strcmp(s->name, "kw_intersect") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, INITIAL};
        else if (strcmp(s->name, "kw_except") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, INITIAL};
        else if (strcmp(s->name, "kw_semicolon") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, INITIAL};
        else if (strcmp(s->name, "kw_groupby") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, stGROUPBY};
        else if (strcmp(s->name, "kw_orderby") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, stORDERBY};
        else if (strcmp(s->name, "kw_having") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, stWHERE};
        else if (strcmp(s->name, "kw_qualify") == 0) return (struct FSQLF_state_change) {FSQLF_SCA_BEGIN, stWHERE};
    }

    return (struct FSQLF_state_change) {0, 0};
}


// This routine has goal to do 4 things:
// 1. Place token on the queue.
// 2. Accumulate tokens till enough info is available for recognition.
// 3. Perform recognition.
// 4. Output fully recognized tokens.
///
// At the moment only 1st and 4th parts are done.
// TODO: implement 2nd and 3rd
struct FSQLF_state_change FSQLF_tokque_putthrough(
    struct FSQLF_queue *tq,
    FILE *fout,
    struct FSQLF_out_buffer *bout,
    int *currindent,
    char *text,
    size_t len,
    struct fsqlf_kw_conf *kw0,
    int cur_state
)
{
    // Place on queue.
    {
        // Note: de-allocation will be done based on token-class.
        enum FSQLF_token_class tcls =
            kw0 ? FSQLF_TOKEN_CLASS_KW : FSQLF_TOKEN_CLASS_TXT;
        struct fsqlf_kw_conf *kw =
            kw0 ? kw0 : FSQLF_derive_kw_from_text(text, len);

        struct FSQLF_token *tok =
            (struct FSQLF_token *) FSQLF_queue_alloc_back(tq);
        *currindent += kw->before.global_indent_change;
        FSQLF_set_token(tok, tcls, text, len, kw, (*currindent));
        *currindent += kw->after.global_indent_change;
    }

    // Retrieve from queue and print.
    tokque_print_one(tq, fout, bout);

    // Send command for state change
    return decide_new_state(cur_state, kw0);
}


void FSQLF_tokque_finish_out(struct FSQLF_queue *tq, FILE *fout, struct FSQLF_out_buffer *bout)
{
    // Print out all queue
    while (tq->length > LOOK_BEHIND_COUNT) {
        tokque_print_one(tq, fout, bout);
    }
    // Cleanup
    FSQLF_queue_clear(tq);
}
