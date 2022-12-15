#include "./../include/processB_utilities.h"
#include <semaphore.h>
#include <bmpfile.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#define SEM_PATH_WRITER "/sem_writer"
#define SEM_PATH_READER "/sem_reader"

#define W 1600
#define H 600
#define D 4

sem_t *sem_id_writer;
sem_t *sem_id_reader;

void get_position(int *ptr, int *x, int *y)
{
    sem_wait(sem_id_reader);
    int c = 0;
    int x_pos = 0;
    int y_pos = 0;
    for (int row = 0; row < H; row++)
    {
        c = 0;
        for (int col = 0; col < W; col++)
        {
            int index = col + row * W;
            if(ptr[index] == 255){
                c++;
            }
            else{
                c = 0;
            }

            if(c == 59){
                x_pos = col-29;
                y_pos = row;
            }
        }
    }

    *x = x_pos / 20;
    *y = y_pos / 20;

    sem_post(sem_id_writer);
}

int main(int argc, char const *argv[])
{
    int shared_map[W][H];
    const int SIZE = sizeof(shared_map);

    int pos_x = 0;
    int pos_y = 0;

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    const char *shm_name = "/STATIC_SHARED_MEM";
    int i, shm_fd;
    int *ptr;

    shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == 1)
    {
        perror("Shared memory segment failed\n");
        return -1;
    }

    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("Map failed\n");
        return 1;
    }

    sem_id_writer = sem_open(SEM_PATH_WRITER, 0);
    if (sem_id_writer == (void *)-1)
    {
        perror("sem_open failure");
        return -1;
    }

    sem_id_reader = sem_open(SEM_PATH_READER, 0);
    if (sem_id_reader == (void *)-1)
    {
        perror("sem_open failure");
        return -1;
    }

    // Infinite loop
    while (TRUE)
    {
        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if (cmd == KEY_RESIZE)
        {
            if (first_resize)
            {
                first_resize = FALSE;
            }
            else
            {
                reset_console_ui();
            }
        }

        else
        {
            get_position(ptr, &pos_x, &pos_y);
            mvprintw(LINES - 10, 1, "X: %i Y: %i", pos_x, pos_y);
            mvaddch(pos_y, pos_x, '0');
            refresh();
        }
    }

    if (shm_unlink(shm_name) == 1)
    {
        perror("Error removing\n");
        return -1;
    }

    sem_close(sem_id_reader);
    sem_close(sem_id_writer);
    sem_unlink(SEM_PATH_READER);
    sem_unlink(SEM_PATH_WRITER);

    endwin();
    return 0;
}
