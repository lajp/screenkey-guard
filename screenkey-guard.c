#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h> // usleep()
#include <string.h>
#include <dirent.h>

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

bool prunning(const char* program) {
    struct dirent* dirent;
    DIR* proc = opendir("/proc");
    if(!proc) {
        perror("Unable to open /proc: ");
        exit(EXIT_FAILURE);
    }
    char fname[512];
    char pname[256];
    while((dirent = readdir(proc)) != NULL) {
        if(dirent->d_type != 4) {
            // Not a directory
            continue;
        }
        sprintf(fname, "/proc/%s/status", dirent->d_name);
        FILE* status = fopen(fname, "r");
        if(!status) {
            continue;
        }
        if(fscanf(status, "Name:%*[ \t]%s\n", pname) != EOF) {
            if(strstr(pname, program) != NULL) {
                fclose(status);
                return 1;
            }
        }
        fclose(status);
    }
    free(dirent);
    closedir(proc);
    return 0;
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
    bool found = false;
    while(running) {
        found = 0;
        for(int i = 0; programs[i]; ++i) {
            found = prunning(programs[i]);
            if(found) {
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
