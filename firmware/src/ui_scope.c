/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* (Skiff screen link) the output oscilloscope's reading, one for the screen and the editor: graph_scope (ui_graph.c,
 * HOME and the EDIT pages) draws it, SCOPE (editor.c, 93, INFO 56 01 4) sends it. audio.c fills scope_buf from the
 * output every block whatever the page (every second frame, the left channel, before the click); this copies the
 * ring in the main loop, as the screen always did, so the audio code knows nothing of who looks. Its own file so that
 * the editor's host test (tests/editor_test.c, no drawing code) takes the same function the screen does.
 *   scope_take(&peak) -> the trigger: scope_snap[] holds the ring from its oldest sample, *peak the largest |sample|
 *   (at least SCOPE_FLOOR: quiet stays small instead of being blown up to full height, silence a flat line), and the
 *   trace is the SCOPE_SPAN samples from the first rising zero crossing (none: from 0, a free-running picture)
 * Main loop only: the screen and the editor share scope_snap (each takes it fresh and is done with it before the
 * other runs: ed_service and ui_draw are turns of the same loop, main.c). */
#define SCOPE_SPAN 240u                              /* the trace: one sample a pixel, the panel's width */
#define SCOPE_FLOOR 1500                             /* the scale's least peak (Q15, about -27 dB) */
static int16_t scope_snap[SCOPE_N];
static uint32_t scope_take(int32_t *peak)
{
    uint32_t w = scope_w, i, trig = 0;
    int32_t pk = SCOPE_FLOOR;
    for (i = 0; i < SCOPE_N; i++) {
        scope_snap[i] = scope_buf[(w + i) & (SCOPE_N - 1u)];
        if (scope_snap[i] > pk)
            pk = scope_snap[i];
        else if (-scope_snap[i] > pk)
            pk = -scope_snap[i];
    }
    for (i = 1; i < SCOPE_N - SCOPE_SPAN; i++)
        if (scope_snap[i - 1] < 0 && scope_snap[i] >= 0) {
            trig = i;
            break;
        }
    *peak = pk;
    return trig;
}
