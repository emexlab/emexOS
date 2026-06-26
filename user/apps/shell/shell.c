/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: shell.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>


//-////////////////////////////////////////-//
//-//                                    //-//
//-//       emex user-space shell        //-//
//-//                                    //-//
//-////////////////////////////////////////-//

#define BUFFER 256
#define SHELL_PROMPT "\n\033[0mx1:\033[31m[pc@emex]#\033[0m "
#define SHELL_CONFIG "/.config/exsh/"
#define BIN_PATH "/bin/"

#define WELCOME_MESSAGE "\n\n\033[0m Welcome to eXsh, emexOS's default shell.\n Type \"ls /bin\" for a list of commands.\n\n"


static int parse_args(char *buf, char **argv, int max_args)
{
    int argc = 0;
    char *p = buf;

    while (*p && argc < max_args - 1) {
        // skip spaces
        while (*p == ' ') p++;
        if (*p == '\0') break;

        argv[argc++] = p;

        // find end of token
        while (*p && *p != ' ') p++;
        if (*p == ' ') *p++ = '\0';
    }

    argv[argc] = NULL;
    return argc;
}
static int exec_from_bin(const char *cmd, char **argv)
{
    // /bin/ + cmd + .elf
    char path[BUFFER];
    char *const envp[] = { (char *)0 };
    size_t bin_len = sizeof(BIN_PATH) - 1; // length of "/bin/"
    size_t cmd_len = strlen(cmd);
    size_t elf_len = 4; // ".elf"

    if (bin_len + cmd_len + elf_len + 1 > BUFFER) return -1;

    memcpy(path, BIN_PATH, bin_len);
    memcpy(path + bin_len, cmd, cmd_len);
    memcpy(path + bin_len + cmd_len, ".elf", elf_len + 1); // +1 for \0

    return execve(path, argv, envp);
}

static void builtin_exec(char **argv)
{
    if (!argv[1]) {
        printf("exec: missing path\n");
        return;
    }

    int bg = 0;
    int arg_start = 1;
    if (strcmp(argv[1], "-bg") == 0) {
        bg = 1;
        arg_start = 2;
    }

    if (!argv[arg_start]) {
        printf("exec: missing path\n");
        return;
    }

    char *const envp[] = { (char *)0 };

    for (int i = arg_start; argv[i]; i++)
    {
        pid_t pid = fork();

        if (pid == 0) {
            char *child_argv[] = { argv[i], NULL };

            execve(argv[i], child_argv, envp);

            printf("exec: failed to run: %s\n", argv[i]);
            _exit(1);
        }
        else if (pid < 0) {
            printf("exec: fork failed\n");
            return;
        }

        if (!bg) {
            waitpid(pid, NULL, 0);
        }
    }
}

static void builtin_gui(void)
{
    char *const envp[] = { (char *)0 };

    struct {
        const char *path;
        int bg;
    } apps[] =
    {
        { "/emr/system/desktop.elf",  1 },
        { "/emr/system/login.elf",    0 },
        { "/emr/system/sysinfo.elf",  1 },
        { "/emr/system/terminal.elf", 0 },
        { NULL, 0 }
    };

    for (int i = 0; apps[i].path; i++)
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            char *argv[] = { (char *)apps[i].path, NULL };
            execve(apps[i].path, argv, envp);

            printf("gui: failed to run %s\n", apps[i].path);
            _exit(1);
        }
        else if (pid < 0) {
            printf("gui: fork failed\n");
            return;
        }

        if (!apps[i].bg) {
            waitpid(pid, NULL, 0);
        }
    }
}
int main(void)
{
	printf(WELCOME_MESSAGE);

    char buf[BUFFER];
    char *argv[32];

    for (;;)
    {
        printf(SHELL_PROMPT);

        // block until newline (\n)
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);

        if (n <= 0) continue;

        buf[n] = '\0';

        if (n > 0 && buf[n - 1] == '\n') buf[--n] = '\0';
        if (n == 0) continue;

        int argc = parse_args(buf, argv, 32);
        if (argc == 0) continue;

        builtins: {
            if (strcmp(argv[0], "exec") == 0) {
                builtin_exec(argv);
                continue;
            }

            if (strcmp(argv[0], "gui") == 0) {
                builtin_gui();
                continue;
            }
        }

        int ret = exec_from_bin(argv[0], argv);
        if (ret < 0) {
            printf(argv[0]);
            printf(": command not found\n");
        }
    }

    return 0;
}
