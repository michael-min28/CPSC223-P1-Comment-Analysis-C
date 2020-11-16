#ifndef __COMMENTS_H
#define __COMMENTS_H

#include <stdbool.h>
#include <stdio.h>

/**
 * Checks to see if there's a line continuation (\\n); if so, it skips.
 * @param input: a file to open
 **/
bool lineCont(char c, char d);

#endif
