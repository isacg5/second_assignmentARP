#include "./../include/processA_utilities.h"
#include <bmpfile.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#define SEM_PATH_WRITER "/sem_writer"
#define SEM_PATH_READER "/sem_reader"

#define W 1600
#define H 600
#define D 4

int index_snapshot = 0;
char filename[128];
bmpfile_t *bmp;
void *ptr;
sem_t *sem_id_writer;
sem_t *sem_id_reader;

void write_on_shared_mem(int *ptr, int radius, int x_pos, int y_pos)
{
    x_pos = 20 * x_pos;
    y_pos = 20 * y_pos;

    sem_wait(sem_id_writer);

    // Remove previous circle
    for (int row = 0; row < H; row++)
    {
        for (int col = 0; col < W; col++)
        {
            ptr[col + row * W] = 0;
        }
    }
    
    // Write the new one given the center coords and the radious
    for (int x = -radius; x <= radius; x++)
    {
        for (int y = -radius; y <= radius; y++)
        {
            // If distance is smaller, point is within the circle
            if (sqrt(x * x + y * y) < radius)
            {
                /*
                 * Color the pixel at the specified (x,y) position
                 * with the given pixel values
                 */
                int x_p = x_pos + x;
                int y_p = y_pos + y;
                ptr[y_p + W * x_p] = 255;
            }
        }
    }
    sem_post(sem_id_reader);
}

void remove_previous_circle()
{
    rgb_pixel_t *pixel;
    rgb_pixel_t white = {255, 255, 255, 0};
    for (int x = 0; x <= W; x++)
    {
        for (int y = 0; y <= H; y++)
        {
            bmp_set_pixel(bmp, x, y, white);
        }
    }
}

void print_circle(int radius, rgb_pixel_t pixel, int x_pos, int y_pos)
{
    for (int x = -radius; x <= radius; x++)
    {
        for (int y = -radius; y <= radius; y++)
        {
            // If distance is smaller, point is within the circle
            if (sqrt(x * x + y * y) < radius)
            {
                /*
                 * Color the pixel at the specified (x,y) position
                 * with the given pixel values
                 */
                bmp_set_pixel(bmp, (x_pos * 20) + x, (y_pos * 20) + y, pixel);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    const int SIZE = W*H*sizeof(int);
    const char *shm_name = "/STATIC_SHARED_MEM";
    int i, shm_fd;
    int *ptr;

    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == 1)
    {
        perror("Shared memory segment failed\n");
        return -1;
    }

    ftruncate(shm_fd, SIZE);
    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("Map failed\n");
        return -1;
    }

    sem_id_writer = sem_open(SEM_PATH_WRITER, O_CREAT, 0644, 1);
    if (sem_id_writer == (void *)-1)
    {
        perror("sem_open failure");
        return -1;
    }

    sem_id_reader = sem_open(SEM_PATH_READER, O_CREAT, 0644, 1);
    if (sem_id_reader == (void *)-1)
    {
        perror("sem_open failure");
        return -1;
    }

    sem_init(sem_id_writer, 1, 1);
    sem_init(sem_id_reader, 1, 0);

    rgb_pixel_t pixel = {255, 0, 0, 0};

    if ((bmp = bmp_create(W, H, D)) == NULL)
    {
        perror("Error creating bitmap");
        return -1;
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    int pos_x = 45;
    int pos_y = 15;
    write_on_shared_mem(ptr, 30, pos_y, pos_x);
    print_circle(30, pixel, pos_x, pos_y);

    // Infinite loop
    while (TRUE)
    {
        mvprintw(LINES - 2, 1, "X: %i, Y:%i", pos_x, pos_y);
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

        // Else, if user presses print button...
        else if (cmd == KEY_MOUSE)
        {
            if (getmouse(&event) == OK)
            {
                if (check_button_pressed(print_btn, &event))
                {
                    sprintf(filename, "./out/snapshot_%d.bmp", index_snapshot);
                    index_snapshot++;
                    // Save image as .bmp file
                    bmp_save(bmp, filename);

                    mvprintw(LINES - 1, 1, "Print button pressed");
                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS - BTN_SIZE_X - 2; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }

        // If input is an arrow key, move circle accordingly...
        else if (cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN)
        {
            switch (cmd)
            {
            case KEY_LEFT:
                if (pos_x > 1)
                {
                    pos_x -= 1;
                }
                break;

            case KEY_RIGHT:
                if (pos_x < 80)
                {
                    pos_x += 1;
                }
                break;

            case KEY_UP:
                if (pos_y > 2)
                {
                    pos_y -= 1;
                }
                break;

            case KEY_DOWN:
                if (pos_y < 28)
                {
                    pos_y += 1;
                }
                break;
            }

            // Dynamic private memory
            remove_previous_circle();
            print_circle(30, pixel, pos_x, pos_y);

            // Static shared memory
            write_on_shared_mem(ptr, 30, pos_y, pos_x);

            move_circle(cmd);
            draw_circle();
        }
    }

    sem_close(sem_id_reader);
    sem_close(sem_id_writer);
    sem_unlink(SEM_PATH_READER);
    sem_unlink(SEM_PATH_WRITER);

    munmap(ptr, SIZE);
    bmp_destroy(bmp);
    endwin();
    return 0;
}
