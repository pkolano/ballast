//
// Copyright (C) 2010 United States Government as represented by the
// Administrator of the National Aeronautics and Space Administration
// (NASA).  All Rights Reserved.
//
// This software is distributed under the NASA Open Source Agreement
// (NOSA), version 1.3.  The NOSA has been approved by the Open Source
// Initiative.  See http://www.opensource.org/licenses/nasa1.3.php
// for the complete NOSA document.
//
// THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF ANY
// KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT
// LIMITED TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO
// SPECIFICATIONS, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
// A PARTICULAR PURPOSE, OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT
// THE SUBJECT SOFTWARE WILL BE ERROR FREE, OR ANY WARRANTY THAT
// DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE SUBJECT SOFTWARE. THIS
// AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN ENDORSEMENT BY
// GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS, RESULTING
// DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS RESULTING
// FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY DISCLAIMS
// ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE, IF
// PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS".
//
// RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST THE UNITED STATES
// GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR
// RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS IN ANY
// LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH USE,
// INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
// RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND
// HOLD HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND
// SUBCONTRACTORS, AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED
// BY LAW.  RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE
// IMMEDIATE, UNILATERAL TERMINATION OF THIS AGREEMENT.
//

#include <netdb.h> 
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>

#define VERSION "1.4"

#ifndef CONF_FILE
#define CONF_FILE "/etc/ballastrc"
#endif
#ifndef DATA_SIZE
#define DATA_SIZE 1024
#endif

// note that -std=gnu99 must be used with gcc
// note that -maix64 must be used with aix

// required config settings
char *conf_req[] = {
    "alias_domain",
    "data_host",
    "relay_path",
};

// config settings and count
char **conf = NULL;
int nconf = 0;

// calling environment for timeouts
jmp_buf env;

// trim and store config setting in string
void putconf(char *string) {
    char *key = strdup(string);
    if (key == NULL) return;
    // trim comments
    char *tmp = index(key, '#');
    if (tmp != NULL) tmp[0] = 0;
    // trim trailing whitespace
    for (int i = strlen(key) - 1; i >= 0; --i)
        if (key[i] == ' ' || key[i] == '\t') key[i] = 0;
        else break;
    // trim leading whitespace
    key = &key[strspn(key, " \t")];
    // truncate string at whitespace
    key = strsep(&key, " \t");
    if (key == NULL) return;
    // allocate memory and assign value
    if (conf = realloc(conf, (nconf + 1) * sizeof(char *)))
        conf[nconf++] = key;
}

// return string value assigned to key in config
char *getconf(char *key) {
    // loop in reverse order so defaults are read last
    for (int i = nconf - 1; i >= 0; --i) {
        if (!strcmp(key, conf[i])) {
            // value stored after first string terminator
            char *val = &conf[i][strlen(conf[i]) + 1];
            // trim leading whitespace and return value
            return &val[strspn(val, " \t")];
        }
    }
    return NULL;
}

// return integer value assigned to key in config
int igetconf(char *key) {
    char *val = getconf(key);
    return strtol(val, NULL, 0);
}

// send data to host/port in ainfo and return response in data
int getdata(char *data, struct addrinfo *ainfo) {
    if (ainfo == NULL) return -1;

    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    // connect to server
    if (connect(sock, ainfo->ai_addr, ainfo->ai_addrlen) < 0) return -1;
    freeaddrinfo(ainfo);

    // send request
    int n = write(sock, data, strlen(data));
    if (n < 0) return -1;

    // read reply
    bzero(data, DATA_SIZE);
    n = read(sock, data, DATA_SIZE);
    close(sock);
    if (n < 0 || n == DATA_SIZE) return -1;
    if (n > 0 && data[n - 1] == '\n') data[n - 1] = 0;
    return 0;
}

// resolve host name and return addrinfo with data_port filled in
struct addrinfo *resolve(char *data_host) {
    if (data_host == NULL) return NULL;
    struct addrinfo *ainfo;
    struct addrinfo hints;
    bzero(&hints, sizeof(hints));
    if (strspn(data_host, "0123456789.") == strlen(data_host))
        hints.ai_flags = AI_NUMERICHOST;
    // get ip address
    if (getaddrinfo(data_host, getconf("data_port"), &hints, &ainfo))
        return NULL;
    return ainfo;
}

// alarm handler that jumps back to calling environment with invoking signal
void timeout(int sig) {
    longjmp(env, sig);
}

