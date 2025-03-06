#include <./lib.h>

int main() {
    std::unordered_set<int> all_id;
    all_id.insert(-1);
    std::list<message> saved_mes;
    std::list<Node> children;

    std::string command;
    while (true) {
        for (auto &i : children) {
            message m = get_mes(i);
            switch (m.command) {
            case Create:
                all_id.insert(m.id);
                std::cout << "Ok: " << m.num << std::endl;
                for (auto it = saved_mes.begin(); it != saved_mes.end(); ++it) {
                    if (it->command == Create and it->num == m.id) {
                        saved_mes.erase(it);
                        break;
                    }
                }
                break;
            case Ping:
                std::cout << "Ok: " << m.id << " is available" << std::endl;
                for (auto it = saved_mes.begin(); it != saved_mes.end(); ++it) {
                    if (it->command == Ping and it->id == m.id) {
                        saved_mes.erase(it);
                        break;
                    }
                }
                break;
            case ExecErr:
                std::cout << "Ok: " << m.id << " '" << m.st << "'not fount" << std::endl;
                for (auto it = saved_mes.begin(); it != saved_mes.end(); ++it) {
                    if (it->command == ExecFnd and it->id == m.id) {
                        saved_mes.erase(it);
                        break;
                    }
                }
                break;
            case ExecAdd:
                std::cout << "Ok: " << m.id << std::endl;
                for (auto it = saved_mes.begin(); it != saved_mes.end(); ++it) {
                    if (it->command == ExecAdd and it->id == m.id) {
                        saved_mes.erase(it);
                        break;
                    }
                }
                break;
            case ExecFnd:
                std::cout << "Ok: " << m.id << " '" << m.st << "' " << m.num << std::endl;
                for (auto it = saved_mes.begin(); it != saved_mes.end(); ++it) {
                    if (it->command == ExecFnd and it->id == m.id) {
                        saved_mes.erase(it);
                        break;
                    }
                }
                break;
            default:
                continue;
            }
        }
        for (auto it = saved_mes.begin(); it != saved_mes.end(); ++it) {
            if (std::difftime(t_now(), it->sent_time) > 5) {
                switch (it->command) {
                case Ping:
                    std::cout << "Error: Ok " << it->id << " is unavailable" << std::endl;
                    break;
                case Create:
                    std::cout << "Error: Parent  " << it->id << " is unavailable" << std::endl;
                    break;
                case ExecAdd:
                case ExecFnd:
                    std::cout << "Error: Node  " << it->id << " is unavailable" << std::endl;
                    break;
                default:
                    break;
                }
                saved_mes.erase(it);
                break;
            }
        }

        if (!inputAvailable()) {
            continue;
        }
        std::cin >> command;
        if (command == "create") {
            int parent_id, child_id;
            std::cin >> child_id >> parent_id;
            if (all_id.count(child_id)) {
                std::cout << "Error: Node with id " << child_id << " already exists" << std::endl;
            }
            else if (!all_id.count(parent_id)) {
                std::cout << "Error: Parent with id " << parent_id << " not found" << std::endl;
            }
            else if (parent_id == -1) {
                Node child = createProcess(child_id);
                children.push_back(child);
                all_id.insert(child_id);
                std::cout << "Ok: " << child.pid << std::endl;
            }
            else {
                message m(Create, parent_id, child_id);
                saved_mes.push_back(m);
                for (auto &i : children)
                    send_mes(i, m);
            }
        }
        else if (command == "exec") {
            char input[100];
            fgets(input, sizeof(input), stdin);

            int id, val;
            char key[30];

            if (sscanf(input, "%d %30s %d", &id, key, &val) == 3) {
                if (!all_id.count(id)) {
                    std::cout << "Error: Node with id " << id << " doesn't exist" << std::endl;
                    continue;
                }
                message m = {ExecAdd, id, val, key};
                saved_mes.push_back(m);
                for (auto &i : children) {
                    send_mes(i, m);
                }
            }
            else if (sscanf(input, "%d %30s", &id, key) == 2) {
                if (!all_id.count(id)) {
                    std::cout << "Error: Node with id " << id << " doesn't exist" << std::endl;
                    continue;
                }
                message m = {ExecFnd, id, -1, key};
                saved_mes.push_back(m);
                for (auto &i : children) {
                    send_mes(i, m);
                }
            }
        }
        else if (command == "ping") {
            int id;
            std::cin >> id;
            if (!all_id.count(id)) {
                std::cout << "Error: Node with id " << id << " doesn't exist" << std::endl;
            }
            else {
                message m(Ping, id, 0);
                saved_mes.push_back(m);
                for (auto &i : children) {
                    send_mes(i, m);
                }
            }
        }
        else if (command == "exit"){
            break;
        }
        else
            std::cout << "Error: Command doesn't exist!" << std::endl;
        usleep(100000);
    }
    return 0;
}
