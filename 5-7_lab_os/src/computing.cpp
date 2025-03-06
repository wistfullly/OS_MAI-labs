#include <lib.h>

int main(int argc, char *argv[]) {
    Node I = createNode(atoi(argv[1]), true);
    std::map<std::string, int> dict;

    std::list<Node> children;
    while (true) {
        for (auto &i : children) {
            message m = get_mes(i);
            if (m.command != None) {
                send_mes(I, m);
            }
        }

        message m = get_mes(I);
        switch (m.command) {
        case Create:
            if (m.id == I.id) {
                Node child = createProcess(m.num);
                children.push_back(child);
                send_mes(I, {Create, child.id, child.pid});
            }
            else {
                for (auto &i : children) {
                    send_mes(i, m);
                }
            }
            break;
        case Ping:
            if (m.id == I.id) {
                send_mes(I, m);
            }
            else {
                for (auto &i : children) {
                    send_mes(i, m);
                }
            }
            break;
        case ExecAdd:
            if (m.id == I.id) {
                dict[std::string(m.st)] = m.num;
                send_mes(I, m);
            }
            else {
                for (auto &i : children) {
                    send_mes(i, m);
                }
            }
            break;
        case ExecFnd:
            if (m.id == I.id) {
                if (dict.find(std::string(m.st)) != dict.end()) {
                    send_mes(I, {ExecFnd, I.id, dict[std::string(m.st)], m.st});
                }
                else {
                    send_mes(I, {ExecErr, I.id, -1, m.st});
                }
            }
            else {
                for (auto &i : children) {
                    send_mes(i, m);
                }
            }
            break;
        default:
            break;
        }
        usleep(100000);
    }
    return 0;
}