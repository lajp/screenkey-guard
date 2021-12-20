#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

static int running = 1;

void handleInt(int _s) {
    running = 0;
}

pid_t new_screenkey() {
    pid_t cpid = fork();
    if(cpid > 0) {
        printf("Spawning screenkey %d\n", cpid);
        return cpid;
    } else if(cpid == 0) {
        char* args[] = {"/usr/bin/screenkey", NULL};
        execv(args[0], args);
        perror("execv: ");
        exit(EXIT_FAILURE);
    } else {
        perror("fork: ");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv) {
    pid_t screenkey;
    if(argc >= 2) {
        screenkey = atoi(argv[1]);
    } else {
        screenkey = new_screenkey();
    }
    if(screenkey == 0) {
        printf("Invalid PID of screenkey");
        return EXIT_FAILURE;
    }
    signal(SIGINT, handleInt);
    bool paused = false;
    while(running) {
        FILE* sudo = popen("pgrep sudo", "r");
        pid_t sudopid = 0;
        if(sudo) {
            fscanf(sudo, "%d", &sudopid);
        }
        pclose(sudo);
        FILE* pinentry = popen("pgrep pinentry", "r");
        pid_t pinentrypid = 0;
        if(pinentry) {
            fscanf(pinentry, "%d", &pinentrypid);
        }
        pclose(pinentry);
        if(sudopid != 0 || pinentrypid != 0) {
            if(!paused) {
                paused = true;
                printf("Killing screenkey of PID %d\n", screenkey);
                kill(screenkey, SIGTERM);
            }
        } else if (paused) {
            paused = false;
            screenkey = new_screenkey();
        }
        usleep(100);
    }
    return 0;
}
