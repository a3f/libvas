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

static void heal(const char *spell);
static HWND hwnd;

int main(void)
{
    uint32_t old_health = 0;
    vas_t *proc;
    vas_poll_t *poller;
    DWORD pid;

    hwnd = FindWindow("tibiaclient", NULL);
    assert(hwnd != (void*)-1);
    GetWindowThreadProcessId(hwnd, &pid);

    proc = vas_open(pid, 0);
    assert(proc);
    poller = vas_poll_new(proc, CHARACTER_HEALTH, sizeof(uint32_t), VAS_O_REPORT_ERROR);
    assert(poller);

    while (1) {
        uint32_t health;
        vas_poll(poller, &health);
        if (health != old_health)
            printf("Character health is %" PRIu32 "\n", health);
        old_health = health;

        if (health < MIN_HEALTH) {
            while (health < MAX_HEALTH) {
                heal(SPELL);
                Sleep(MIN_HEALTH);
                vas_poll(poller, &health);
                printf("Character health is %" PRIu32 "\n", health);
            }
        }


        Sleep(350);
    }

    vas_poll_del(poller);
    return 0;
}
static void send_char(char ch) {
    SendMessage(hwnd, WM_KEYDOWN, ch - 0x20, 0);
    SendMessage(hwnd, WM_CHAR, ch, 0);
    SendMessage(hwnd, WM_KEYUP, ch - 0x20, 0);
}
static void heal(const char *spell) {
    const char *ch;
    for (ch = spell; *ch; ch++)
        send_char(*ch);


    SendMessage(hwnd, WM_KEYDOWN, 0xD, 0);
    SendMessage(hwnd, WM_CHAR, 0xD, 0);
    SendMessage(hwnd, WM_KEYUP, 0xD, 0);
}
