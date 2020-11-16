# Comment-Analysis-C

Extracting and reading Javadoc/doxygen-style tags from comments in valid C source code

Part of my collection of schoolwork I did for the course Data Structures and Programming Techniques.

A program called Comments that reads C source code and writes the Javadoc/doxygen-style tags contained in the top-level comments – a comment that is not in a C code block delimited by { and }. 
The program operates in one of two modes: either all tags in top-level comments are output, or only the tags at the beginning of a line in such comments. The mode the program operates in is determined by a command-line argument: -a for all tags, and -l for leading tags (those at the beginning of a line), with leading tags being the default if neither -a nor -l is given. 
Input and output are by default read from and written to standard input and standard output. This may be changed with up to one occurrence each of the -i and -o command-line switches. 
In each case, the argument following the switch is the name of the file to read from or write to (which may not be the same physical file after resolving path names and symbolic links). If the output file already exists, its existing contents are overwritten.

# How to determine what is a tag
A tag is a sequence of characters that starts with an at symbol (@) that is preceded by the beginning of the comment, whitespace (as determined by the isspace function), or an asterisk that is part of the beginning of a comment or line (see below); the tag continues to (but not including) the next whitespace character or the end of the comment. For example,

    // *** *** @this_tag_is_considered_to_be_at_the_beginning_of_a_line
    
    //@this_is_a_tag
    
    //@this_is_a_single@tag
    

# How to determine which tags are at the beginning of a line
A tag is considered to be at the beginning of a line if it is the first thing on the line after any sequence of whitespace characters and asterisks (\*). The beginning of a comment is considered to be the beginning of a line regardless of whether the comment itself is at the beginning of a line. For example,
    
    int x; /* @this_tag_is_considered_to_be_at_the_beginning_of_a_line. */
  
# How to process command-line arguments
Normally, a command-line argument that starts with a hyphen (-) is interpreted as a switch. But a command-line argument after an -i or -o that is interpreted as a switch is interpreted as a filename and never as a switch (and the precedence is left to right). 