int main(int argc, char *argv[]) {
    int opt;
    int opt_list = 0;
    char *conf_file = CONF_FILE;
    while ((opt = getopt(argc, argv, "c:l")) != -1) {
        switch (opt) {
            case 'c':
                conf_file = optarg;
                break;
            case 'l':
                opt_list = 1;
                break;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "ERROR: no hostname given\n");
        exit(1);
    } else if (argc - optind > 1) {
        fprintf(stderr, "ERROR: too many arguments\n");
        exit(1);
    }

    // open config file
    char data[DATA_SIZE];
    FILE *f = fopen(conf_file, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: config file %s does not exist\n", conf_file);
        exit(1);
    }

    // store default configuration
    putconf("alias_last -last");
    putconf("alias_text unavailable");
    putconf("data_port 4411");
    putconf("data_timeout 2");
    putconf("relay_port 22");

    // store config settings
    while (fgets(data, DATA_SIZE, f) != NULL) {
        if (strlen(data) == DATA_SIZE - 1) {
            fprintf(stderr, "ERROR: config entry \"%s\" too long\n", data);
            exit(1);
        }
        if (data[strlen(data) - 1] == '\n') data[strlen(data) - 1] = 0;
        if (data[strlen(data) - 1] == '\r') data[strlen(data) - 1] = 0;
        putconf(data);
    }
    fclose(f);

    // check required config settings
    for (int i = 0; i < sizeof(conf_req) / sizeof(char *); i++) {
        char *tmp = getconf(conf_req[i]);
        if (tmp == NULL) {
            fprintf(stderr, "ERROR: config parameter %s is not defined\n",
                conf_req[i]);
            exit(1);
        }
    }

    // copy argument
    char arg[strlen(argv[optind]) + 1];
    strcpy(arg, argv[optind]);
    // chop off domain
    char *alias_domain = getconf("alias_domain");
    char *tmp = strstr(arg, alias_domain);
    if (tmp == arg + strlen(arg) - strlen(alias_domain)) tmp[0] = 0;
    // chop off last
    int last = 0;
    char *alias_last = getconf("alias_last");
    tmp = strstr(arg, alias_last);
    if (tmp == arg + strlen(arg) - strlen(alias_last)) {
        tmp[0] = 0;
        last = 1;
    }

    // find default hosts for given alias
    char hosts_alias[strlen(arg) + 7];
    sprintf(hosts_alias, "hosts_%s", arg);
    char *defaults = getconf(hosts_alias);

    if (defaults != NULL) {
        // given host matches defined alias, so load balance
        // retrieve least loaded hosts from data server or time out
        snprintf(data, DATA_SIZE, "%s%s %d\n", arg, 
            last ? getconf("alias_last") : "", getuid());
        signal(SIGALRM, timeout);
        char *host = strdup(getconf("data_host"));
        for (host = strtok(host, " "); host; host = strtok(NULL, " ")) {
            // resolve host name outside alarm to avoid reentrancy issues
            struct addrinfo *ainfo = resolve(host);
            alarm(igetconf("data_timeout"));
            if (setjmp(env) != SIGALRM && !getdata(data, ainfo)) break;
            // loop to try alternate data servers on failure
            alarm(0);
        }
        alarm(0);
        if (host == NULL) {
            // copy default data on failed retrieval for strtok
            strncpy(data, defaults, DATA_SIZE - 1);
            data[DATA_SIZE - 1] = 0;
        }
        if (strspn(data, " ") == strlen(data)) {
            // data is empty so no hosts available
            fprintf(stderr, "ERROR: all hosts for alias %s are %s\n",
                arg, getconf("alias_text"));
            exit(1);
        }
        // count number of hosts in data
        host = data;
        long n = 1;
        while ((host = strchr(host + 1, ' ')) != NULL) n++;
        // pick random host from data
        srand(time(NULL));
        n = rand() % n;
        for (host = strtok(data, " "); n-- > 0; host = strtok(NULL, " "));
        if (opt_list) {
            // print chosen host
            printf("%s%s\n", host, getconf("alias_domain"));
            exit(0);
        } else {
            // connect to chosen host
            execl(getconf("relay_path"), getconf("relay_path"), host,
                getconf("relay_port"), NULL);
            // does not return
        }
    }
    // given host does not match alias, so pass through untouched
    if (opt_list) {
        // print given host
        printf("%s\n", argv[optind]);
    } else {
        // connect to given host
        execl(getconf("relay_path"), getconf("relay_path"), argv[optind],
            getconf("relay_port"), NULL);
        // does not return
    }
}

