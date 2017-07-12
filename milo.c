/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)
//this definition sets the upper 3 bits of the character to 0 -- similar to what the cntrl key does

/*** data ***/

struct termios orig_terminos;

/*** terminal ***/

void die(const char *s){
    // a function to handle errors
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3); 

    perror(s);
    exit(1);
}

void disableRawMode(){
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_terminos) == -1){
        die("tcsetattr");
    }
}

void enableRawMode(){
    //a function to turn off Echoing i.e. no text displayed ala passwords
    if(tcgetattr(STDIN_FILENO, &orig_terminos) == -1){
        die("tcgetattr");
    }

    atexit(disableRawMode); // on exit execute the disableRawMode function

    struct termios raw = orig_terminos;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    //as IXON is an input flag we modify the c_iflag field - fix cntrl s
    //ICRNL fixes ctrl m - misinterpreted carriage returns or new lines
    //nb this means that carriage returns are not enabled by default - and must manually precede newline characters 

    raw.c_oflag &= ~(OPOST);
    //opost stops all output processing features

    raw.c_cflag |= (CS8);

    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    //bitshifts - ICANON is a flag to read 1 bit at a time.
    //ISIG is to stop SIGINT or SIGTSTP - cntrl c
    //IEXTEN is to stop cntrl v

    raw.c_cc[VMIN] = 0; //sets min number of bytes before read returns
    raw.c_cc[VTIME] = 1; //sets max time before read returns

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1){
        die("tcsetattr");
    }
}

char editorReadKey(){
    int nread;
    char c;
    while((nread = read(STDIN_FILENO, &c, 1)) != 1){
        if(nread == -1 && errno != EAGAIN){
            die("read");
        }
    }
    return c;
}

/*** output ***/

void editorRefreshScreen(){
    write(STDOUT_FILENO, "\x1b[2J", 4); 
    //4 bytes are wrtten to the terminal first byte is \x1b the escape character (27 decimal)
    //J command is Erase In Display - clears screen, takes argument beforehand (it is 2 here)
    //2 means to clear entier screen, 1 means to clear up to cursor and 0 clears from cursor to end
    //REF: VT100 escape sequences -- TODO: ncurses library to handle all terminals

    write(STDOUT_FILENO, "\x1b[H", 3);
    //Uses H command which takes in row and column to position screen - multiple args seperated with ';' default is top though
}

/*** input ***/

void editorProcessKeypress(){
    char c = editorReadKey();
    switch(c){
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}

/*** init ***/

int main(){
    enableRawMode();

    while(1){
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}
