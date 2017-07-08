/*** includes ***/ 

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/

struct termios orig_terminos;

/*** terminal ***/

void die(const char *s){
    // a function to handle errors
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

/*** init ***/

int main(){
    enableRawMode();

    while(1){
        char c = '\0';

        if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN){
            die("read");
        }

        if(iscntrl(c)){
            printf("%d\r\n", c);
        }else{
            printf("%d ('%c')\r\n", c, c);
        }
        if(c=='q'){
            break; // exit if a q is entered
        }
    }

    return 0;
}
