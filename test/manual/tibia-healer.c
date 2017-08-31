#include <vas.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#define CHARACTER_HEALTH 0x63FE94
#define MIN_HEALTH 1000
#define MAX_HEALTH 1500
#define SPELL "exura gran"

void heal(const char *spell);
HANDLE hwnd;

int main(int argc, char *argv[]) {
    uint32_t old_health = 0;
    long pid;
    if (argc != 2 || !(pid = strtol(argv[1], NULL, 0))) {
        fprintf(stderr, "Specify PID as argument!\n");
        exit(1);
    }

    hwnd = FindWindow("tibiaclient", NULL);
    assert(hwnd != (void*)-1);

    while (1) {
        vas_t *proc;
        uint32_t health;
        int ret;
        proc = vas_open(pid, 0);
        assert(proc);
        ret = vas_read(proc, CHARACTER_HEALTH, &health, sizeof(uint32_t));
        assert(ret != -1);
        if (health != old_health)
            printf("Character health is %" PRIu32 "\n", health);
        old_health = health;

        if (health < MIN_HEALTH) {
            while (health < MAX_HEALTH) {
                heal(SPELL);
                Sleep(MIN_HEALTH);
                vas_read(proc, CHARACTER_HEALTH, &health, sizeof(uint32_t));
            }
        }


        Sleep(350);
    }
}
void send_char(char ch) {
    SendMessage(hwnd, WM_KEYDOWN, ch - 0x20, 0);
    SendMessage(hwnd, WM_CHAR, ch, 0);
    SendMessage(hwnd, WM_KEYUP, ch - 0x20, 0);
}
void heal(const char *spell) {
    const char *ch;
    for (ch = spell; *ch; ch++)
        send_char(*ch);


    SendMessage(hwnd, WM_KEYDOWN, 0xD, 0);
    SendMessage(hwnd, WM_CHAR, 0xD, 0);
    SendMessage(hwnd, WM_KEYUP, 0xD, 0);
}
