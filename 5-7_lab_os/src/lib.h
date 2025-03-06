#include <iostream>
#include <list>
#include <unordered_set>
#include <chrono>
#include <ctime>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "zmq.h"
#include <sys/select.h>
#include <map>

bool inputAvailable();

std::time_t t_now();

enum com : char {
    None = 0,
    Create = 1,
    Ping = 2,
    ExecAdd = 3,
    ExecFnd = 4,
    ExecErr = 5
};

class message {
public:
    message() {}
    message(com command, int id, int num) : command(command), id(id), num(num), sent_time(t_now()) {}
    message(com command, int id, int num, char s[]) : command(command), id(id), num(num), sent_time(t_now()) {
        for (int i = 0; i < 30; ++i) {
            st[i] = s[i];
        }
    }

    bool operator==(const message &other) const {
        return command == other.command && id == other.id && num == other.num && sent_time == other.sent_time;
    }

    com command;           // команда
    int id;                // айди
    int num;               // данные
    std::time_t sent_time; // время отправки
    char st[30];           // строка
};

class Node {
public:
    int id;
    pid_t pid;
    void *context;
    void *socket;
    std::string address;

    bool operator==(const Node &other) const {
        return id == other.id && pid == other.pid;
    }
};

Node createNode(int id, bool is_child);

Node createProcess(int id);

void send_mes(Node &node, message m);

message get_mes(Node &node);