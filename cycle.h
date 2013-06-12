#include <windows.h>

void detach();
extern bool detaching;

void handle_debug_event (DEBUG_EVENT *de);
void cycle();

