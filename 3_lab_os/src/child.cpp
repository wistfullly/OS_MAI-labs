#include <iostream>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdlib>

struct shared_data {
    sem_t sem_parent;
    sem_t sem_child;
    char buffer[1024];
    int terminate;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./child <shm_name>" << std::endl;
        return 1;
    }

    const char *shm_name = argv[1];

    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Can't open shared memory object");
        exit(1);
    }

    shared_data *shm_ptr = (shared_data *) mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Can't mmap shared memory");
        exit(1);
    }

    while (true) {
        sem_wait(&shm_ptr->sem_parent);

        if (shm_ptr->terminate) {
            sem_post(&shm_ptr->sem_child); 
            break;
        }

        std::string str(shm_ptr->buffer);
        std::reverse(str.begin(), str.end());

        std::cout << str << std::endl;
        std::cout.flush();

        sem_post(&shm_ptr->sem_child);
    }

    munmap(shm_ptr, sizeof(shared_data));
    close(shm_fd);

    return 0;
}
