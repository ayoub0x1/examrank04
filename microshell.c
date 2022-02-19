/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aymoulou <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/02/19 11:24:53 by aymoulou          #+#    #+#             */
/*   Updated: 2022/02/19 11:26:01 by aymoulou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int tmp;

int error(char *str)
{
    int i = 0;
    while (str[i])
        write(2, &str[i++], 1);
    return 1;
}

int cd(char **argv, int i)
{
    if (i != 2)
        return(error("error: cd: bad arguments\n"));
    if (chdir(argv[1]))
        return(error("error: cd: cannot change directory to ") && error(argv[1]) && error("\n"));
    return 0;
}

int execute(char **argv, char **envp, int i)
{
    int tmp2[2];
    int r;
    int pip = (argv[i] && argv[i][0] == '|');
    if (pip && pipe(tmp2))
        return(error("error: fatal\n"));
    int pid = fork();
    if (pid == 0)
    {
        argv[i] = 0;
        if (dup2(tmp, 0) == -1 || close(tmp) == -1 || (pip && (dup2(tmp2[1], 1) == -1 || close(tmp2[0]) == -1 ||
        close(tmp2[1]) == -1)))
            return(error("error: fatal\n"));
        execve(*argv, argv, envp);
        return(error("error: cannot execute ") && error(argv[0]) && error("\n"));
    }
    if ((pip && (dup2(tmp2[0], tmp)  == -1 || close(tmp2[0])  == -1 || close(tmp2[1]) == -1)) || 
        (!pip && dup2(0, tmp) == -1) || waitpid(pid, &r, 0) == -1)
            return(error("error: fatal\n"));
    return (r > 0);
}

int main(int argc, char *argv[], char *envp[])
{
    int r;
    int i = 0;
    tmp = dup(0);
    while(argv[i] && argv[++i])
    {
        argv = argv + i;
        i = 0;
        while(argv[i] && argv[i][0] != '|' && argv[i][0] != ';')
            i++;
        if (!strcmp(argv[0], "cd"))
            r = cd(argv,i);
        else if (i)
            r = execute(argv, envp, i);
    }  
    return ((dup2(0, tmp) == -1 && error("error: fatal\n")) || r );
}
