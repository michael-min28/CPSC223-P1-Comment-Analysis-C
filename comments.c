#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "comments.h"

enum state {READING, INCODE, INCODE_SINGLECOMM, INCODE_MULTICOMM, INCODE_SINGLEQUOTES, INCODE_DOUBLEQUOTES, SINGLEQUOTES, DOUBLEQUOTES, CHECK, SINGCOMM_WS, SINGCOMM_PRINT, SINGCOMM_WAIT, MULTICOMM_WS, MULTICOMM_PRINT, MULTICOMM_WAIT};
enum comment {MULTI, SINGLE};



int main(int argc, char *argv[]) {
    // Initialize to look at command line arguments
    // If none, l is automatically true
    bool a = false;
    bool l = true;
    // Check if the mode has been set; if so, don't change it afterwards
    bool modeChange = false;
    // Check if input and output files have been given
    bool in = false;
    bool out = false;
    // The file indices should be where -i and -o are, +1
    int i_fileIndex = 0;
    int o_fileIndex = 0;
    FILE* inputFile, * outputFile;
    

    // Checking command line arguments
    // TODO: more handling for -i and -o
    for (int i = 1; i < argc; i++) {
        char *val = argv[i];
        // mode -a; if the mode is set, it cannot change.
        if (strcmp(val, "-a") == 0 && !modeChange) {
            a = true;
            l = false;
            modeChange = true;
        }
        // mode -l; if the mode is set, it cannot change.
        else if (strcmp(val, "-l") == 0 && !modeChange) {
            a = false;
            l = true;
            modeChange = true;
        }
        // input file; if the file is followed by nothing, the program will fail gracefully.
        else if (strcmp(val, "-i") == 0 && (i + 1) < argc) {
            in = true;
            i_fileIndex = i + 1;
        }

        // output file; if the file is followed by nothing, the program will fail gracefully
        else if (strcmp(val, "-o") == 0 && (i + 1) < argc) {
            out = true;
            o_fileIndex = i + 1;
        }
    }

    // INPUT
    if (in) {
        inputFile = fopen(argv[i_fileIndex], "r");
        if (!inputFile) {
            fprintf(stderr, "%s: couldn't open %s\n", argv[0], argv[i_fileIndex]);
            return 1;
        }
    }
    else {
        inputFile = stdin;
    }

    // OUTPUT
    if (out) {
        outputFile = fopen(argv[o_fileIndex], "w");
        if (!outputFile) {
            fprintf(stderr, "%s: couldn't open %s\n", argv[0], argv[o_fileIndex]);
            return 1;
        }
    }
    else {
        outputFile = stdout;
    }

    enum state curr = READING;
    char c;

    // if mode -l
    if (l) {
        int bracketCount = 0;
        while(fscanf(inputFile, "%c", &c) > 0) {
            switch(curr) {
                // While case is READING
                case READING:
                    // if we see a bracket, begin counting; we're in code
                    if (c == '{') {
                        curr = INCODE;
                        bracketCount++;
                    }
                    // if we see quotes, we're looking only for quotes (and a pair of them)
                    if (c == '\'') {
                        curr = SINGLEQUOTES;
                    }
                    if (c == '\"') {
                        curr = DOUBLEQUOTES;
                    }
                    // if we see a backslash, check next char to see if we have a comment
                    if (c == '/') {
                        curr = CHECK;
                    }
                    break;

                // While case is INCODE; or between c code blocks
                case INCODE:
                    // if we're in code, and the } bracket is the last one to exit the code
                    if (c == '}' && bracketCount == 1) {
                        curr = READING;
                        bracketCount--;
                    }
                    // There's still more brackets to exit the code
                    else if (c == '}') {
                        bracketCount--;
                    }
                    // We have nested brackets within brackets
                    else if (c == '{') {
                        bracketCount++;
                    }
                    // We want to ignore brackets that are in comments
                    else if (c == '/') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '*') {
                            curr = INCODE_MULTICOMM;
                        }
                        else if (d == '/') {
                            curr = INCODE_SINGLECOMM;
                        }
                    }
                    // We also want to ignore brackets that are in quotes

                    else if (c == '\'') {
                        curr = INCODE_SINGLEQUOTES;
                    }
                    else if (c == '\"') {
                        curr = INCODE_DOUBLEQUOTES;
                    }
                    break;
                
                case INCODE_SINGLECOMM:
                    if (c == '\n') {
                        curr = INCODE;
                    }
                    break;
                
                case INCODE_MULTICOMM:
                    if (c == '*') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '/') {
                            curr = INCODE;
                        }
                    }
                    break;

                // single quotes, but in code
                case INCODE_SINGLEQUOTES:
                    // if after the escape sequence there's a \\, just skip to the next char
                    if (c == '\\') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '\'') {
                        }
                    }
                    else if (c == '\'') {
                        curr = READING;
                    } 
                    break;

                // Double quotes, but in code
                case INCODE_DOUBLEQUOTES:
                // if after the escape sequence there's a \\, just skip to the next char
                    if (c == '\\') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '\"') {
                        }
                    }
                    if (c == '\"') {
                        curr = READING;
                    }
                    break;

                case SINGLEQUOTES:
                    // if after the escape sequence there's a \\, just skip to the next char
                    if (c == '\\') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '\'') {
                        }
                    }
                    else if (c == '\'') {
                        curr = READING;
                    } 
                    break;

                case DOUBLEQUOTES:
                    // if after the escape sequence there's a \\, just skip to the next char
                    if (c == '\\') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '\"') {
                        }
                    }
                    if (c == '\"') {
                        curr = READING;
                    }
                    break;

                // Check to see if the comment is a single-line comment or a multi-line comment
                case CHECK:
                    if (c == '/') {
                        curr = SINGCOMM_WS;
                    }
                    else if (c == '*') {
                        curr = MULTICOMM_WS;
                    }
                    else {
                        fprintf(stderr, "Your input file contains invalid C.\n");
                        return 1;
                    }
                    break;
                
                // Look for white space before the tag
                case SINGCOMM_WS:
                    // Line continuation
                    if (c == '\\') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (lineCont(c, d) == true) {
                            fscanf(inputFile, "%c", &c);
                        }
                        else {
                            c = d;
                        }
                    }
                    // If we see a newline at any point (excluding line continuation, which is handled above), we end the comment
                    if (c == '\n') {
                        curr = READING;
                        break;
                    }
                    // We do nothing for * and whitespace
                    else if (c == '*' || isspace(c) != 0) {
                        // Nothing!
                    }
                    // If we see an @, we print the @ and begin printing
                    else if (c == '@') {
                        fprintf(outputFile, "@");
                        curr = SINGCOMM_PRINT;
                    }
                    // If we see anything that's not an @, *, or whitespace, we're not at the beginning of a line, so we wait until we see a new line (ie the comment is finished) to begin scanning again
                    else {
                        curr = SINGCOMM_WAIT;
                    }
                    break;
                
                // Handles printing for single-line comments
                case SINGCOMM_PRINT:
                    // While there's no whitespace
                    while (isspace(c) == 0) {
                        // line continuation
                        if (c == '\\') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            // print both chars if it's not line continuation
                            // if it is, do nothing
                            if (lineCont(c, d) == false) {
                                fprintf(outputFile, "%c%c", c, d);
                            }
                        }
                        // the \ is the only character we need to handle, so we print all
                        else {
                            fprintf(outputFile, "%c", c);
                        }
                        fscanf(inputFile, "%c", &c);
                    }
                    // print the newline 
                    fprintf(outputFile, "\n");
                    // go to waiting; however, it's possible that the whitespace that we ended on is a newline
                    curr = SINGCOMM_WAIT;
                    // if so, we just immediately begin scanning again
                    if (c == '\n') {
                        curr = READING;
                    }
                    break;

                // Waits for a new line to start before scanning
                case SINGCOMM_WAIT:
                    if (c == '\n') {
                        curr = READING;
                    }  
                    break;
                    
                // Handles the whitespace before tags in multi-line comments
                case MULTICOMM_WS:;
                    // are we on a new line? in -l, we go to print only on new lines
                    bool newLine = true;
                    bool run = true;
                    while (run) {
                        // Check to see if we're on a new line
                        if (c == '\n') {
                            newLine = true;
                        }
                        // check for line continuation
                        if (c == '\\') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            // if line continues, move to next char
                            if (lineCont(c, d) == true) {
                                fscanf(inputFile, "%c", &c);
                            }
                            // if line doesn't continue, we're not at the beginning of the line
                            else {
                                c = d;
                                newLine = false;
                            }
                        }
                        // if not a space and also not an asterisk or an at; move on, we're not at the beginning of the line
                        if (isspace(c) == 0 && !(c == '*' || c == '@')) {
                            newLine = false;
                            fscanf(inputFile, "%c", &c);
                        }
                        // if we are at the beginning of the line, we start printing
                        else if (c == '@' && newLine) {
                            fprintf(outputFile, "@");
                            curr = MULTICOMM_PRINT;
                            run = false;
                        }
                        // if an asterisk (because multiline)
                        else if (c == '*') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            if (d == '/') {
                                curr = READING;
                                break;
                            }
                            else {
                                c = d;
                            }
                        }
                        else if (isspace(c) != 0){
                            fscanf(inputFile, "%c", &c);
                        }
                        else {
                            newLine = false;
                            fscanf(inputFile, "%c", &c);
                        }
                        
                    }
                    break;
                
                // handles printing
                case MULTICOMM_PRINT:
                    while (isspace(c) == 0) {
                        if (c == '\\') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            // print both chars if it's not line continuation
                            // if it is, do nothing
                            if (lineCont(c, d) == false) {
                                fprintf(outputFile, "%c%c", c, d);
                            }
                        }
                        // Check to see if an asterisk is part of the tag or part the end of the comment
                        else if (c == '*') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            // if it's part of the end of the comment, go back to scanning
                            if (d == '/') {
                                curr = READING;
                                run = false;
                                break;
                            }
                            // otherwise, print both
                            else {
                                fprintf(outputFile, "%c%c", c, d);
                            }
                        }
                        else {
                            fprintf(outputFile, "%c", c);
                        }
                        
                        fscanf(inputFile, "%c", &c);
                    }
                    fprintf(outputFile, "\n");
                    // if we're not complete with the comment, then we need to wait for the new line to begin scanning for tags again
                    if (curr != READING) {
                        curr = MULTICOMM_WAIT;
                    }
                    // it's possible that the character that stops the printing is the newline; if so, we start scanning for tags immediately
                    if (c == '\n') {
                        curr = MULTICOMM_WS;
                    }
                    break;

                // wait for new line or for the comment to finish
                case MULTICOMM_WAIT:
                    // asterisk handling
                    if (c == '*') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '/') {
                            curr = READING;
                            run = false;
                        }
                        else {
                            c = d;
                        }
                    }
                    if (c == '\n') {
                        curr = MULTICOMM_WS;
                    }
                    break;
                
                default:
                    break;
            }
        }
    }
    // If there's no comment explaining it, assume that the functionality is the same as in -l, which is described above
    if (a) {
        // some upper-level variables that aren't in the same scope as the cases are necessary to see if we're at the beginning of comments or not (for asterisks, to see if they are part of the whitespace before tags or not)
        bool tagSCWS = true;
        bool beginningSCWS = true;
        bool tagMCWS = true;
        int bracketCount = 0;
        while(fscanf(inputFile, "%c", &c) > 0) {
            switch(curr) {
                // While case is READING
                case READING:
                    if (c == '{') {
                        curr = INCODE;
                        bracketCount++;
                    }
                    if (c == '\'') {
                        curr = SINGLEQUOTES;
                    }
                    if (c == '\"') {
                        curr = DOUBLEQUOTES;
                    }
                    if (c == '/') {
                        curr = CHECK;
                    }
                    break;

                // While case is INCODE; or between c code blocks
                case INCODE:
                    if (c == '}' && bracketCount == 1) {
                        curr = READING;
                        bracketCount--;
                    }
                    else if (c == '}') {
                        bracketCount--;
                    }
                    else if (c == '{') {
                        bracketCount++;
                    }
                    else if (c == '/') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '*') {
                            curr = INCODE_MULTICOMM;
                        }
                        else if (d == '/') {
                            curr = INCODE_SINGLECOMM;
                        }
                    }
                    else if (c == '\'') {
                        curr = INCODE_SINGLEQUOTES;
                    }
                    else if (c == '\"') {
                        curr = INCODE_DOUBLEQUOTES;
                    }
                    break;
                
                case INCODE_SINGLECOMM:
                    if (c == '\n') {
                        curr = INCODE;
                    }
                    break;
                
                case INCODE_MULTICOMM:
                    if (c == '*') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '/') {
                            curr = INCODE;
                        }
                    }
                    break;

                // single quotes, but in code
                case INCODE_SINGLEQUOTES:
                    // if after the escape sequence there's a \\, just skip to the next char
                    if (c == '\\') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '\'') {
                        }
                    }
                    else if (c == '\'') {
                        curr = READING;
                    } 
                    break;

                // Double quotes, but in code
                case INCODE_DOUBLEQUOTES:
                // if after the escape sequence there's a \\, just skip to the next char
                    if (c == '\\') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '\"') {
                        }
                    }
                    if (c == '\"') {
                        curr = READING;
                    }
                    break;

                case SINGLEQUOTES:
                    // if after the escape sequence there's a \\, just skip to the next char
                    if (c == '\\') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '\'') {
                        }
                    }
                    else if (c == '\'') {
                        curr = READING;
                    } 
                    break;

                case DOUBLEQUOTES:
                    // if after the escape sequence there's a \\, just skip to the next char
                    if (c == '\\') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (d == '\"') {
                        }
                    }
                    if (c == '\"') {
                        curr = READING;
                    }
                    break;

                case CHECK:
                    if (c == '/') {
                        curr = SINGCOMM_WS;
                        beginningSCWS = true;
                    }
                    else if (c == '*') {
                        curr = MULTICOMM_WS;
                    }
                    else {
                        fprintf(stderr, "Your input file contains invalid C.\n");
                    }
                    break;
                
                case SINGCOMM_WS:
                    // Line continuation case;
                    if (c == '\\') {
                        char d;
                        fscanf(inputFile, "%c", &d);
                        if (lineCont(c, d) == true) {
                            // move to the next char
                            fscanf(inputFile, "%c", &c);
                        }
                        else {
                            // but if it's just \\ stranded then it's not a tag
                            beginningSCWS = false;
                            tagSCWS = false;
                            c = d;
                        }
                    }
                    if (c == '\n') {
                        curr = READING;
                        break;
                    }
                    // Check for beginning asterisks; stop when you hit an @ or a character that doesn't belong at the beginning of a comment

                    // while at the beginning AND the character is an asterisk OR at the beginning OR the character is a space
                    while ((beginningSCWS && (c == '*')) || ((isspace(c) != 0) || beginningSCWS)) {
                        tagSCWS = true;
                        // break immediately if c is @
                        if (c == '@') {
                            break;
                        }
                        // line continuation
                        if (c == '\\') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            if (lineCont(c, d) == false) {
                                beginningSCWS = false;
                                tagSCWS = false;
                                c = d;
                            }
                        }
                        // if c is NOT a beginning asterisk AND is NOT a whitespace
                        if (!(beginningSCWS && (c == '*')) && isspace(c) == 0) {
                            // then you aren't at the beginning and you're not in whitespace
                            beginningSCWS = false;
                            tagSCWS = false;
                            break;
                        }
                        fscanf(inputFile, "%c", &c);
                    }

                    // when approved for tags
                    if (tagSCWS) {
                        if (c == '@') {
                            fprintf(outputFile, "@");
                            curr = SINGCOMM_PRINT;
                        }
                        //
                        else if (isspace(c) == 0 && c != '*' ) {
                            tagSCWS = false;
                        }
                    }
                    break;
                
                case SINGCOMM_PRINT:
                    while (isspace(c) == 0) {
                        if (c == '\\') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            // print both chars if it's not line continuation
                            // if it is, do nothing
                            if (lineCont(c, d) == false) {
                                fprintf(outputFile, "%c%c", c, d);
                            }
                        }
                        else {
                            fprintf(outputFile, "%c", c);
                        }
                        fscanf(inputFile, "%c", &c);
                    }
                    fprintf(outputFile, "\n");
                    // we have whitespace, because we finish printing because of the whitespace
                    tagSCWS = true;
                    curr = SINGCOMM_WS; 
                    break;
                    
                case MULTICOMM_WS:;
                    bool run = true;
                    while (run) {
                        if (c == '\\') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            // if line continues, move to next char
                            if (lineCont(c, d) == true) {
                                fscanf(inputFile, "%c", &c);
                            }
                            // if line doesn't continue, set c to be that next char
                            else {
                                c = d;
                                tagMCWS = false;
                            }
                        }
                        // if not a space, not an asterisk, and not an @
                        if (isspace(c) == 0 && !(c == '*' || c == '@')) {
                            // then move on
                            fscanf(inputFile, "%c", &c);
                            tagMCWS = false;
                        }
                        // if an @
                        else if (c == '@' && tagMCWS) {
                            fprintf(outputFile, "@");
                            curr = MULTICOMM_PRINT;
                            break;
                        }
                        else if (isspace(c) != 0) {
                            tagMCWS = true;
                            fscanf(inputFile, "%c", &c);
                        }
                        // if an asterisk
                        if (c == '*') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            if (d == '/') {
                                curr = READING;
                                run = false;
                            }
                            else {
                                c = d;
                            }
                        }   
                    }
                    break;
                
                case MULTICOMM_PRINT:
                    while (isspace(c) == 0) {
                        if (c == '\\') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            // Line continuation case;
                            if (lineCont(c, d) == false) {
                                fprintf(outputFile, "%c%c", c, d);
                            }
                        }
                        else if (c == '*') {
                            char d;
                            fscanf(inputFile, "%c", &d);
                            // if it's part of the end of the comment, go back to scanning
                            if (d == '/') {
                                curr = READING;
                                run = false;
                                break;
                            }
                            // otherwise, print both
                            else {
                                fprintf(outputFile, "%c%c", c, d);
                            }
                        }
                        else {
                            fprintf(outputFile, "%c", c);
                        }
                        fscanf(inputFile, "%c", &c);
                    }
                    fprintf(outputFile, "\n");
                    tagMCWS = true;
                    curr = MULTICOMM_WS;
                    break;
                
                case SINGCOMM_WAIT:;
                    break;

                case MULTICOMM_WAIT:;
                    break;

                default:
                    break;

            }
        }        
    }
    return 0;
}

bool lineCont(char c, char d) {
    bool lineContinued = false;

    if (c == '\\') {
        if (d == '\n') {
            lineContinued = true;
        }
    }
    return lineContinued;
}

