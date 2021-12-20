#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h> // usleep()
#include <string.h>

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

const char* programs[] = { "sudo", "pinentry", "doas", NULL };

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
        bool found = 0;
        for(int i = 0; programs[i]; ++i) {
            char* command = malloc(7*strlen(programs[i]));
            strcpy(command, "pgrep ");
            strcat(command, programs[i]);
            FILE* pipe = popen(command, "r");
            pid_t ppid = 0;
            if(pipe) {
                fscanf(pipe, "%d", &ppid);
            }
            pclose(pipe);
            if((found = (ppid > 0))) {
                printf("%s is running\n", programs[i]);
                break;
            }
        }
        if(found) {
            if(!paused) {
                paused = true;
                printf("Killing screenkey of PID %d\n", screenkey);
                kill(screenkey, SIGTERM);
            }
        } else if (paused) {
            paused = false;
            screenkey = new_screenkey();
        }
        usleep(1000);
    }
    return 0;
}
