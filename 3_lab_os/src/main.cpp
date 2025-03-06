#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>

//обмен данными через отображаемый файл
struct shared_data {
    sem_t sem_parent;
    sem_t sem_child;
    char buffer[1024];
    int terminate;
};

int main() {
    std::cout << "name for child process 1: ";
    std::string file1_name;
    std::getline(std::cin, file1_name);

    std::cout << "name for child process 2: ";
    std::string file2_name;
    std::getline(std::cin, file2_name);

    int file1 = open(file1_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int file2 = open(file2_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file1 < 0 || file2 < 0) {
        perror("Can't open file");
        exit(1);
    }

    const char *shm_name1 = "/shm_child1";
    const char *shm_name2 = "/shm_child2";

    int shm_fd1 = shm_open(shm_name1, O_CREAT | O_RDWR, 0666);
    int shm_fd2 = shm_open(shm_name2, O_CREAT | O_RDWR, 0666);

    if (shm_fd1 == -1 || shm_fd2 == -1) {
        perror("Can't create shared memory object");
        exit(1);
    }

    ftruncate(shm_fd1, sizeof(shared_data));
    ftruncate(shm_fd2, sizeof(shared_data));

    shared_data *shm_ptr1 = (shared_data *) mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
    shared_data *shm_ptr2 = (shared_data *) mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);

    if (shm_ptr1 == MAP_FAILED || shm_ptr2 == MAP_FAILED) {
        perror("Can't mmap shared memory");
        exit(1);
    }

    sem_init(&shm_ptr1->sem_parent, 1, 0);
    sem_init(&shm_ptr1->sem_child, 1, 0);
    sem_init(&shm_ptr2->sem_parent, 1, 0);
    sem_init(&shm_ptr2->sem_child, 1, 0);

    shm_ptr1->terminate = 0;
    shm_ptr2->terminate = 0;

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("Can't fork");
        exit(1);
    }

    if (pid1 == 0) {
        munmap(shm_ptr2, sizeof(shared_data));
        close(shm_fd2);

        if (dup2(file1, STDOUT_FILENO) < 0) {
            perror("Can't redirect stdout for child process 1");
            exit(1);
        }
        close(file1);
        close(file2);

        execl("./child", "./child", shm_name1, NULL);
        perror("Can't execute child process 1");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("Can't fork");
        exit(1);
    }

    if (pid2 == 0) {
        munmap(shm_ptr1, sizeof(shared_data));
        close(shm_fd1);

        if (dup2(file2, STDOUT_FILENO) < 0) {
            perror("Can't redirect stdout for child process 2");
            exit(1);
        }
        close(file2);
        close(file1);

        execl("./child", "./child", shm_name2, NULL);
        perror("Can't execute child process 2");
        exit(1);
    }

    close(file1);
    close(file2);

    while (true) {
        std::string s;
        std::getline(std::cin, s);

        if (s.empty()) {
            shm_ptr1->terminate = 1;
            shm_ptr2->terminate = 1;

            sem_post(&shm_ptr1->sem_parent);
            sem_post(&shm_ptr2->sem_parent);
            break;
        }

        if (s.size() > 10) {
            strcpy(shm_ptr2->buffer, s.c_str());
            sem_post(&shm_ptr2->sem_parent);
            sem_wait(&shm_ptr2->sem_child);
        } else {
            strcpy(shm_ptr1->buffer, s.c_str());
            sem_post(&shm_ptr1->sem_parent); 
            sem_wait(&shm_ptr1->sem_child);
        }
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    sem_destroy(&shm_ptr1->sem_parent);
    sem_destroy(&shm_ptr1->sem_child);
    sem_destroy(&shm_ptr2->sem_parent);
    sem_destroy(&shm_ptr2->sem_child);

    munmap(shm_ptr1, sizeof(shared_data));
    munmap(shm_ptr2, sizeof(shared_data));
    close(shm_fd1);
    close(shm_fd2);
    shm_unlink(shm_name1);
    shm_unlink(shm_name2);

    return 0;
}
