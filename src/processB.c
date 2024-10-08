#include "./../include/processB_utilities.h"

int main(int argc, char const *argv[])
{
    int shared_map[W][H];
    const int SIZE = sizeof(shared_map);
    bmpfile_t *bmp;

    int pos_x = 0;
    int pos_y = 0;

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Define shared memory segment
    const char *shm_name = "/STATIC_SHARED_MEM";
    int i, shm_fd;
    int *ptr;

    // Open shared memory
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

    // Open semaphores
    if (open_semaphores() == -1)
        return -1;

    rgb_pixel_t pixel = {255, 0, 0, 0};

    // Create dynamic private memory
    if ((bmp = bmp_create(W, H, D)) == NULL)
    {
        perror("Error creating bitmap");
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

            // Dynamic private memory
            remove_previous_circle(bmp);
            print_circle(30, pixel, pos_x, pos_y, bmp);
            mvaddch(pos_y, pos_x, '0');
            refresh();
        }
    }

    // Unlink shared memory
    if (shm_unlink(shm_name) == 1)
    {
        perror("Error removing\n");
        return -1;
    }

    // Close and unlink semaphores
    sem_close(sem_id_reader);
    sem_close(sem_id_writer);
    sem_unlink(SEM_PATH_READER);
    sem_unlink(SEM_PATH_WRITER);

    // Destroy dynamic and shared memory
    munmap(ptr, SIZE);
    bmp_destroy(bmp);
    endwin();
    return 0;
}
